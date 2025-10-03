# Configurações
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = chip8
SOURCES = chip8.cpp main.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# Regra principal
all: $(TARGET)

# Criar executável
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET)
	@echo "✅ Compilação concluída!"

# Compilar .cpp para .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "🔨 Compilando: $<"

# Executar
run: $(TARGET)
	./$(TARGET)

# Executar com ROM
run-rom: $(TARGET)
	./$(TARGET) $(ROM)

# Limpeza
clean:
	rm -f $(OBJECTS) $(TARGET)
	@echo "🧹 Limpeza concluída"

# Targets que não são arquivos
.PHONY: all run run-rom clean