CC = gcc -Wall 

header:
	$(CC) -c header.c -lrt -lpthread
	ar rcs header.a header.o
	$(CC) main.c header.o -o main -lrt -lpthread
	$(CC) pizzerman.c header.o -o pizzerman -lrt -lpthread
	$(CC) deliveryman.c header.o -o deliveryman -lrt -lpthread