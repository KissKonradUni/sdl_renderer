ifeq ($(OS),Windows_NT)
	EXT = exe
	OPENGL = -lopengl32
# Copy from ..\win_dlls to bin using unix commands
	COPY_DLLS = cp -r ./win_dlls/* bin/
	SKIP_ASAN = 1
else
	EXT = out
	OPENGL = -lGL
endif

RM = rm -rf
MKDIR = mkdir -p $(1)

SDLFLAGS = `pkg-config sdl3 --cflags`
SDLLIBS = `pkg-config sdl3 --libs`

ASSIMPFLAGS = `pkg-config assimp --cflags`
ASSIMPLIBS = `pkg-config assimp --libs`

# Specify the path to the imgui library
IMGUI_PATH = ../imgui

IMGUI_SRCS = $(IMGUI_PATH)/imgui.cpp \
			 $(IMGUI_PATH)/imgui_demo.cpp \
			 $(IMGUI_PATH)/imgui_draw.cpp \
			 $(IMGUI_PATH)/imgui_tables.cpp \
			 $(IMGUI_PATH)/imgui_widgets.cpp \
			 $(IMGUI_PATH)/backends/imgui_impl_sdl3.cpp \
			 $(IMGUI_PATH)/backends/imgui_impl_opengl3.cpp
IMGUI_OBJS = $(patsubst $(IMGUI_PATH)/%.cpp, $(IMGUI_PATH)/obj/%.o, $(IMGUI_SRCS))
IMGUI_FLAGS = -I$(IMGUI_PATH) -I$(IMGUI_PATH)/backends

THIRD_PARTY_FOLDER = ./lib
THIRD_PARTY_FLAGS = -Ilib/include
THIRD_PARTY_LIBS = -L./lib -lcinderdeps

CXX = clang++

# Compiler and flags
BUILD_MODE ?= debug
SANITIZE ?= address
ifeq ($(SKIP_ASAN),1)
	DEBUG_FLAGS = -O0 -g -fno-omit-frame-pointer -fno-optimize-sibling-calls \
			  	  -Wall -DDEBUG
else
	DEBUG_FLAGS = -O0 -g -fsanitize=$(SANITIZE) -fno-omit-frame-pointer -fno-optimize-sibling-calls \
			  	  -Wall -DDEBUG
endif

RELEASE_FLAGS = -O3 -DRELEASE -march=native -flto -fomit-frame-pointer

ifeq ($(BUILD_MODE),debug)
	BUILD_FLAGS = $(DEBUG_FLAGS)
else
	BUILD_FLAGS = $(RELEASE_FLAGS)
endif

CXXFLAGS = -Wall -std=c++23 -msse4.2 -mavx $(BUILD_FLAGS) $(SDLFLAGS) $(ASSIMPFLAGS) $(IMGUI_FLAGS) $(THIRD_PARTY_FLAGS)
LDFLAGS = $(BUILD_FLAGS) $(OPENGL) $(SDLLIBS) $(ASSIMPLIBS) $(THIRD_PARTY_LIBS)

# Project structure
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BUILD_DIR = bin

SRCS = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/*/*.cpp) $(wildcard $(SRC_DIR)/*/*/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS)) $(IMGUI_OBJS)
TARGET = $(BUILD_DIR)/sdl3_app.$(EXT)

MAKEFLAGS += -j12

# Build target
all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "[MAKEFILE] Building target in $(BUILD_MODE) mode..."
	@$(call MKDIR,$(BUILD_DIR))
	@$(CXX) -o $@ $^ $(LDFLAGS) -g
	@echo "[MAKEFILE] Build complete."
ifdef COPY_DLLS
# Print message to copy dlls
	@echo "[MAKEFILE] (Windows) Copying dlls to bin directory..."
	@$(COPY_DLLS)
endif

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@$(call MKDIR,$(dir $@))
	@$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@
	@echo "[MAKEFILE] Compiled $<"

$(IMGUI_PATH)/obj/%.o: $(IMGUI_PATH)/%.cpp
	@$(call MKDIR,$(dir $@))
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

# Clean imgui objects
clean-imgui:
	@echo "[MAKEFILE] Cleaning up imgui objects..."
	@$(RM) $(IMGUI_PATH)/obj

.PHONY: all run clean