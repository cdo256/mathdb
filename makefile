ifeq ($(OS),Windows_NT)
	CC = cl
	EXT = .exe
	PLATOPT += -Wall -Fetest.exe
else
	CC = gcc
	EXT =
	PLATOPT += -Wall -Wextra -g -otest
endif
test$(ext): $(wildcard *.c *.h makefile)
	$(CC) -Wall $(PLATOPT) *.c
