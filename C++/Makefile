
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2
TARGET := chip8

# Sources / objects: search for .cpp files (including src/ subdir)
# We limit depth to 2 to avoid searching too deep or other folders
SRCS := $(shell find . -maxdepth 2 -name '*.cpp' -print)
ifeq ($(strip $(SRCS)),)
$(error No .cpp sources found (looked with find . -maxdepth 2 -name '*.cpp'))
endif
OBJS := $(SRCS:.cpp=.o)

# Detect available SDL package (prefer sdl2, fallback to sdl3)
PKG := $(shell if pkg-config --exists sdl2 2>/dev/null; then echo sdl2; elif pkg-config --exists sdl3 2>/dev/null; then echo sdl3; fi)
PKG_CFLAGS := $(shell if [ -n "$(PKG)" ]; then pkg-config --cflags $(PKG) 2>/dev/null; fi)
PKG_LIBS := $(shell if [ -n "$(PKG)" ]; then pkg-config --libs $(PKG) 2>/dev/null; fi)

# Include project headers (lib/) and SDL cflags
CPPFLAGS += $(PKG_CFLAGS) -I./lib
LDFLAGS += $(PKG_LIBS)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# pattern rule: compile .cpp -> .o (handles paths like ./src/main.cpp)
%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

run-rom: $(TARGET)
	./$(TARGET) $(ROM)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all run run-rom clean