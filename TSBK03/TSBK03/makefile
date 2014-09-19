EXE = main
BIN = bin

CC = gcc
CXX = g++

WARNINGS = -Wall -Wextra -Wno-unused-parameter # Till att börja med, är rädd för Ingemars kod

CFLAGS = -g $(WARNINGS) -std=c99 -DGL_GLEXT_PROTOTYPES
CXXFLAGS = -g $(WARNINGS) -std=c++0x -DGL_GLEXT_PROTOTYPES

LIB = GL m glut X11

LDXFLAGS = $(addprefix -l,$(LIB))

CSOURCES = common/GL_utilities.c common/VectorUtils3.c common/zpr.c
CXSOURCES = common/LoadTGA.cpp common/loadobj.cpp main.cpp

COBJ = $(patsubst %.c,$(BIN)/%.o, $(CSOURCES))
CXOBJ = $(patsubst %.cpp,$(BIN)/%.o, $(CXSOURCES))

OBJ = $(COBJ) $(CXOBJ)

all: bin $(EXE)

$(BIN)/%.o: %.c
	$(CC) $(CFLAGS) -c $? -o $@

$(BIN)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $? -o $@

main: $(OBJ)
	$(CXX) $^ -o $@ $(LDXFLAGS)

bin:
	mkdir -p bin/common

clean:
	rm $(OBJ)

cleanall:
	rm $(EXE) $(OBJ)
