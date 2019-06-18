CFLAGS = -fomit-frame-pointer -march=native -Wall -pipe -O2
CFLAGS_DEBUG = -march=native -g -Wall -pipe
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
