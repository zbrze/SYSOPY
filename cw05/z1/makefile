CC = gcc -Wall  -std=gnu99

test:
	make clean 
	make interpreter
	./interpreter.o commands.txt

interpreter: 
	$(CC) interpreter.c -o interpreter.o

clean:
	rm -f *.o
