CC = gcc -Wall 

header:
	$(CC) -c common_header.c -lrt
	ar rcs common_header.a common_header.o
	$(CC) server.c common_header.o -o server -lrt
	$(CC) client.c common_header.o -o client -lrt
	#./server
	