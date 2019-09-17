CFLAGS_COMMON = -march=native -Wall -Wextra -pipe
CFLAGS = ${CFLAGS_COMMON} -fomit-frame-pointer  -O2
CFLAGS_DEBUG = ${CFLAGS_COMMON} -g
CC = gcc
LIBRARIES = -lglut -lGL -lGLU -lm

src = $(wildcard *.c)
obj = $(src:.c=.o)

All: viewstl 

debug: CFLAGS = $(CFLAGS_DEBUG)
debug: viewstl

viewstl: $(obj)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBRARIES)

rebuild: clean viewstl

clean:
	rm -v $(obj) viewstl
