CC = gcc -Wall

test: clean
	printf "Biblioteka statyczna: \n" >> results3a.txt
	make static

	printf "\n\nBiblioteka wspoldzielona: \n" >> results3a.txt
	make shared

	printf "\n\nBiblioteka dynamiczna:\n" >> results3a.txt
	make dynamic

static:
		$(CC) -c ../z1/lib.c
		ar crs lib.a lib.o
		$(CC) main.c lib.o -o output
		./output $(shell cat args.txt)

shared:
		$(CC) -c -fPIC ../z1/lib.c -o lib.o 
		$(CC) -shared -fPIC -o lib.so lib.o
		$(CC) -o sharedOutput main.c ../z3a/lib.so 
		./sharedOutput $(shell cat args.txt)
	
dynamic:
		$(CC) -c -fPIC ../z1/lib.c -o lib.o 
		$(CC) -shared -fPIC -o lib.so lib.o
		$(CC) -o sharedOutput main.c ../z3a/lib.so 
		$(CC)  main_dynamic.c -ldl -o dynamicOutput
		./dynamicOutput $(shell cat args.txt)

clean:
	rm -f main
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f results3a.txt
