# ============================================================
# Overdrive — Makefile (Box2D v3)
# ============================================================

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -I include

# Box2D v3 en MSYS2: el paquete se llama mingw-w64-x86_64-box2d
# y linkea como -lbox2d (minúsculas)
LIBS := -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lbox2d

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

TARGET := $(BIN_DIR)/Overdrive.exe

SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC_FILES))

all: dirs $(TARGET)

dirs:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)

$(TARGET): $(OBJ_FILES)
	$(CXX) $(OBJ_FILES) -o $@ $(LIBS)
	@echo "Compilado: $(TARGET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: all
	$(TARGET)

clean:
	rm -f $(OBJ_DIR)/*.o $(TARGET)

.PHONY: all dirs run clean
