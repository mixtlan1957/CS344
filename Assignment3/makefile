all: smallsh


smallsh.o: smallsh.c
	gcc -g -Wall -std=c99 -c smallsh.c

smallish: smallish.o
	gcc -g -Wall -std=c99 -o smallsh smallsh.o


clean:
	rm -f smallsh
	
