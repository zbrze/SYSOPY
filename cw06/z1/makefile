CC = gcc -Wall 

header:
	$(CC) -c common_header.c 
	ar rcs common_header.a common_header.o
	$(CC) server.c common_header.o -o server
	$(CC) client.c common_header.o -o client
	#./server
	