# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -lm
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



LIBS = cutils

LIBS_A = $(addsuffix .a, $(addprefix lib/$(LIBS)/, $(LIBS)))

# Output executables
TARGET = $(BIN_DIR)/hcml
TEST_TARGET = $(BIN_DIR)/test

# Default target
all: $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS)  $(OBJ) $(LIBS_A) -o $(TARGET)
	@echo "Linking complete!"

# Compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c

	@echo "Making libs : $(LIBS_A)"

	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< $(LIBS_A) -o $@
	@echo "Compiled $<"


# Build test executable
$(TEST_TARGET): $(OBJ_NO_MAIN) libs

	@echo "Using libs : $(LIBS_A)" 

	@mkdir -p $(BIN_DIR)
	$(CC) test/*.c -lm $(SRC_NO_MAIN) $(LIBS_A) -o $(TEST_TARGET)
	@echo "Test binary built!"

# make the libs
libs:
	@echo "Making libs : $(LIBS)"

	@for lib in $(LIBS); do \
	    cp -f lib/$$lib/$$lib.h include/; \
	    $(MAKE) -C lib/$$lib/; \
	done

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
