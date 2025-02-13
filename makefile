# Detect OS
ifeq ($(OS),Windows_NT)
	RM = rmdir /S /Q
	MKDIR = if not exist $(1) mkdir $(1)
	SDLFLAGS = -I$(SDL3)\include
	SDLLIBS = -L$(SDL3)\lib -lSDL3
	CXX = clang++.exe -target x86_64-pc-windows-msvc
	EXT = exe
	SHADER_SCRIPT = (cd assets/shaders/source && compile.bat)
else
	RM = rm -rf
	MKDIR = mkdir -p $(1)
	SDLFLAGS = `pkg-config sdl3 --cflags`
	SDLLIBS = `pkg-config sdl3 --libs`
	CXX = clang++
	EXT = out
	SHADER_SCRIPT = (cd assets/shaders/source && bash compile.sh)
endif

# Compiler and flags
# TODO: Remove sanitizers for release builds
# TODO: Add O3 optimization for release builds
SANITIZERS = # -fsanitize=address -fno-omit-frame-pointer -O1 -fno-optimize-sibling-calls
CXXFLAGS = -Wall -std=c++23 $(SDLFLAGS) -g -msse4.2 -mavx $(SANITIZERS)
LDFLAGS = $(SDLLIBS) $(SANITIZERS)

# Project structure
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BUILD_DIR = build
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))
TARGET = $(BUILD_DIR)/sdl3_app.$(EXT)

# Build target
all: $(TARGET)
# If on Windows, copy SDL3.dll to build directory
ifeq ($(OS),Windows_NT)
	if not exist $(BUILD_DIR)\SDL3.dll copy $(SDL3)\bin\SDL3.dll $(BUILD_DIR)
endif

$(TARGET): $(OBJS)
	$(call MKDIR,$(BUILD_DIR))
	$(CXX) -o $@ $^ $(LDFLAGS) -g

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(call MKDIR,$(OBJ_DIR))
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

# Precompile shaders
shaders: 
	$(SHADER_SCRIPT)

# Run target
run: $(TARGET)
	$(TARGET)

# Clean target
clean:
	$(RM) $(OBJ_DIR) $(BUILD_DIR)

.PHONY: all shaders run clean