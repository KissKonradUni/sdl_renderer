ifeq ($(OS),Windows_NT)
	EXT = exe
	OPENGL = -lopengl32
	SHADER_SCRIPT = (cd assets/shaders/source && compile.bat)
# Copy from ..\win_dlls to bin using unix commands
	COPY_DLLS = cp -r ./win_dlls/* bin/
else
	EXT = out
	OPENGL = -lGL
	SHADER_SCRIPT = (cd assets/shaders/source && bash compile.sh)
endif

RM = rm -rf
MKDIR = mkdir -p $(1)

SDLFLAGS = `pkg-config sdl3 --cflags`
SDLLIBS = `pkg-config sdl3 --libs`

ASSIMPFLAGS = `pkg-config assimp --cflags`
ASSIMPLIBS = `pkg-config assimp --libs`

GLAD = -Iinclude/glad -Iinclude/KHR

CXX = clang++

# Compiler and flags
# TODO: Remove sanitizers for release builds
# TODO: Add O3 optimization for release builds
SANITIZERS = -O1 # -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls

CXXFLAGS = -Wall -std=c++23 -g $(SDLFLAGS) $(ASSIMPFLAGS) -msse4.2 -mavx $(SANITIZERS)
LDFLAGS = $(SDLLIBS) $(ASSIMPLIBS) $(OPENGL) $(SANITIZERS)

# Project structure
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BUILD_DIR = bin

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))
TARGET = $(BUILD_DIR)/sdl3_app.$(EXT)

# Build target
all: $(TARGET)

$(TARGET): $(OBJS)
	@$(call MKDIR,$(BUILD_DIR))
	@echo "[MAKEFILE] Building target..."
	@$(CXX) -o $@ $^ $(LDFLAGS) -g
	@echo "[MAKEFILE] Build complete."
ifdef COPY_DLLS
# Print message to copy dlls
	@echo "[MAKEFILE] (Windows) Copying dlls to bin directory..."
	@$(COPY_DLLS)
endif

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@$(call MKDIR,$(OBJ_DIR))
	@$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@
	@echo "[MAKEFILE] Compiled $<"

# Run target
run: $(TARGET)
	@echo "[MAKEFILE] Running target..."
	$(TARGET)

# Clean target
clean:
	@echo "[MAKEFILE] Cleaning up..."
	@$(RM) $(OBJ_DIR) $(BUILD_DIR)

.PHONY: all run clean