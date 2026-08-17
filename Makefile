# PICO8 Build script

CC      := gcc
CFLAGS  := -Wall -Wextra -std=c99 -pedantic -O2 -I./include -MMD -MP
LDFLAGS := -lm

SRC_DIR     := src
INCLUDE_DIR := include
BUILD_DIR   := build
DEV_DIR     := devices

CORE_SOURCES := $(SRC_DIR)/validation.c \
                $(SRC_DIR)/memory.c \
                $(SRC_DIR)/state.c \
                $(SRC_DIR)/decoder.c \
                $(SRC_DIR)/alu.c \
                $(SRC_DIR)/execute.c \
                $(SRC_DIR)/host.c \
                $(SRC_DIR)/harness.c \
                $(SRC_DIR)/bus.c \
                $(DEV_DIR)/stddev.c \
                $(SRC_DIR)/main.c


# Map C source files to object files in build/
CORE_OBJECTS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(notdir $(CORE_SOURCES)))

# Driver object files
TEST_OBJ := $(BUILD_DIR)/test.o
MAIN_OBJ := $(BUILD_DIR)/main.o

# Output Artifacts
LIBRARY  := $(BUILD_DIR)/libp8.a
MAIN_BIN := $(BUILD_DIR)/pico8
TEST_BIN := $(BUILD_DIR)/test


.PHONY: all test run clean verify

# Default target builds the library, test runner, and main application binary
all: $(BUILD_DIR) $(LIBRARY) $(TEST_BIN) $(MAIN_BIN)

# Creates target build directory if missing
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Builds static library from core modules
$(LIBRARY): $(CORE_OBJECTS)
	ar rcs $@ $^

# Link Unit Test Binary
$(TEST_BIN): $(TEST_OBJ) $(LIBRARY)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Link Main Executable Binary
$(MAIN_BIN): $(MAIN_OBJ) $(LIBRARY)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Generic rule for src/*.c files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Generic rule for devices/*.c files
$(BUILD_DIR)/%.o: $(DEV_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_BIN)
	@echo "MAKE: Running Unit Tests..."
	@./$(TEST_BIN)

run: $(MAIN_BIN)
	@echo "MAKE: Running Pico8 runtime..."
	@./$(MAIN_BIN)

verify:
	$(CC) $(CFLAGS) -fsyntax-only $(CORE_SOURCES) $(SRC_DIR)/test.c $(SRC_DIR)/main.c
	@echo "Syntax verification passed."

clean:
	rm -rf $(BUILD_DIR)

# Include automatically generated dependency files (*.d)
-include $(BUILD_DIR)/*.d
