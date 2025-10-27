CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2
TARGET := chip8

# Sources / objects
SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.o)

# Detect available SDL package (prefer sdl2, fallback to sdl3)
PKG := $(shell if pkg-config --exists sdl2 2>/dev/null; then echo sdl2; elif pkg-config --exists sdl3 2>/dev/null; then echo sdl3; fi)
PKG_CFLAGS := $(shell pkg-config --cflags $(PKG) 2>/dev/null)
PKG_LIBS := $(shell pkg-config --libs $(PKG) 2>/dev/null)

CPPFLAGS += $(PKG_CFLAGS)
LDFLAGS += $(PKG_LIBS)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

run-rom: $(TARGET)
	./$(TARGET) $(ROM)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all run run-rom clean