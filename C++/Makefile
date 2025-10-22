# Configurações
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = chip8

# Auto-detect all .cpp files in this directory (keeps Makefile maintenance low)
SOURCES := $(wildcard *.cpp)
OBJECTS := $(SOURCES:.cpp=.o)

# Discover SDL3 and SDL_ttf flags using pkg-config. CPPFLAGS supplies include
# flags for compilation, and SDL_LIBS supplies linker flags. If pkg-config
# cannot find the packages, you may set USER_SDL_FLAGS or SDL_LIBS in the env.
PKG_CPPFLAGS := $(shell pkg-config --cflags sdl3 SDL3_ttf 2>/dev/null)
PKG_LIBS := $(shell pkg-config --libs sdl3 SDL3_ttf 2>/dev/null)
CPPFLAGS ?= $(PKG_CPPFLAGS)
SDL_LIBS ?= $(PKG_LIBS) $(USER_SDL_FLAGS)

# Optional verbose flag: make V=1 to show full commands
V ?= 0
Q = $(if $(filter 1,$V),,@)

# Regra principal
all: $(TARGET)

# Criar executável
$(TARGET): $(OBJECTS)
	$(Q)$(CXX) $(OBJECTS) -o $(TARGET) $(SDL_LIBS)
	@echo "✅ Compilação concluída!"

# Compilar .cpp para .o
%.o: %.cpp
	$(Q)$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
	@echo "🔨 Compilando: $<"

# Executar
run: $(TARGET)
	./$(TARGET)

# Executar com ROM
run-rom: $(TARGET)
	./$(TARGET) $(ROM)

# Limpeza
demo: clean
	$(MAKE) BUILD_SDL_DEMO=1 SDL_LIBS="$(SDL_LIBS)" V=$(V) all

clean:
	rm -f $(OBJECTS) $(TARGET)
	@echo "🧹 Limpeza concluída"

# Targets que não são arquivos
.PHONY: all run run-rom clean demo