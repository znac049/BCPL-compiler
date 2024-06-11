SRC=cg.cc \
	machine.cc

OBJ=$(subst .cc,.o,$(SRC))
HDRS=$(wildcard *.h)

EXE=cg09

CC=g++ -m32
LD=g++ -m32

CFLAGS=-O2 -Wno-format -fpermissive
LDFLAGS=

.cc.o: $(HDRS)
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJ)
	$(LD) -o $(EXE) $(OBJ) $(LDFLAGS)

clean:
	rm -f *.o *~ \#* $(EXE)