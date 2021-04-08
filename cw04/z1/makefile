CC = gcc -Wall 

all: 
	make clean
	make lib 
	make main
	make test_fork
	make test_exec

lib:
	$(CC) -c lib.c 
	ar rcs lib.a lib.o

main:
	$(CC) main.c lib.o -o main.o
	
test_fork:
	./main.o ignore 10 fork
	./main.o mask 10 fork
	./main.o handler 10 fork
	./main.o pending 10 fork

test_exec:
	$(CC) exec.c lib.o -o exec.o
	make main
	./main.o ignore 10 exec
	#./main.o handler 10 exec - testowanie wywala sie
	./main.o mask 10 exec
	./main.o pending 10 exec

clean:
	rm -f *.a
	rm -f *.o