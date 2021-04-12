CC = gcc -Wall 

all: 
	make clean
	make sender
	make catcher

sender:
	$(CC) sender.c -o sender.o

catcher: 
	$(CC) catcher.c -o catcher.o

clean:
	rm -f *.o