/**
 * @file backend.h
 * @brief Заголовочный файл для внутренней логики игры "Тетрис".
 *
 * Содержит определения констант, перечислений, структур и прототипов
 * функций для управления состоянием игры и обработки игровой логики.
 */

#ifndef BACKEND_H
#define BACKEND_H

#include <stdbool.h>

#include "brick_game.h"

/** @brief Ширина игрового поля (10 пикселей). */
#define FIELD_WIDTH 10
/** @brief Высота игрового поля (20 пикселей). */
#define FIELD_HEIGHT 20
/** @brief Размер фигуры (4x4). */
#define TETR_SIZE 4
/** @brief Пустая клетка на поле. */
#define EMPTY 0
/** @brief Заполненная клетка (блок фигуры). */
#define BLOCK 1
/** @brief Код выхода из игры. */
#define EXIT_GAME -1

/**
 * @enum TetraminoType_t
 * @brief Типы фигур (тетрамино).
 *
 * Перечисляет все возможные формы фигур в игре "Тетрис".
 */
typedef enum {
  O_tetramino, /**< Квадрат (O-фигура). */
  I_tetramino, /**< Линия (I-фигура). */
  T_tetramino, /**< Т-образная фигура. */
  S_tetramino, /**< S-образная фигура. */
  Z_tetramino, /**< Z-образная фигура. */
  J_tetramino, /**< J-образная фигура. */
  L_tetramino  /**< L-образная фигура. */
} TetraminoType_t;

/**
 * @enum FiniteState_t
 * @brief Состояния конечного автомата игры.
 *
 * Определяет возможные состояния игры для управления логикой.
 */
typedef enum {
  Start_state, /**< Ожидание начала игры. */
  Spawn,       /**< Создание новой фигуры. */
  Moving,      /**< Перемещение фигуры пользователем. */
  Shifting,    /**< Автоматический сдвиг фигуры вниз. */
  Attaching,   /**< Присоединение фигуры к полю. */
  Game_over,   /**< Конец игры. */
  Pause_state  /**< Пауза. */
} FiniteState_t;

/**
 * @struct GameState
 * @brief Внутреннее состояние игры.
 *
 * Содержит все данные, необходимые для управления текущей игрой.
 */
typedef struct {
  FiniteState_t state;      /**< Текущее состояние конечного автомата. */
  UserAction_t user_action; /**< Последнее действие пользователя. */
  bool hold;                /**< Флаг удержания клавиши. */
  int field[FIELD_HEIGHT][FIELD_WIDTH]; /**< Игровое поле. */
  int next[TETR_SIZE][TETR_SIZE];       /**< Следующая фигура. */
  TetraminoType_t next_type;            /**< Тип следующей фигуры. */
  int current[TETR_SIZE][TETR_SIZE];    /**< Текущая фигура. */
  TetraminoType_t current_type;         /**< Тип текущей фигуры. */
  int x_position;                       /**< Горизонтальная позиция фигуры. */
  int y_position;                       /**< Вертикальная позиция фигуры. */
  int score;                            /**< Текущий счет. */
  int high_score;                       /**< Рекорд. */
  int level;                            /**< Уровень игры. */
  int pause;                            /**< Состояние паузы. */
  long long timer; /**< Таймер для автоматического сдвига. */
} GameState;

/**
 * @brief Получает указатель на состояние игры.
 * @return Указатель на структуру GameState.
 */
GameState *get_game_state();

/**
 * @brief Выполняет действия в зависимости от текущего состояния.
 * @param state Указатель на состояние игры.
 */
void run_actions_by_state(GameState *state);

/**
 * @brief Инициализирует игру.
 * @param state Указатель на состояние игры.
 */
void initialize_game(GameState *state);

/**
 * @brief Создает новую фигуру.
 * @param state Указатель на состояние игры.
 */
void spawn_tetramino(GameState *state);

/**
 * @brief Обрабатывает перемещение фигуры пользователем.
 * @param state Указатель на состояние игры.
 */
void move_tetramino(GameState *state);

/**
 * @brief Сдвигает фигуру вниз по таймеру.
 * @param state Указатель на состояние игры.
 */
void shift_tetramino(GameState *state);

/**
 * @brief Присоединяет фигуру к игровому полю.
 * @param state Указатель на состояние игры.
 */
void attach_tetramino(GameState *state);

/**
 * @brief Обрабатывает действия во время паузы.
 * @param state Указатель на состояние игры.
 */
void pause_actions(GameState *state);

/**
 * @brief Обрабатывает действия при завершении игры.
 * @param state Указатель на состояние игры.
 */
void game_over_actions(GameState *state);

/**
 * @brief Проверяет и удаляет заполненные линии.
 * @param state Указатель на состояние игры.
 */
void calculate_lines(GameState *state);

/**
 * @brief Удаляет заполненную линию.
 * @param state Указатель на состояние игры.
 * @param line Номер линии для удаления.
 */
void clear_line(GameState *state, int line);

/**
 * @brief Обновляет счет игрока.
 * @param state Указатель на состояние игры.
 * @param lines_cleared Количество удаленных линий.
 */
void update_score(GameState *state, int lines_cleared);

/**
 * @brief Обновляет уровень и скорость игры.
 * @param state Указатель на состояние игры.
 */
void update_speed_and_level(GameState *state);

/**
 * @brief Генерирует следующую фигуру.
 * @param state Указатель на состояние игры.
 * @return Тип новой фигуры.
 */
TetraminoType_t generate_next_tetramino(GameState *state);

/**
 * @brief Заполняет массив фигуры в зависимости от типа.
 * @param tetramino Массив 4x4 для фигуры.
 * @param type Тип фигуры.
 */
void fill_tetramino(int tetramino[TETR_SIZE][TETR_SIZE], TetraminoType_t type);

/**
 * @brief Проверяет столкновение фигуры с полем или границами.
 * @param state Указатель на состояние игры.
 * @param tetramino Массив 4x4 текущей фигуры.
 * @param x Горизонтальная позиция.
 * @param y Вертикальная позиция.
 * @return true, если есть столкновение, иначе false.
 */
bool check_collision(GameState *state, int tetramino[TETR_SIZE][TETR_SIZE],
                     int x, int y);

/**
 * @brief Поворачивает текущую фигуру.
 * @param state Указатель на состояние игры.
 */
void rotate_tetramino(GameState *state);

/**
 * @brief Обновляет таймер игры.
 * @return Текущее время в миллисекундах.
 */
long long update_timer();

/**
 * @brief Сохраняет рекорд в файл.
 * @param high_score Значение рекорда.
 */
void write_score(int high_score);

/**
 * @brief Читает рекорд из файла.
 * @return Значение рекорда.
 */
int read_score();

#endif