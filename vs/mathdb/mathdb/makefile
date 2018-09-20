ifeq ($(OS),Windows_NT)
	CC = cl
	EXT = .exe
	PLATOPT += -Fetest1.exe -ZI
else
	CC = gcc
	EXT = 
	PLATOPT += -Wextra -g -otest
endif
test1$(ext): $(wildcard ../../../*.c ../../../*.h makefile)
	$(CC) -Wall $(PLATOPT) ../../../*.c
