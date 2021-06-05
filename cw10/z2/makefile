CC = gcc -Wall 

main:
	#make clean
	$(CC) -c header.c -lrt -lpthread
	ar rcs header.a header.o
	$(CC) server.c header.o -o server.o -lrt -lpthread -lm
	$(CC) client.c header.o -o client.o -lrt -lpthread -lm
	#./server.o  6666 socket
	

clean:
	rm -f socket
	rm -f *.o
	rem -f leszek