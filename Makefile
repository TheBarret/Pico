# PICO CPU Emulator - Build Configuration

CC      := gcc
CFLAGS  := -Wall -Wextra -std=c99 -pedantic -O2 -I./include -MMD -MP
LDFLAGS := -lm

SRC_DIR     := src
INCLUDE_DIR := include
BUILD_DIR   := build

# Core Engine Source Files (excluding test harnesses containing main)
CORE_SOURCES := $(SRC_DIR)/validation.c \
                $(SRC_DIR)/memory.c \
                $(SRC_DIR)/state.c \
                $(SRC_DIR)/decoder.c \
                $(SRC_DIR)/alu.c \
                $(SRC_DIR)/execute.c

CORE_OBJECTS := $(CORE_SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
TEST0_OBJ    := $(BUILD_DIR)/test.o

# Output Artifacts
LIBRARY  := $(BUILD_DIR)/libp8.a
TEST_BIN := $(BUILD_DIR)/test

.PHONY: all test clean verify layer0 layer1

# Default target builds static library and test binary
all: $(BUILD_DIR) $(LIBRARY) $(TEST_BIN)

# Creates target build folder if missing
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Builds static library from core modules
$(LIBRARY): $(CORE_OBJECTS)
	ar rcs $@ $^

# Links the Layer 0+1 test executable
$(TEST_BIN): $(TEST0_OBJ) $(LIBRARY)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Rule to compile any C source to object file
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Target to compile and run primitive layer test suite
test: $(TEST_BIN)
	@echo "MAKE: Running Tests..."
	@./$(TEST_BIN)

# Layer-specific target helpers
layer0: $(BUILD_DIR)/validation.o
layer1: $(BUILD_DIR)/memory.o $(BUILD_DIR)/state.o

# Dry-run validation check
verify:
	$(CC) $(CFLAGS) -fsyntax-only $(CORE_SOURCES) $(SRC_DIR)/test.c
	@echo "Finished"

clean:
	rm -rf $(BUILD_DIR)

# Include automatically generated dependency files (*.d)
-include $(BUILD_DIR)/*.d
