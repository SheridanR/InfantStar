#################################################################################
#	
#	INFANT STAR
#	File: mingw32 makefile
#	Desc: The Infant Star makefile, mingw32 version
#	
#	Copyright 2013 (c) Sheridan Rathbun, all rights reserved.
#	See LICENSE for details.
#	
#################################################################################

CC=gcc
CFLAGS=-Wall -O3 -ffast-math -funroll-loops -malign-double -fstrict-aliasing
INCLUDE=-I/usr/include/SDL
LIB1=-L/usr/lib
LIB2=-L/usr/local/lib

all: init.o buttons.o behaviors.o cursors.o draw.o objects.o list.o files.o game.o editor.o
	$(CC) $(CFLAGS) $(INCLUDE) game.o init.o list.o objects.o cursors.o draw.o behaviors.o files.o -o game.exe $(LIB1) -lmingw32 -lsprig $(LIB2) -lSDLmain -lSDL -lSDL_mixer -mwindows -lpthread
	$(CC) $(CFLAGS) $(INCLUDE) editor.o init.o list.o objects.o cursors.o draw.o buttons.o files.o -o editor.exe $(LIB1) -lmingw32 -lsprig $(LIB2) -lSDLmain -lSDL -lSDL_mixer -mwindows -lpthread
	make clean
	
game: init.o behaviors.o cursors.o draw.o objects.o list.o game.o
	$(CC) $(CFLAGS) $(INCLUDE) game.o init.o list.o objects.o cursors.o draw.o behaviors.o files.o -o game.exe $(LIB1) -lmingw32 -lsprig $(LIB2) -lSDLmain -lSDL -lSDL_mixer -mwindows -lpthread
	make clean
	
editor: init.o buttons.o cursors.o draw.o objects.o list.o editor.o
	$(CC) $(CFLAGS) $(INCLUDE) editor.o init.o list.o objects.o cursors.o draw.o buttons.o files.o -o editor.exe $(LIB1) -lmingw32 -lsprig $(LIB2) -lSDLmain -lSDL -lSDL_mixer -mwindows -lpthread
	make clean

editor.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c src/editor.c -o editor.o
	
game.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c src/game.c -o game.o
	
init.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c src/init.c -o init.o
	
list.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c src/list.c -o list.o
	
objects.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c src/objects.c -o objects.o
	
draw.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c src/draw.c -o draw.o
	
behaviors.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c src/behaviors.c -o behaviors.o
	
buttons.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c src/buttons.c -o buttons.o
	
files.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c src/files.c -o files.o
	
cursors.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c src/cursors.c -o cursors.o
	
clean:
	rm *.o