# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra 
TEST_FLAGS = $(CFLAGS) -g

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
INC_DIR = include


# Source and object files
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

SRC_NO_MAIN = $(filter-out $(SRC_DIR)/main.c, $(SRC))


# Output executables
TARGET = $(BIN_DIR)/hcml
TEST_TARGET = $(BIN_DIR)/test

# Default target
all: $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJ) cutils
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(OBJ) lib/cutils/cutils.a -o $(TARGET) -lm
	@echo "Linking complete!"

# Compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c

	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled $<"


# Build test executable
$(TEST_TARGET): $(OBJ_NO_MAIN) cutils

	@mkdir -p $(BIN_DIR)
	$(CC) test/*.c $(SRC_NO_MAIN) lib/cutils/cutils.a -o $(TEST_TARGET) -lm
	@echo "Test binary built!"

# make the libs
cutils:
	@echo "Making cutils"
	@make -C lib/cutils
	@echo "cutils made"

# Build and run tests
test: $(TEST_TARGET)
	./$(TEST_TARGET) test all



# Clean build files
clean:
	rm -rf $(OBJ_DIR)/* $(BIN_DIR)/*

# Clean and rebuild
rebuild: clean all

# Clean, rebuild, and run tests
rebuild_test: clean test

.PHONY: all clean rebuild test
