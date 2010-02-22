SRC=$(wildcard *.cpp)
OBJ=$(patsubst %.cpp,%.o,$(SRC))
MAIN=srv
CPPFLAGS=-Wno-deprecated
LDFLAGS=-lboost_system -lboost_filesystem

$(MAIN): $(OBJ)
	g++ -o $@ $^ $(LDFLAGS)

-include depend.mk

.PHONY: clean depend
depend:
	gcc -M $(SRC) >depend.mk

clean:
	-rm $(MAIN) $(OBJ)
