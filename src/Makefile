CC = gcc
CFLAGS = -Wall -Werror -Wextra -std=c11 -Ibrick_game
LDFLAGS = -lncurses
CHECK_FLAGS = $(shell pkg-config --cflags --libs check)
GCOV_FLAGS = -fprofile-arcs -ftest-coverage
SRC_DIR = .
BRICK_GAME_DIR = $(SRC_DIR)/brick_game
GUI_DIR = $(SRC_DIR)/gui
TEST_DIR = $(SRC_DIR)/tests
OBJ_DIR = obj
BIN_DIR = $(SRC_DIR)/bin
INSTALL_DIR = $(SRC_DIR)/s21_tetris
DOCS_DIR = docs/html

SOURCES = $(BRICK_GAME_DIR)/backend.c $(GUI_DIR)/front.c
OBJECTS = $(OBJ_DIR)/backend.o $(OBJ_DIR)/front.o
TEST_SOURCES = $(TEST_DIR)/test_backend.c
TEST_OBJECTS = $(OBJ_DIR)/test_backend.o $(OBJ_DIR)/backend_test.o
EXEC = $(BIN_DIR)/tetris
TEST_EXEC = $(BIN_DIR)/test
INSTALLED_EXEC = $(INSTALL_DIR)/tetris

all: $(EXEC)

$(EXEC): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(OBJECTS) -o $(EXEC) $(LDFLAGS)

$(OBJ_DIR)/backend.o: $(BRICK_GAME_DIR)/backend.c $(BRICK_GAME_DIR)/backend.h $(BRICK_GAME_DIR)/brick_game.h
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/backend_test.o: $(BRICK_GAME_DIR)/backend.c $(BRICK_GAME_DIR)/backend.h $(BRICK_GAME_DIR)/brick_game.h
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(GCOV_FLAGS) -c $< -o $@

$(OBJ_DIR)/front.o: $(GUI_DIR)/front.c $(GUI_DIR)/front.h $(BRICK_GAME_DIR)/brick_game.h
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/test_backend.o: $(TEST_DIR)/test_backend.c $(BRICK_GAME_DIR)/backend.h $(BRICK_GAME_DIR)/brick_game.h
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(CHECK_FLAGS) -c $< -o $@

test: $(TEST_EXEC)
	./$(TEST_EXEC)

valgrind_test: $(TEST_EXEC)
	@valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out.txt ./$(TEST_EXEC)
	@echo "Valgrind report generated in valgrind-out.txt"
	@grep -E "ERROR SUMMARY|definitely lost|indirectly lost|possibly lost" valgrind-out.txt || echo "No memory leaks detected"

$(TEST_EXEC): $(TEST_OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(TEST_OBJECTS) -o $(TEST_EXEC) $(CHECK_FLAGS) $(GCOV_FLAGS)

gcov_report: test
	@lcov --capture --directory . --output-file coverage.info
	@genhtml coverage.info --output-directory coverage_report
	@xdg-open coverage_report/index.html 2>/dev/null || \
	 open coverage_report/index.html 2>/dev/null || \
	 echo "Could not open coverage report automatically. Please open coverage_report/index.html manually"

docs: doxygen open-docs

doxygen:
	doxygen Doxyfile

open-docs:
	@xdg-open $(DOCS_DIR)/index.html 2>/dev/null || \
	 open $(DOCS_DIR)/index.html 2>/dev/null || \
	 echo "Could not open docs automatically. Please open $(DOCS_DIR)/index.html manually"

install: $(EXEC)
	@mkdir -p $(INSTALL_DIR)
	@cp $(EXEC) $(INSTALLED_EXEC)

clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR) $(INSTALL_DIR) docs coverage_report coverage.info *.gcno *.gcda score.txt valgrind-out.txt

dvi: docs

dist:
	@tar -czf tetris.tar.gz $(SRC_DIR) Makefile

.PHONY: all install clean dvi dist test gcov_report docs doxygen open-docs valgrind_test