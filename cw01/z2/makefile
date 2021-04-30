CC = gcc -Wall 

test: static
		make static
		./output $(shell cat args.txt)

static:
	make clean
	$(CC) -c  ../z1/lib.c ../z1/lib.a -o lib.o
	ar rcs lib.a lib.o
	$(CC) main.c lib.o -o output 

shared: 
	make clean
	$(CC) -c -fPIC ../z1/lib.c -o lib.o -I.
	$(CC) -shared -fPIC -o lib.so lib.o
	$(CC) -o sharedOutput main.c ../z2/lib.so -I.

clean:
		rm -f *.o
		rm -f *.a
		rm -f *.so
