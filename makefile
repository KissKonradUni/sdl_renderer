# Compiler and flags
CXX = clang++
CXXFLAGS = -Wall -std=c++23 `pkg-config sdl3 --cflags`
LDFLAGS = `pkg-config sdl3 --libs`

# Project structure
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BUILD_DIR = build
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))
TARGET = $(BUILD_DIR)/my_sdl3_app

# Build target
all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

# Run target
run: $(TARGET)
	$(TARGET)

# Clean target
clean:
	rm -rf $(OBJ_DIR) $(BUILD_DIR)

# Phony targets
.PHONY: all clean