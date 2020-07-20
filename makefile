CXX      := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -g $(shell pkg-config --cflags sdl2)
LDFLAGS  := $(shell pkg-config --libs sdl2)
BUILD    := ./build
OBJ_DIR  := $(BUILD)/obj
TARGET   := gb
INCLUDE  := -Isrc/ -Iinclude/
SRC      :=               \
    $(wildcard src/*.cpp) \

OBJECTS  := $(SRC:%.cpp=$(OBJ_DIR)/%.o)

all: build $(TARGET)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@ $(LDFLAGS)

$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $^ $(LDFLAGS)

.PHONY: all build clean debug release

build:
	@mkdir -p $(OBJ_DIR)

debug: CXXFLAGS += -DDEBUG -g
debug: all

release: CXXFLAGS += -O2
release: all

clean:
	-@rm -rvf $(TARGET)
	-@rm -rvf $(OBJ_DIR)/*
