#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../brick_game/backend.h"

// Вспомогательная функция для создания временного файла с рекордом
void setup_score_file(int score) {
  FILE *file = fopen("score.txt", "w");
  if (file) {
    fprintf(file, "%d", score);
    fclose(file);
  }
}

// Вспомогательная функция для удаления временного файла
void teardown_score_file() { remove("score.txt"); }

// Тест для get_game_state
START_TEST(test_get_game_state) {
  GameState *state = get_game_state();
  ck_assert_ptr_nonnull(state);
  ck_assert_int_eq(state->state, Start_state);
  ck_assert_int_eq(state->user_action, Start);
  ck_assert_int_eq(state->score, 0);
  ck_assert_int_eq(state->level, 1);
  ck_assert_int_eq(state->pause, 2);
  for (int y = 0; y < FIELD_HEIGHT; y++)
    for (int x = 0; x < FIELD_WIDTH; x++)
      ck_assert_int_eq(state->field[y][x], EMPTY);
}
END_TEST

// Тест для userInput
START_TEST(test_userInput) {
  GameState *state = get_game_state();
  userInput(Left, true);
  ck_assert_int_eq(state->user_action, Left);
  ck_assert_int_eq(state->hold, true);
  userInput(Pause, false);
  ck_assert_int_eq(state->user_action, Pause);
  ck_assert_int_eq(state->hold, false);
}
END_TEST

// Тест для initialize_game
START_TEST(test_initialize_game) {
  GameState *state = get_game_state();
  state->score = 100;
  state->level = 2;
  state->pause = 1;
  state->field[0][0] = BLOCK;
  state->user_action = Start;
  state->hold = true;

  initialize_game(state);

  ck_assert_int_eq(state->score, 0);
  ck_assert_int_eq(state->level, 1);
  ck_assert_int_eq(state->pause, 0);
  ck_assert_int_eq(state->field[0][0], EMPTY);
}
END_TEST

// Тест для spawn_tetramino
START_TEST(test_spawn_tetramino) {
  GameState *state = get_game_state();
  state->next_type = O_tetramino;
  fill_tetramino(state->next, O_tetramino);
  spawn_tetramino(state);

  ck_assert_int_eq(state->current_type, O_tetramino);
  ck_assert_int_eq(state->x_position, FIELD_WIDTH / 2 - 2);
  ck_assert_int_eq(state->y_position, 0);
  ck_assert_int_eq(state->state, Moving);
}
END_TEST

// Тест для move_tetramino (движение влево)
START_TEST(test_move_tetramino_left) {
  GameState *state = get_game_state();
  state->state = Moving;
  state->user_action = Left;
  state->hold = true;
  state->x_position = 2;
  state->current_type = O_tetramino;
  fill_tetramino(state->current, O_tetramino);

  move_tetramino(state);

  ck_assert_int_eq(state->x_position, 1);
}
END_TEST

// Тест для move_tetramino (падение вниз)
START_TEST(test_move_tetramino_down) {
  GameState *state = get_game_state();
  state->state = Moving;
  state->user_action = Down;
  state->hold = true;
  state->y_position = 0;
  state->current_type = O_tetramino;
  fill_tetramino(state->current, O_tetramino);

  move_tetramino(state);

  ck_assert_int_eq(state->state, Attaching);
  ck_assert_int_ge(state->y_position, FIELD_HEIGHT - TETR_SIZE);
}
END_TEST

// Тест для shift_tetramino
START_TEST(test_shift_tetramino) {
  GameState *state = get_game_state();
  state->state = Shifting;
  state->y_position = 0;
  state->current_type = O_tetramino;
  fill_tetramino(state->current, O_tetramino);

  shift_tetramino(state);

  ck_assert_int_eq(state->y_position, 1);
  ck_assert_int_eq(state->state, Moving);
}
END_TEST

// Тест для attach_tetramino
START_TEST(test_attach_tetramino) {
  GameState *state = get_game_state();
  state->state = Attaching;
  state->y_position = FIELD_HEIGHT - TETR_SIZE;
  state->x_position = 0;
  state->current_type = O_tetramino;
  fill_tetramino(state->current, O_tetramino);

  attach_tetramino(state);

  ck_assert_int_eq(state->field[FIELD_HEIGHT - 3][1], BLOCK);
  ck_assert_int_eq(state->field[FIELD_HEIGHT - 3][2], BLOCK);
  ck_assert_int_eq(state->field[FIELD_HEIGHT - 2][1], BLOCK);
  ck_assert_int_eq(state->field[FIELD_HEIGHT - 2][2], BLOCK);
  ck_assert_int_eq(state->state, Spawn);
}
END_TEST

// Тест для pause_actions
START_TEST(test_pause_actions) {
  GameState *state = get_game_state();
  state->state = Pause_state;
  state->user_action = Pause;
  state->hold = true;

  pause_actions(state);

  ck_assert_int_eq(state->pause, 0);
  ck_assert_int_eq(state->state, Moving);
}
END_TEST

// Тест для game_over_actions
START_TEST(test_game_over_actions) {
  GameState *state = get_game_state();
  state->state = Game_over;
  state->score = 1000;
  state->high_score = 500;

  setup_score_file(500);
  game_over_actions(state);
  teardown_score_file();

  ck_assert_int_eq(state->high_score, 1000);
  ck_assert_int_eq(state->pause, 2);
  ck_assert_int_eq(state->state, Start_state);
}
END_TEST

// Тест для calculate_lines
START_TEST(test_calculate_lines) {
  GameState *state = get_game_state();
  for (int x = 0; x < FIELD_WIDTH; x++)
    state->field[FIELD_HEIGHT - 1][x] = BLOCK;
  state->score = 0;

  calculate_lines(state);

  ck_assert_int_eq(state->score, 100);
  for (int x = 0; x < FIELD_WIDTH; x++)
    ck_assert_int_eq(state->field[FIELD_HEIGHT - 1][x], EMPTY);
}
END_TEST

// Тест для clear_line
START_TEST(test_clear_line) {
  GameState *state = get_game_state();
  for (int x = 0; x < FIELD_WIDTH; x++) {
    state->field[FIELD_HEIGHT - 1][x] = BLOCK;
    state->field[FIELD_HEIGHT - 2][x] = BLOCK;
  }

  clear_line(state, FIELD_HEIGHT - 1);

  for (int x = 0; x < FIELD_WIDTH; x++) {
    ck_assert_int_eq(state->field[FIELD_HEIGHT - 1][x], BLOCK);
    ck_assert_int_eq(state->field[0][x], EMPTY);
  }
}
END_TEST

// Тест для update_score
START_TEST(test_update_score) {
  GameState *state = get_game_state();
  state->score = 0;

  update_score(state, 1);
  ck_assert_int_eq(state->score, 100);

  update_score(state, 2);
  ck_assert_int_eq(state->score, 400);

  update_score(state, 3);
  ck_assert_int_eq(state->score, 1100);

  update_score(state, 4);
  ck_assert_int_eq(state->score, 2600);
}
END_TEST

// Тест для update_speed_and_level
START_TEST(test_update_speed_and_level) {
  GameState *state = get_game_state();
  state->score = 1200;

  update_speed_and_level(state);

  ck_assert_int_eq(state->level, 3);
}
END_TEST

// Тест для generate_next_tetramino
START_TEST(test_generate_next_tetramino) {
  GameState *state = get_game_state();
  srand(0);  // Фиксируем генератор для воспроизводимости
  TetraminoType_t type = generate_next_tetramino(state);

  ck_assert_int_ge(type, 0);
  ck_assert_int_le(type, 6);
  for (int y = 0; y < TETR_SIZE; y++)
    for (int x = 0; x < TETR_SIZE; x++)
      if (state->next[y][x]) ck_assert_int_eq(state->next[y][x], BLOCK);
}
END_TEST

// Тест для fill_tetramino
START_TEST(test_fill_tetramino) {
  int tetramino[TETR_SIZE][TETR_SIZE] = {0};
  fill_tetramino(tetramino, O_tetramino);

  ck_assert_int_eq(tetramino[1][1], BLOCK);
  ck_assert_int_eq(tetramino[1][2], BLOCK);
  ck_assert_int_eq(tetramino[2][1], BLOCK);
  ck_assert_int_eq(tetramino[2][2], BLOCK);
}
END_TEST

// Тест для check_collision
START_TEST(test_check_collision) {
  GameState *state = get_game_state();
  int tetramino[TETR_SIZE][TETR_SIZE] = {0};
  fill_tetramino(tetramino,
                 I_tetramino);  // Используем I-тетрамино для большей длины

  // Нет столкновения
  ck_assert_int_eq(check_collision(state, tetramino, 0, 0), false);

  // Столкновение с левой границей
  ck_assert_int_eq(check_collision(state, tetramino, -2, 0),
                   true);  // I-тетрамино длиннее, поэтому -2

  // Столкновение с нижней границей
  ck_assert_int_eq(check_collision(state, tetramino, 0, FIELD_HEIGHT - 1),
                   true);

  // Столкновение с полем
  state->field[FIELD_HEIGHT - 1][0] = BLOCK;
  ck_assert_int_eq(check_collision(state, tetramino, 0, FIELD_HEIGHT - 3),
                   true);
}
END_TEST

// Тест для write_score и read_score
START_TEST(test_write_read_score) {
  write_score(1000);
  int score = read_score();
  ck_assert_int_eq(score, 1000);
  teardown_score_file();
}
END_TEST

// Тест для move_tetramino (движение вправо)
START_TEST(test_move_tetramino_right) {
  GameState *state = get_game_state();
  state->state = Moving;
  state->user_action = Right;
  state->hold = true;
  state->x_position = 0;
  state->current_type = O_tetramino;
  fill_tetramino(state->current, O_tetramino);

  move_tetramino(state);

  ck_assert_int_eq(state->x_position, 1);
}
END_TEST

// Тест для move_tetramino (вращение)
START_TEST(test_move_tetramino_rotate) {
  GameState *state = get_game_state();
  state->state = Moving;
  state->user_action = Action;
  state->hold = true;
  state->current_type = I_tetramino;
  fill_tetramino(state->current, I_tetramino);
  int original[TETR_SIZE][TETR_SIZE];
  memcpy(original, state->current, sizeof(original));

  move_tetramino(state);

  // Проверяем, что фигура изменила ориентацию
  ck_assert(memcmp(original, state->current, sizeof(original)) != 0);
}
END_TEST

// Тест для game_over_actions без нового рекорда
START_TEST(test_game_over_actions_no_highscore) {
  GameState *state = get_game_state();
  state->state = Game_over;
  state->score = 400;
  state->high_score = 500;

  setup_score_file(500);
  game_over_actions(state);
  teardown_score_file();

  ck_assert_int_eq(state->high_score, 500);  // Рекорд не должен измениться
  ck_assert_int_eq(state->pause, 2);
  ck_assert_int_eq(state->state, Start_state);
}
END_TEST

// Тест для clear_line с несколькими линиями
START_TEST(test_clear_multiple_lines) {
  GameState *state = get_game_state();
  // Заполняем две нижние линии
  for (int y = FIELD_HEIGHT - 2; y < FIELD_HEIGHT; y++)
    for (int x = 0; x < FIELD_WIDTH; x++) state->field[y][x] = BLOCK;

  // Добавляем блоки выше
  state->field[FIELD_HEIGHT - 3][0] = BLOCK;

  calculate_lines(state);

  // Проверяем, что линии очищены, а блок сверху опустился
  ck_assert_int_eq(state->field[FIELD_HEIGHT - 1][0], BLOCK);
  ck_assert_int_eq(state->field[FIELD_HEIGHT - 2][0], EMPTY);
  ck_assert_int_eq(state->score, 300);  // 100 за первую линию + 200 за вторую
}
END_TEST

// Тест для update_speed_and_level без изменения уровня
START_TEST(test_update_speed_no_level_change) {
  GameState *state = get_game_state();
  state->score = 400;  // Недостаточно для перехода на следующий уровень

  update_speed_and_level(state);

  ck_assert_int_eq(state->level, 1);
}
END_TEST

// Тест для write_score с нулевым значением
START_TEST(test_write_zero_score) {
  write_score(0);
  int score = read_score();
  ck_assert_int_eq(score, 0);
  teardown_score_file();
}
END_TEST

// Тест для read_score с отсутствующим файлом
START_TEST(test_read_missing_score_file) {
  teardown_score_file();  // Убедимся, что файла нет
  int score = read_score();
  ck_assert_int_eq(score, 0);
}
END_TEST

// Вспомогательная функция для проверки освобождения памяти
int memory_leak_check(GameInfo_t *info) {
  if (info->field) {
    for (int i = 0; i < FIELD_HEIGHT; i++) {
      if (info->field[i]) return 1;
    }
    free(info->field);
  }
  if (info->next) {
    for (int i = 0; i < TETR_SIZE; i++) {
      if (info->next[i]) return 1;
    }
    free(info->next);
  }
  return 0;
}

// Тест для userInput
START_TEST(test_userInput_basic) {
  userInput(Left, true);
  GameState *state = get_game_state();
  ck_assert_int_eq(state->user_action, Left);
  ck_assert_int_eq(state->hold, true);

  userInput(Pause, false);
  ck_assert_int_eq(state->user_action, Pause);
  ck_assert_int_eq(state->hold, false);
}
END_TEST

// Исправленный тест для updateCurrentState в нормальном режиме
START_TEST(test_updateCurrentState_normal) {
  // Полная инициализация состояния
  GameState *state = get_game_state();
  initialize_game(state);
  state->state = Moving;
  state->current_type = O_tetramino;
  fill_tetramino(state->current, O_tetramino);
  state->x_position = 3;
  state->y_position = 5;
  state->score = 500;
  state->high_score = 1000;
  state->level = 2;
  state->pause = 0;

  // Явная очистка поля
  memset(state->field, EMPTY, sizeof(state->field));

  GameInfo_t info = updateCurrentState();

  // Проверки базовых полей
  ck_assert_ptr_nonnull(info.field);
  ck_assert_ptr_nonnull(info.next);
  ck_assert_int_eq(info.score, 500);
  ck_assert_int_eq(info.high_score, 1000);
  ck_assert_int_eq(info.level, 2);
  ck_assert_int_eq(info.pause, 0);
}
END_TEST

START_TEST(test_updateCurrentState_exit) {
  GameState *state = get_game_state();
  state->pause = EXIT_GAME;
  state->state = Start_state;  // Добавляем явное состояние

  GameInfo_t info = updateCurrentState();

  ck_assert_ptr_null(info.field);
  ck_assert_ptr_null(info.next);
  ck_assert_int_eq(info.score, 0);
  ck_assert_int_eq(info.high_score,
                   state->high_score);  // Используем значение из состояния
  ck_assert_int_eq(info.level, 1);
  ck_assert_int_eq(info.pause, EXIT_GAME);
}
END_TEST

// Тест для run_actions_by_state в состоянии Start_state
START_TEST(test_run_actions_start_state) {
  GameState *state = get_game_state();
  state->state = Start_state;
  state->user_action = Start;
  state->hold = true;

  run_actions_by_state(state);
  ck_assert_int_eq(state->state, Spawn);

  // Проверка инициализации
  state->state = Start_state;
  state->user_action = Terminate;
  run_actions_by_state(state);
  ck_assert_int_eq(state->pause, EXIT_GAME);
}
END_TEST

// Тест для run_actions_by_state в состоянии Moving
START_TEST(test_run_actions_moving_state) {
  GameState *state = get_game_state();
  state->state = Moving;
  state->user_action = Down;
  state->hold = true;
  state->x_position = 3;
  state->y_position = 5;
  state->current_type = O_tetramino;
  fill_tetramino(state->current, O_tetramino);

  // Установим таймер так, чтобы сработал Shifting
  state->timer = update_timer() - 1000;

  run_actions_by_state(state);
  ck_assert_int_eq(state->state, Shifting);
}
END_TEST

// Тест для run_actions_by_state в состоянии Game_over
START_TEST(test_run_actions_game_over) {
  GameState *state = get_game_state();
  state->state = Game_over;
  state->score = 1500;
  state->high_score = 1000;
  setup_score_file(1000);

  run_actions_by_state(state);
  ck_assert_int_eq(state->state, Start_state);
  ck_assert_int_eq(state->high_score, 1500);
  ck_assert_int_eq(state->pause, 2);

  teardown_score_file();
}
END_TEST

// Тест для run_actions_by_state в состоянии Pause_state
START_TEST(test_run_actions_pause_state) {
  GameState *state = get_game_state();
  state->state = Pause_state;
  state->user_action = Pause;
  state->hold = true;
  state->pause = 1;

  run_actions_by_state(state);
  ck_assert_int_eq(state->state, Moving);
  ck_assert_int_eq(state->pause, 0);
}
END_TEST

// Тест для userInput с проверкой всех возможных действий
START_TEST(test_userInput_all_actions) {
  UserAction_t actions[] = {Start, Pause, Terminate, Left, Right, Down, Action};

  for (size_t i = 0; i < sizeof(actions) / sizeof(actions[0]); i++) {
    userInput(actions[i], true);
    GameState *state = get_game_state();
    ck_assert_int_eq(state->user_action, actions[i]);
    ck_assert_int_eq(state->hold, true);

    userInput(actions[i], false);
    ck_assert_int_eq(state->hold, false);
  }
}
END_TEST

// Тест для run_actions_by_state в состоянии Spawn
START_TEST(test_run_actions_spawn_state) {
  GameState *state = get_game_state();
  state->state = Spawn;
  state->next_type = I_tetramino;
  fill_tetramino(state->next, I_tetramino);

  run_actions_by_state(state);

  ck_assert_int_eq(state->current_type, I_tetramino);
  ck_assert_int_eq(state->x_position, FIELD_WIDTH / 2 - 2);
  ck_assert_int_eq(state->y_position, 0);
  ck_assert_int_ne(state->next_type,
                   I_tetramino);  // Должна сгенерироваться новая фигура
}
END_TEST

// Тест для run_actions_by_state в состоянии Shifting
START_TEST(test_run_actions_shifting_state) {
  GameState *state = get_game_state();
  state->state = Shifting;
  state->current_type = O_tetramino;
  fill_tetramino(state->current, O_tetramino);
  state->x_position = 3;
  state->y_position = 5;

  run_actions_by_state(state);

  // Фигура должна либо сдвинуться вниз, либо перейти в Attaching
  ck_assert(state->state == Moving || state->state == Attaching);
  if (state->state == Moving) {
    ck_assert_int_eq(state->y_position, 6);
  }
}
END_TEST

// Тест для run_actions_by_state в состоянии Attaching
START_TEST(test_run_actions_attaching_state) {
  GameState *state = get_game_state();
  state->state = Attaching;
  state->current_type = O_tetramino;
  fill_tetramino(state->current, O_tetramino);
  state->x_position = 3;
  state->y_position = FIELD_HEIGHT - 2;  // Почти внизу

  // Очищаем поле
  memset(state->field, EMPTY, sizeof(state->field));

  run_actions_by_state(state);

  ck_assert_int_eq(state->state, Spawn);

  // Проверяем что фигура прикрепилась к полю
  int attached = 0;
  for (int y = 0; y < TETR_SIZE; y++) {
    for (int x = 0; x < TETR_SIZE; x++) {
      if (state->current[y][x] && state->y_position + y >= 0 &&
          state->y_position + y < FIELD_HEIGHT) {
        attached += state->field[state->y_position + y][state->x_position + x];
      }
    }
  }
  ck_assert_int_gt(attached, 0);
}
END_TEST

// Тест для run_actions_by_state с обработкой Terminate
START_TEST(test_run_actions_terminate) {
  GameState *state = get_game_state();
  state->state = Moving;
  state->user_action = Terminate;
  state->hold = true;

  run_actions_by_state(state);

  ck_assert_int_eq(state->pause, EXIT_GAME);
}
END_TEST

// Создание набора тестов
Suite *backend_suite(void) {
  Suite *s = suite_create("Backend");
  TCase *tc_core = tcase_create("Core");

  // Добавляем существующие тесты
  tcase_add_test(tc_core, test_get_game_state);
  tcase_add_test(tc_core, test_userInput);
  tcase_add_test(tc_core, test_initialize_game);
  tcase_add_test(tc_core, test_spawn_tetramino);
  tcase_add_test(tc_core, test_move_tetramino_left);
  tcase_add_test(tc_core, test_move_tetramino_down);
  tcase_add_test(tc_core, test_shift_tetramino);
  tcase_add_test(tc_core, test_attach_tetramino);
  tcase_add_test(tc_core, test_pause_actions);
  tcase_add_test(tc_core, test_game_over_actions);
  tcase_add_test(tc_core, test_calculate_lines);
  tcase_add_test(tc_core, test_clear_line);
  tcase_add_test(tc_core, test_update_score);
  tcase_add_test(tc_core, test_update_speed_and_level);
  tcase_add_test(tc_core, test_generate_next_tetramino);
  tcase_add_test(tc_core, test_fill_tetramino);
  tcase_add_test(tc_core, test_check_collision);
  tcase_add_test(tc_core, test_write_read_score);
  tcase_add_test(tc_core, test_move_tetramino_right);
  tcase_add_test(tc_core, test_move_tetramino_rotate);
  tcase_add_test(tc_core, test_game_over_actions_no_highscore);
  tcase_add_test(tc_core, test_clear_multiple_lines);
  tcase_add_test(tc_core, test_update_speed_no_level_change);
  tcase_add_test(tc_core, test_write_zero_score);
  tcase_add_test(tc_core, test_read_missing_score_file);
  tcase_add_test(tc_core, test_userInput_basic);
  tcase_add_test(tc_core, test_updateCurrentState_normal);
  tcase_add_test(tc_core, test_updateCurrentState_exit);
  tcase_add_test(tc_core, test_run_actions_start_state);
  tcase_add_test(tc_core, test_run_actions_moving_state);
  tcase_add_test(tc_core, test_run_actions_game_over);
  tcase_add_test(tc_core, test_run_actions_pause_state);
  tcase_add_test(tc_core, test_userInput_all_actions);
  tcase_add_test(tc_core, test_run_actions_spawn_state);
  tcase_add_test(tc_core, test_run_actions_shifting_state);
  tcase_add_test(tc_core, test_run_actions_attaching_state);
  tcase_add_test(tc_core, test_run_actions_terminate);

  suite_add_tcase(s, tc_core);
  return s;
}

int main(void) {
  Suite *s = backend_suite();
  SRunner *sr = srunner_create(s);
  srunner_run_all(sr, CK_NORMAL);
  int failures = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (failures == 0) ? 0 : 1;
}
