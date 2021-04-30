CC = gcc -Wall 

all: static shared clean

static:
	$(CC) -c lib.c 
	ar rcs lib.a lib.o
	$(CC) main.c lib.o -o output

shared:
	$(CC) -c -fPIC lib.c
	$(CC) -shared -fPIC -o lib.so lib.o 

clean:
	rm -f *.o
