# Camel Programming Language - Makefile

CC = cc
CFLAGS = -Wall -Wextra -std=c99 -O2
LDFLAGS =

SRC_DIR = src
BUILD_DIR = build

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))
TARGET = $(BUILD_DIR)/camel

# Default target
all: $(TARGET)

# Create build directory
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Compile object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	@echo "Built $(TARGET) successfully"

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

# Run REPL
run: $(TARGET)
	./$(TARGET)

# Debug build
debug: CFLAGS += -g -DDEBUG_TRACE_EXECUTION
debug: clean $(TARGET)

.PHONY: all clean run debug

