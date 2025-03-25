# Static Library for the Cinder project
# Includes:
# 	- Glad, Khrplatform
# 	- Nholmann's JSON
# 	- stb_image
#   - ImGui Icons

CXX = clang++
CXXFLAGS = -Wall -std=c++23 -msse4.2 -mavx -O3 -DRELEASE -march=native -fomit-frame-pointer
LDFLAGS = 

# Project structure
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BUILD_DIR = .

SRCS = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/*/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))
TARGET = $(BUILD_DIR)/libcinderdeps.a

MAKEFLAGS += -j12
RM = rm -rf
MKDIR = mkdir -p $(1)

# Build target
all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "[MAKEFILE] Building 3rd party libraries..."
	@$(call MKDIR,$(BUILD_DIR))
	@ar rcs $@ $^
	@echo "[MAKEFILE] Build complete."

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@$(call MKDIR,$(dir $@))
	@$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@
	@echo "[MAKEFILE] Compiled $<"

# Clean target
clean:
	@echo "[MAKEFILE] Cleaning up..."
	@$(RM) $(OBJ_DIR) $(TARGET)
	@echo "[MAKEFILE] Clean complete."

.PHONY: all clean