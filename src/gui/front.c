/**
 * @file front.c
 * @brief Реализация терминального интерфейса игры "Тетрис".
 *
 * Содержит функции для инициализации ncurses, отрисовки игрового поля,
 * следующей фигуры и информационной панели, а также обработки ввода.
 */

#include "front.h"

#include <locale.h>
#include <time.h>

/** @brief Инициализирует и запускает игру. */
int main() {
  srand(time(NULL));
  setlocale(LC_ALL, "");
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(0);
  nodelay(stdscr, TRUE);
  start_color();
  init_pair(1, COLOR_BLACK, COLOR_YELLOW);
  init_pair(2, COLOR_BLACK, COLOR_CYAN);
  init_pair(3, COLOR_BLACK, COLOR_MAGENTA);
  init_pair(4, COLOR_BLACK, COLOR_GREEN);
  init_pair(5, COLOR_BLACK, COLOR_RED);
  init_pair(6, COLOR_BLACK, COLOR_BLUE);
  init_pair(7, COLOR_BLACK, COLOR_WHITE);

  run_game_loop();

  endwin();
  return 0;
}

/** @brief Запускает основной игровой цикл. */
void run_game_loop() {
  Interface_t windows;

  int start_y = 1;
  int start_x = 2;
  int panel_width = FIELD_WIDTH * 2 + 2;

  windows.game_win = newwin(FIELD_HEIGHT + 2, panel_width, start_y, start_x);
  windows.next_win = newwin(TETR_SIZE + 2, panel_width - 2, start_y,
                            start_x + panel_width + 1);
  windows.info_win = newwin(16, panel_width - 2, start_y + TETR_SIZE + 2,
                            start_x + panel_width + 1);

  while (true) {
    GameInfo_t info = updateCurrentState();
    if (info.pause == EXIT_GAME) break;

    print_field(&info, &windows);
    print_next(&info, &windows);
    print_info(&info, &windows);

    int key = getch();
    bool hold = (key != ERR);
    UserAction_t action = get_action(key);
    userInput(action, hold);

    free_game_info(&info);
    refresh();
    napms(5);
  }

  delwin(windows.game_win);
  delwin(windows.next_win);
  delwin(windows.info_win);
}

/** @brief Отрисовывает игровое поле с текущей фигурой. */
void print_field(const GameInfo_t *info, Interface_t *windows) {
  werase(windows->game_win);
  box(windows->game_win, 0, 0);
  for (int y = 0; y < FIELD_HEIGHT; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      if (info->field[y][x]) {
        wattron(windows->game_win, COLOR_PAIR(info->field[y][x]));
        mvwaddch(windows->game_win, y + 1, x * 2 + 1, ' ');
        mvwaddch(windows->game_win, y + 1, x * 2 + 2, ' ');
        wattroff(windows->game_win, COLOR_PAIR(info->field[y][x]));
      } else {
        mvwaddch(windows->game_win, y + 1, x * 2 + 1, ' ');
        mvwaddch(windows->game_win, y + 1, x * 2 + 2, ' ');
      }
    }
  }
  wrefresh(windows->game_win);
}

/** @brief Отрисовывает следующую фигуру в окне превью. */
void print_next(const GameInfo_t *info, Interface_t *windows) {
  werase(windows->next_win);
  box(windows->next_win, 0, 0);
  for (int y = 0; y < TETR_SIZE; y++) {
    for (int x = 0; x < TETR_SIZE; x++) {
      if (info->next[y][x]) {
        wattron(windows->next_win, COLOR_PAIR(info->next[y][x]));
        mvwaddch(windows->next_win, y + 1,
                 (getmaxx(windows->next_win) - TETR_SIZE * 2) / 2 + x * 2 + 1,
                 ' ');
        mvwaddch(windows->next_win, y + 1,
                 (getmaxx(windows->next_win) - TETR_SIZE * 2) / 2 + x * 2 + 2,
                 ' ');
        wattroff(windows->next_win, COLOR_PAIR(info->next[y][x]));
      }
    }
  }
  wrefresh(windows->next_win);
}

/** @brief Отрисовывает информацию о счете, рекорде и уровне. */
void print_info(const GameInfo_t *info, Interface_t *windows) {
  int win_width = FIELD_WIDTH * 2;
  werase(windows->info_win);
  box(windows->info_win, 0, 0);

  if (info->pause == 1) {
    mvwprintw(windows->info_win, 1, (win_width - 9) / 2, "= PAUSE =");
    mvwprintw(windows->info_win, 3, (win_width - 10) / 2, "Press 'P'");
    mvwprintw(windows->info_win, 4, (win_width - 12) / 2, "to continue");
  } else if (info->pause == 2) {
    mvwprintw(windows->info_win, 1, (win_width - 9) / 2, "= TETRIS =");
    mvwprintw(windows->info_win, 3, (win_width - 13) / 2, "Press 'ENTER'");
    mvwprintw(windows->info_win, 4, (win_width - 10) / 2, "to start");
  }

  if (info->pause) {
    mvwprintw(windows->info_win, 6, (win_width - 11) / 2, "Press 'ESC'");
    mvwprintw(windows->info_win, 7, (win_width - 9) / 2, "to exit");
    mvwprintw(windows->info_win, 9, (win_width - 8) / 2, "CONTROL");
    mvwprintw(windows->info_win, 10, (win_width - 11) / 2, "< arrows >");
    mvwprintw(windows->info_win, 11, (win_width - 2) / 2, "v");
    mvwprintw(windows->info_win, 12, (win_width - 8) / 2, "'space'");
    mvwprintw(windows->info_win, 13, (win_width - 13) / 2, "for rotation");
  } else {
    int offset_score = offset_counter(info->score);
    int offset_high = offset_counter(info->high_score);
    mvwprintw(windows->info_win, 1, (win_width - 6) / 2, "Record");
    mvwprintw(windows->info_win, 2, (win_width - offset_high) / 2, "%d",
              info->high_score);
    mvwprintw(windows->info_win, 4, (win_width - 5) / 2, "Score");
    mvwprintw(windows->info_win, 5, (win_width - offset_score) / 2, "%d",
              info->score);
    mvwprintw(windows->info_win, 7, (win_width - 5) / 2, "Level");
    mvwprintw(windows->info_win, 8, (win_width - 1) / 2, "%d", info->level);
  }
  wrefresh(windows->info_win);
}

/** @brief Освобождает память, выделенную для полей GameInfo_t. */
void free_game_info(GameInfo_t *info) {
  if (info->field) {
    for (int i = 0; i < FIELD_HEIGHT; i++) free(info->field[i]);
    free(info->field);
  }
  if (info->next) {
    for (int i = 0; i < TETR_SIZE; i++) free(info->next[i]);
    free(info->next);
  }
  info->field = NULL;
  info->next = NULL;
}

/** @brief Преобразует код клавиши в действие пользователя. */
UserAction_t get_action(int key) {
  switch (key) {
    case ENTER_KEY:
      return Start;
    case 'p':
    case 'P':
      return Pause;
    case 'q':
    case 'Q':
    case 27:
      return Terminate;
    case KEY_LEFT:
      return Left;
    case KEY_RIGHT:
      return Right;
    case KEY_DOWN:
      return Down;
    case SPACE_KEY:
      return Action;
    default:
      return Start;
  }
}

/** @brief Вычисляет количество цифр в числе для центрирования. */
int offset_counter(int number) {
  int count = 0;
  if (number == 0) return 1;
  while (number > 0) {
    count++;
    number /= 10;
  }
  return count;
}