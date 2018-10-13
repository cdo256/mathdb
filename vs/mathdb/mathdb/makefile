ifeq ($(OS),Windows_NT)
	CC = cl
	EXT = .exe
	PLATOPT += -Fetest.exe -ZI
else
	CC = gcc
	EXT = 
	PLATOPT += -Wextra -g -otest
endif
test$(ext): $(wildcard ../../../*.c ../../../*.h makefile)
	$(CC) -Wall $(PLATOPT) ../../../*.c
