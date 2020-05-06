CFLAGS = -Wall -Werror -Wextra -pedantic \
	-Wshadow -Winvalid-pch -Wcast-align -Wformat=2 \
	-Wformat-nonliteral -Wmissing-declarations \
	-Wmissing-format-attribute \
	-Wmissing-include-dirs -Wredundant-decls \
	-Wswitch-default -g

SOURCES = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJS = $(SOURCES:.c=.o)

.PHONY : all
all : test

test : $(OBJS)
	$(CC) $(CFLAGS) -o test $(OBJS) $(LFLAGS) $(LIBS)

.PHONY : clean
clean :
	rm $(OBJS) test

fuzztest.o : fuzztest.c $(HEADERS)
mdb_generic_map.o : mdb_generic_map.c $(HEADERS)
mdb_mem.o : mdb_mem.c $(HEADERS)
mdb_multiset.o : mdb_multiset.c $(HEADERS)
mdb_node_map.o : mdb_node_map.c $(HEADERS)
mdb_search_graph.o : mdb_search_graph.c $(HEADERS)
mdb_simple_graph.o : mdb_simple_graph.c $(HEADERS)
mdb_util.o : mdb_util.c $(HEADERS)
mdb_vector.o : mdb_vector.c $(HEADERS)
unittest.o : unittest.c $(HEADERS)
