# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -I./include
TEST_FLAGS = $(CFLAGS) -g

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
INC_DIR = include


# Source and object files
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Exclude main.c for test builds
SRC_NO_MAIN = $(filter-out $(SRC_DIR)/main.c, $(SRC))

LIBS = cutils

# Output executables
TARGET = $(BIN_DIR)/hcml
TEST_TARGET = $(BIN_DIR)/test

# Default target
all: $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $(OBJ) -o $(TARGET)
	@echo "Linking complete!"

# Compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled $<"

# Compile test source files
$(OBJ_DIR)/%.o: $(TEST_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(TEST_FLAGS) -c $< -o $@
	@echo "Compiled $<"

# make the libs
libs:
	@echo "Making libs : $(LIBS)"

	@for lib in $(LIBS); do \
	    cp -f lib/$$lib/$$lib.h include/; \
	    $(MAKE) -C lib/$$lib/; \
	done

# Build and run tests
test: $(TEST_TARGET)
	./$(TEST_TARGET)

# Build test executable
$(TEST_TARGET): $(SRC_NO_MAIN)
	@mkdir -p $(BIN_DIR)
	$(CC) test.c $^ -o $(TEST_TARGET)
	@echo "Test binary built!"

# Clean build files
clean:
	rm -rf $(OBJ_DIR)/* $(BIN_DIR)/*

# Clean and rebuild
rebuild: clean all

# Clean, rebuild, and run tests
rebuild_test: clean test

.PHONY: all clean rebuild test
