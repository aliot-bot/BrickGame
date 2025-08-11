/**
 * @file front.h
 * @brief Заголовочный файл для терминального интерфейса игры "Тетрис".
 *
 * Содержит определения структур и прототипов функций для отрисовки
 * игрового поля и обработки пользовательского ввода с использованием ncurses.
 */

#ifndef FRONT_H
#define FRONT_H

#include <ncurses.h>
#include <stdlib.h>

#include "../brick_game/backend.h"

/** @brief Код клавиши Enter. */
#define ENTER_KEY 10
/** @brief Код клавиши пробела. */
#define SPACE_KEY ' '

/**
 * @struct Interface_t
 * @brief Интерфейсные окна игры.
 *
 * Содержит указатели на окна ncurses для игрового поля,
 * следующей фигуры и информационной панели.
 */
typedef struct {
  WINDOW *game_win; /**< Окно игрового поля. */
  WINDOW *next_win; /**< Окно следующей фигуры. */
  WINDOW *info_win; /**< Окно информации (счет, рекорд, уровень). */
} Interface_t;

/**
 * @brief Запускает основной игровой цикл.
 *
 * Инициализирует интерфейс, обрабатывает ввод и обновляет экран.
 */
void run_game_loop();

/**
 * @brief Отрисовывает игровое поле.
 * @param info Указатель на информацию о состоянии игры.
 * @param windows Указатель на интерфейсные окна.
 */
void print_field(const GameInfo_t *info, Interface_t *windows);

/**
 * @brief Отрисовывает следующую фигуру.
 * @param info Указатель на информацию о состоянии игры.
 * @param windows Указатель на интерфейсные окна.
 */
void print_next(const GameInfo_t *info, Interface_t *windows);

/**
 * @brief Отрисовывает информационную панель.
 * @param info Указатель на информацию о состоянии игры.
 * @param windows Указатель на интерфейсные окна.
 */
void print_info(const GameInfo_t *info, Interface_t *windows);

/**
 * @brief Освобождает память, выделенную для GameInfo_t.
 * @param info Указатель на информацию о состоянии игры.
 */
void free_game_info(GameInfo_t *info);

/**
 * @brief Преобразует код клавиши в действие пользователя.
 * @param key Код нажатой клавиши.
 * @return Действие пользователя (UserAction_t).
 */
UserAction_t get_action(int key);

/**
 * @brief Вычисляет количество цифр в числе для центрирования текста.
 * @param number Число для подсчета.
 * @return Количество цифр.
 */
int offset_counter(int number);

#endif