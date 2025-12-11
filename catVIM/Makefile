CXX := clang++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -I./include
LDFLAGS := -llua

# Detect LuaJIT vs Lua
LUAJIT_CHECK := $(shell pkg-config --exists luajit 2>/dev/null && echo yes)
ifeq ($(LUAJIT_CHECK),yes)
    CXXFLAGS += $(shell pkg-config --cflags luajit)
    LDFLAGS := $(shell pkg-config --libs luajit)
else
    LUA_CHECK := $(shell pkg-config --exists lua5.4 2>/dev/null && echo yes)
    ifeq ($(LUA_CHECK),yes)
        CXXFLAGS += $(shell pkg-config --cflags lua5.4)
        LDFLAGS := $(shell pkg-config --libs lua5.4)
    endif
endif

SRC_DIR := src/core
OBJ_DIR := build
SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC))
TARGET := catvim

.PHONY: all clean install run

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/
	mkdir -p ~/.config/catvim
	cp -r src/lua/* ~/.config/catvim/
	cp -r runtime/* ~/.config/catvim/

run: $(TARGET)
	./$(TARGET)
