
CC=mingw32-gcc
CFLAGS=-lmingw32 -lSDL2main -lSDL2

all:
	$(CC) src/*.c $(CFLAGS) -o out/main.exe

clean:
	rm -rf out/main.exe
