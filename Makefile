SRCS=easyAI.c gomoku.c normalAI.c randAI.c reijerAI.c
CFLAGS=-Wall -Wextra -Wpedantic 
CC=clang

gomoku: $(SRCS)
	$(CC) $(CFLAGS) -o gomoku $(SRCS)

clean:
	-rm gomoku
