CC=gcc
LIBDIR=./lib
LIBCGLM=$(LIBDIR)/cglm/libcglm.a
CFLAGS=-I./ -g -std=c99 -pedantic -Wall
CFLAGS_OPENGL=-I./$(LIBDIR)/cglm/include
LDFLAGS=-lGL -lGLU -lglut
LDFLAGS_OPENGL=-lm -lGL -lGLU -lglut $(LIBCGLM)
OUTDIR=.
BINDIR=$(OUTDIR)/bin
TTT_TERMINAL_OUTFILE=$(BINDIR)/ttt-terminal
TTT_OPENGL_OUTFILE=$(BINDIR)/ttt
RUN_TESTS=$(BINDIR)/run-tests
CGLM_VER=0.9.0
CGLM_PACKAGE=v${CGLM_VER}.tar.gz

.PHONY: all clean test ttt-terminal ttt-opengl dirs opengl_deps

all: dirs test ttt-terminal ttt-opengl

clean:
	rm -f $(BINDIR)/*
	rm -f $(RUN_TESTS)

clean-libs:
	rm -rf $(LIBDIR)/*

ttt-terminal: $(TTT_TERMINAL_OUTFILE)

ttt-opengl: $(TTT_OPENGL_OUTFILE)

$(TTT_TERMINAL_OUTFILE): dirs ttt-terminal.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TTT_TERMINAL_OUTFILE) ttt-terminal.c

$(TTT_OPENGL_OUTFILE): dirs opengl_deps glad/glad.o ttt-opengl.c
	$(CC) $(CFLAGS) $(CFLAGS_OPENGL) $(LDFLAGS) $(LDFLAGS_OPENGL) -o $(TTT_OPENGL_OUTFILE) ttt-opengl.c glad/glad.o

dirs:
	mkdir -p $(BINDIR)
	mkdir -p $(LIBDIR)

opengl_deps: $(LIBCGLM)

test: dirs $(RUN_TESTS)
	./$(RUN_TESTS)

$(RUN_TESTS): run-tests.c ttt.h
	$(CC) $(CFLAGS) -o $(RUN_TESTS) run-tests.c

glad/glad.o: glad/glad.c
	$(CC) -c $(CFLAGS) -o glad/glad.o glad/glad.c
