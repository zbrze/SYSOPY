CC = gcc -Wall 

main:
	$(CC) -c header.c -lrt -lpthread
	ar rcs header.a header.o
	$(CC) main.c header.o -o main.o -lrt -lpthread -lm