EXE = main
BIN = bin
SRC = src

CC = gcc
CXX = g++

WARNINGS = -Wall -Wextra -Wno-unused-parameter

CFLAGS = -g $(WARNINGS) -std=c99 -DGL_GLEXT_PROTOTYPES `sdl-config --cflags`
CXXFLAGS = -g $(WARNINGS) -std=c++0x -DGL_GLEXT_PROTOTYPES `sdl-config --cflags`


LIB = GL m X11

LDXFLAGS = $(addprefix -l,$(LIB)) `sdl-config --libs`

CSOURCES = $(wildcard $(SRC)/*.c) $(wildcard $(SRC)/common/*.c)
CXSOURCES = $(wildcard $(SRC)/*.cpp) $(wildcard $(SRC)/common/*.cpp)

COBJ = $(patsubst $(SRC)/%.c,$(BIN)/%.o, $(CSOURCES))
CXOBJ = $(patsubst $(SRC)/%.cpp,$(BIN)/%.o, $(CXSOURCES))

OBJ = $(COBJ) $(CXOBJ)

COMMONDIR = $(SRC)/common

all: bin $(EXE)

$(BIN)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $? -o $@

$(BIN)/%.o: $(SRC)/%.cpp
	$(CXX) $(CXXFLAGS) -c $? -o $@

main: $(OBJ)
	$(CXX) $^ -o $@ $(LDXFLAGS)

bin:
	mkdir -p bin/common

clean:
	rm -r $(OBJ)

cleanall:
	rm -r $(EXE) $(OBJ)
