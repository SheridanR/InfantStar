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
CFLAGS=-Wall -O3 -g -ffast-math -funroll-loops -malign-double -fstrict-aliasing
INCLUDE=-I/usr/include/SDL
LIB1=-L/usr/lib
LIB2=-L/usr/local/lib

all: game clean
	
game: behaviors.o draw.o objects.o list.o game.o
	$(CC) $(CFLAGS) $(INCLUDE) game.o list.o objects.o draw.o behaviors.o -o game.exe $(LIB1) -lmingw32 $(LIB2) -lSDLmain -lSDL -lSDL_mixer -mwindows -lpthread
	
game.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c src/game.c -o game.o
	
list.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c src/list.c -o list.o
	
objects.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c src/objects.c -o objects.o
	
draw.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c src/draw.c -o draw.o
	
behaviors.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c src/behaviors.c -o behaviors.o
	
clean:
	rm *.o