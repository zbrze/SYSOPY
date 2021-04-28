CC = gcc -Wall
O = O0
test_all: clean
			make test_O0
			make test_O1
			make test_Os

test_O0:
		printf "Biblioteka statyczna: O0\n" >> results3a.txt
		make static O=O0
		printf "\n\nBiblioteka współdzielona: O0\n" >> results3a.txt
		make shared O=O0
		printf "\n\nBiblioteka dynamiczna: O0\n" >> results3a.txt
		make dynamic O=O0

test_O1: 
		printf "\n\nBiblioteka statyczna: O1\n" >> results3a.txt
		make static O=O1
		printf "\n\nBiblioteka współdzielona: O1\n" >> results3a.txt
		make shared O=O1
		printf "Biblioteka dynamiczna: O1\n" >> results3a.txt
		make dynamic O=O1

test_Os:
		printf "\n\nBiblioteka statyczna: Os\n" >> results3a.txt
		make static O=Os
		printf "\n\nBiblioteka współdzielona: Os\n" >> results3a.txt
		make shared O=Os
		printf "\n\nBiblioteka dynamiczna: Os\n" >> results3a.txt
		make dynamic O=Os


static:
		$(CC) -c ../z1/lib.c
		ar crs lib.a lib.o
		$(CC) main.c -$O lib.o -o output
		./output $(shell cat args.txt)

shared:
		$(CC) -c -fPIC ../z1/lib.c -o lib.o 
		$(CC) -shared -fPIC -o lib.so lib.o
		$(CC) -o sharedOutput main.c -$O ../z3b/lib.so 
		./sharedOutput $(shell cat args.txt)
	
dynamic:
		$(CC) -c -fPIC ../z1/lib.c -o lib.o 
		$(CC) -shared -fPIC -o lib.so lib.o
		$(CC) -o sharedOutput main.c ../z3b/lib.so 
		$(CC)  main_dynamic.c -$O -ldl -o dynamicOutput
		./dynamicOutput $(shell cat args.txt)

clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f results3a.txt
