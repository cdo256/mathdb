.RECIPEPREFIX +=
a.out: $(wildcard *.c *.h makefile)
    gcc -g -Wextra -Wall *.c
