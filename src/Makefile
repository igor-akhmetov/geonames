IMPORT=import
QUERY=query
SRC=$(wildcard *.c)
OBJ=$(patsubst %.c,%.o,$(SRC))
IMPORT_OBJ=$(filter-out $(QUERY).o, $(OBJ))
QUERY_OBJ=$(filter-out $(IMPORT).o, $(OBJ))
CFLAGS=-Wall

all: $(IMPORT) $(QUERY)

$(IMPORT): $(IMPORT_OBJ)
	gcc -o $@ $^

$(QUERY): $(QUERY_OBJ)
	gcc -o $@ $^

-include depend.mk

.PHONY: clean depend
depend:
	gcc -M $(SRC) >depend.mk

clean:
	-rm $(IMPORT) $(QUERY) $(OBJ) 2>/dev/null

