.phony all:
all: pman inf

inf: inf.c
	gcc inf.c -o inf
pman: pman.c
	gcc pman.c -o pman

.PHONY clean:
clean:
	-rm -rf *.o *.exe
