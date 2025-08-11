/**
 * @file backend.c
 * @brief Реализация внутренней логики игры "Тетрис".
 *
 * Содержит функции для управления состоянием игры, обработки действий
 * пользователя, генерации фигур и обновления игрового поля.
 */

#include "backend.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

/** @brief Получает и инициализирует состояние игры. */
GameState *get_game_state() {
  static GameState state;
  static bool initialized = false;
  if (!initialized) {
    state.state = Start_state;
    state.user_action = Start;
    state.hold = false;
    for (int y = 0; y < FIELD_HEIGHT; y++)
      for (int x = 0; x < FIELD_WIDTH; x++) state.field[y][x] = EMPTY;
    for (int y = 0; y < TETR_SIZE; y++)
      for (int x = 0; x < TETR_SIZE; x++) {
        state.next[y][x] = EMPTY;
        state.current[y][x] = EMPTY;
      }
    state.next_type = generate_next_tetramino(&state);
    state.current_type = 0;
    state.x_position = FIELD_WIDTH / 2 - 2;
    state.y_position = 0;
    state.score = 0;
    state.high_score = read_score();
    state.level = 1;
    state.pause = 2;
    state.timer = update_timer();
    initialized = true;
  }
  return &state;
}

/** @brief Обрабатывает ввод пользователя. */
void userInput(UserAction_t action, bool hold) {
  GameState *state = get_game_state();
  state->user_action = action;
  state->hold = hold;
}

/** @brief Обновляет и возвращает текущее состояние игры. */
GameInfo_t updateCurrentState() {
  GameState *state = get_game_state();
  GameInfo_t result = {NULL, NULL, 0, state->high_score, 1, state->pause};

  // Выделяем память для поля и следующей фигуры
  result.field = malloc(FIELD_HEIGHT * sizeof(int *));
  for (int i = 0; i < FIELD_HEIGHT; i++) {
    result.field[i] = malloc(FIELD_WIDTH * sizeof(int));
  }

  result.next = malloc(TETR_SIZE * sizeof(int *));
  for (int i = 0; i < TETR_SIZE; i++) {
    result.next[i] = malloc(TETR_SIZE * sizeof(int));
  }

  // Обрабатываем состояние игры
  run_actions_by_state(state);

  if (state->pause != EXIT_GAME) {
    // Копируем состояние поля
    for (int y = 0; y < FIELD_HEIGHT; y++) {
      for (int x = 0; x < FIELD_WIDTH; x++) {
        result.field[y][x] = state->field[y][x];
      }
    }

    // Копируем следующую фигуру
    for (int y = 0; y < TETR_SIZE; y++) {
      for (int x = 0; x < TETR_SIZE; x++) {
        result.next[y][x] = state->next[y][x];
      }
    }

    // Добавляем текущую фигуру на поле
    for (int y = 0; y < TETR_SIZE; y++) {
      for (int x = 0; x < TETR_SIZE; x++) {
        if (state->current[y][x] && state->y_position + y >= 0 &&
            state->y_position + y < FIELD_HEIGHT &&
            state->x_position + x >= 0 && state->x_position + x < FIELD_WIDTH) {
          result.field[state->y_position + y][state->x_position + x] =
              state->current[y][x];
        }
      }
    }

    result.score = state->score;
    result.high_score = state->high_score;
    result.level = state->level;
    result.pause = state->pause;
  } else {
    for (int i = 0; i < FIELD_HEIGHT; i++) {
      free(result.field[i]);
    }
    free(result.field);

    for (int i = 0; i < TETR_SIZE; i++) {
      free(result.next[i]);
    }
    free(result.next);

    result.field = NULL;
    result.next = NULL;
    result.pause = EXIT_GAME;
  }

  return result;
}

/** @brief Выполняет действия в зависимости от текущего состояния. */
void run_actions_by_state(GameState *state) {
  switch (state->state) {
    case Start_state:
      initialize_game(state);
      if (state->hold && state->user_action == Start) {
        state->state = Spawn;
      }
      break;
    case Spawn:
      spawn_tetramino(state);
      break;
    case Moving:
      move_tetramino(state);
      break;
    case Shifting:
      shift_tetramino(state);
      break;
    case Attaching:
      attach_tetramino(state);
      break;
    case Pause_state:
      pause_actions(state);
      break;
    case Game_over:
      game_over_actions(state);
      break;
  }
}

/** @brief Инициализирует игровое поле и параметры. */
void initialize_game(GameState *state) {
  if (state->hold && state->user_action == Start) {
    for (int y = 0; y < FIELD_HEIGHT; y++)
      for (int x = 0; x < FIELD_WIDTH; x++) state->field[y][x] = EMPTY;
    state->score = 0;
    state->level = 1;
    state->pause = 0;
    state->high_score = read_score();
  } else if (state->hold && state->user_action == Terminate) {
    state->pause = EXIT_GAME;
  }
}

/** @brief Создает новую фигуру и проверяет столкновение. */
void spawn_tetramino(GameState *state) {
  for (int y = 0; y < TETR_SIZE; y++)
    for (int x = 0; x < TETR_SIZE; x++)
      state->current[y][x] = state->next[y][x];
  state->current_type = state->next_type;
  state->next_type = generate_next_tetramino(state);
  state->x_position = FIELD_WIDTH / 2 - 2;
  state->y_position = 0;
  if (check_collision(state, state->current, state->x_position,
                      state->y_position)) {
    state->state = Game_over;
  } else {
    state->state = Moving;
  }
}

/** @brief Обрабатывает действия пользователя для перемещения фигуры. */
void move_tetramino(GameState *state) {
  if (state->hold) {
    switch (state->user_action) {
      case Left:
        if (!check_collision(state, state->current, state->x_position - 1,
                             state->y_position))
          state->x_position--;
        break;
      case Right:
        if (!check_collision(state, state->current, state->x_position + 1,
                             state->y_position))
          state->x_position++;
        break;
      case Down:
        while (!check_collision(state, state->current, state->x_position,
                                state->y_position + 1))
          state->y_position++;
        state->state = Attaching;
        break;
      case Action:
        rotate_tetramino(state);
        break;
      case Pause:
        state->state = Pause_state;
        state->pause = 1;
        break;
      case Terminate:
        state->pause = EXIT_GAME;
        break;
      default:
        break;
    }
  }
  if (update_timer() - state->timer >= (700 - (state->level - 1) * 52)) {
    state->timer = update_timer();
    state->state = Shifting;
  }
}

/** @brief Сдвигает фигуру вниз, если нет столкновения. */
void shift_tetramino(GameState *state) {
  if (!check_collision(state, state->current, state->x_position,
                       state->y_position + 1)) {
    state->y_position++;
    state->state = Moving;
  } else {
    state->state = Attaching;
  }
}

/** @brief Присоединяет фигуру к полю и переходит к созданию новой. */
void attach_tetramino(GameState *state) {
  for (int y = 0; y < TETR_SIZE; y++) {
    for (int x = 0; x < TETR_SIZE; x++) {
      if (state->current[y][x] && state->y_position + y >= 0 &&
          state->y_position + y < FIELD_HEIGHT && state->x_position + x >= 0 &&
          state->x_position + x < FIELD_WIDTH) {
        state->field[state->y_position + y][state->x_position + x] =
            state->current[y][x];
      }
    }
  }
  calculate_lines(state);
  state->state = Spawn;
}

/** @brief Обрабатывает действия во время паузы. */
void pause_actions(GameState *state) {
  if (state->user_action == Pause && state->hold) {
    state->pause = 0;
    state->state = Moving;
  } else if (state->user_action == Terminate && state->hold) {
    state->pause = EXIT_GAME;
  }
}

/** @brief Обновляет рекорд и переходит к стартовому экрану. */
void game_over_actions(GameState *state) {
  if (state->score > state->high_score) {
    write_score(state->score);
    state->high_score = state->score;
  }
  state->pause = 2;
  state->state = Start_state;
}

/** @brief Проверяет и удаляет заполненные линии, обновляет счет. */
void calculate_lines(GameState *state) {
  int lines_cleared = 0;
  for (int y = 0; y < FIELD_HEIGHT; y++) {
    bool full_line = true;
    for (int x = 0; x < FIELD_WIDTH; x++) {
      if (state->field[y][x] == EMPTY) {
        full_line = false;
        break;
      }
    }
    if (full_line) {
      clear_line(state, y);
      lines_cleared++;
    }
  }
  if (lines_cleared > 0) {
    update_score(state, lines_cleared);
    update_speed_and_level(state);
  }
}

/** @brief Удаляет указанную линию и сдвигает поле вниз. */
void clear_line(GameState *state, int line) {
  while (line > 0) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      state->field[line][x] = state->field[line - 1][x];
    }
    line--;
  }
  for (int x = 0; x < FIELD_WIDTH; x++) {
    state->field[0][x] = EMPTY;
  }
}

/** @brief Начисляет очки за удаленные линии. */
void update_score(GameState *state, int lines_cleared) {
  switch (lines_cleared) {
    case 1:
      state->score += 100;
      break;
    case 2:
      state->score += 300;
      break;
    case 3:
      state->score += 700;
      break;
    case 4:
      state->score += 1500;
      break;
  }
}

/** @brief Обновляет уровень игры на основе счета. */
void update_speed_and_level(GameState *state) {
  int new_level = (state->score / 600) + 1;
  if (new_level <= 10) {
    state->level = new_level;
  }
}

/** @brief Генерирует случайную следующую фигуру. */
TetraminoType_t generate_next_tetramino(GameState *state) {
  TetraminoType_t type = (TetraminoType_t)(rand() % 7);
  fill_tetramino(state->next, type);
  return type;
}

/** @brief Заполняет массив фигуры блоками. */
void fill_tetramino(int tetramino[TETR_SIZE][TETR_SIZE], TetraminoType_t type) {
  for (int y = 0; y < TETR_SIZE; y++)
    for (int x = 0; x < TETR_SIZE; x++) tetramino[y][x] = EMPTY;
  switch (type) {
    case O_tetramino:
      tetramino[1][1] = BLOCK;
      tetramino[1][2] = BLOCK;
      tetramino[2][1] = BLOCK;
      tetramino[2][2] = BLOCK;
      break;
    case I_tetramino:
      tetramino[2][0] = BLOCK;
      tetramino[2][1] = BLOCK;
      tetramino[2][2] = BLOCK;
      tetramino[2][3] = BLOCK;
      break;
    case T_tetramino:
      tetramino[1][2] = BLOCK;
      tetramino[2][1] = BLOCK;
      tetramino[2][2] = BLOCK;
      tetramino[2][3] = BLOCK;
      break;
    case S_tetramino:
      tetramino[1][2] = BLOCK;
      tetramino[1][3] = BLOCK;
      tetramino[2][1] = BLOCK;
      tetramino[2][2] = BLOCK;
      break;
    case Z_tetramino:
      tetramino[1][1] = BLOCK;
      tetramino[1][2] = BLOCK;
      tetramino[2][2] = BLOCK;
      tetramino[2][3] = BLOCK;
      break;
    case J_tetramino:
      tetramino[1][1] = BLOCK;
      tetramino[2][1] = BLOCK;
      tetramino[2][2] = BLOCK;
      tetramino[2][3] = BLOCK;
      break;
    case L_tetramino:
      tetramino[1][3] = BLOCK;
      tetramino[2][1] = BLOCK;
      tetramino[2][2] = BLOCK;
      tetramino[2][3] = BLOCK;
      break;
  }
}

/** @brief Проверяет столкновение фигуры с полем или границами. */
bool check_collision(GameState *state, int tetramino[TETR_SIZE][TETR_SIZE],
                     int x, int y) {
  for (int i = 0; i < TETR_SIZE; i++) {
    for (int j = 0; j < TETR_SIZE; j++) {
      if (tetramino[i][j] != EMPTY) {
        int new_x = x + j;
        int new_y = y + i;
        if (new_x < 0 || new_x >= FIELD_WIDTH || new_y >= FIELD_HEIGHT)
          return true;
        if (new_y >= 0 && state->field[new_y][new_x] != EMPTY) return true;
      }
    }
  }
  return false;
}

/** @brief Поворачивает текущую фигуру, если нет столкновения. */
void rotate_tetramino(GameState *state) {
  if (state->current_type == O_tetramino) return;
  int temp[TETR_SIZE][TETR_SIZE];
  for (int y = 0; y < TETR_SIZE; y++)
    for (int x = 0; x < TETR_SIZE; x++)
      temp[y][x] = state->current[TETR_SIZE - 1 - x][y];
  if (!check_collision(state, temp, state->x_position, state->y_position)) {
    for (int y = 0; y < TETR_SIZE; y++)
      for (int x = 0; x < TETR_SIZE; x++) state->current[y][x] = temp[y][x];
  }
}

/** @brief Возвращает текущее время в миллисекундах. */
long long update_timer() {
  struct timeval tv;
  if (gettimeofday(&tv, NULL) == -1) {
    return 0;
  }
  return (long long)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

/** @brief Сохраняет рекорд в файл score.txt. */
void write_score(int high_score) {
  FILE *file = fopen("score.txt", "w");
  if (file) {
    fprintf(file, "%d", high_score);
    fclose(file);
  }
}

/** @brief Читает рекорд из файла score.txt. */
int read_score() {
  int high_score = 0;
  FILE *file = fopen("score.txt", "r");
  if (file) {
    fscanf(file, "%d", &high_score);
    fclose(file);
  }
  return high_score;
}