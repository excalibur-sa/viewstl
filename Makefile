CFLAGS_COMMON = -march=native -Wall -Wextra -pipe
CFLAGS = ${CFLAGS_COMMON} -fomit-frame-pointer  -O2
CFLAGS_DEBUG = ${CFLAGS_COMMON} -g
CC = gcc
LIBRARIES = -lglut -lGL -lGLU -lm

All: viewstl 

debug: CFLAGS = $(CFLAGS_DEBUG)
debug: viewstl

viewstl: viewstl.o
	$(CC) $(CFLAGS) -o $@ $< $(LIBRARIES)

rebuild: clean viewstl

clean:
	rm -v *.o viewstl
