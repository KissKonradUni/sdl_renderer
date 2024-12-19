# Detect OS
ifeq ($(OS),Windows_NT)
	RM = rmdir /S /Q
	MKDIR = if not exist $(1) mkdir $(1)
	SDLFLAGS = -I$(SDL3)\include
	SDLLIBS = -L$(SDL3)\lib -lSDL3
	CXX = clang++.exe -target x86_64-pc-windows-msvc
	EXT = exe
else
	RM = rm -rf
	MKDIR = mkdir -p $(1)
	SDLFLAGS = `pkg-config sdl3 --cflags`
	SDLLIBS = `pkg-config sdl3 --libs`
	CXX = clang++
	EXT = out
endif

# Compiler and flags
CXXFLAGS = -Wall -std=c++23 $(SDLFLAGS)
LDFLAGS = $(SDLLIBS)

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

$(TARGET): $(OBJS)
	$(call MKDIR,$(BUILD_DIR))
	$(CXX) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(call MKDIR,$(OBJ_DIR))
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

# Precompile shaders
shaders: 
	(cd assets/shaders/source && bash compile.sh)

# Run target
run: $(TARGET)
# If on windows copy the SDL3.dll to the build directory
ifeq ($(OS),Windows_NT)
	copy $(SDL3)\bin\SDL3.dll $(BUILD_DIR)
endif
	$(TARGET)

# Clean target
clean:
	$(RM) $(OBJ_DIR) $(BUILD_DIR)

# Phony targets
.PHONY: all clean