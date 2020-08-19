# 
# Makefile for basic video player
#

CC=gcc
CFLAGS=-lavformat -lavcodec -lavutil -lm `sdl2-config --cflags --libs`
DEPS= player.h display.h
OBJ= player.o display.o 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

videoplayer: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)