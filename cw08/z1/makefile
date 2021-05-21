
CC = gcc -Wall 

main:
	make clean
	$(CC) -c header.c -lrt -lpthread
	ar rcs header.a header.o
	$(CC) main.c header.o -o main.o -lrt -lpthread -lm
	make numbers
	make blocks
	
numbers:
	echo "============================================================N U M B E R S============================================================\n\n" >> "times.txt"
	echo "_______________________________ IMG SIZE: 256  256 _______________________________\n" >> "times.txt"
	./main.o 1 numbers images/pepper.ascii.pgm out.pgm
	./main.o 2 numbers images/pepper.ascii.pgm out.pgm
	./main.o 4 numbers images/pepper.ascii.pgm out.pgm
	./main.o 8 numbers images/pepper.ascii.pgm out.pgm
	./main.o 16 numbers images/pepper.ascii.pgm out.pgm
	echo "\n\n" >> "times.txt"
	echo "_______________________________ IMG SIZE: 640 480 _______________________________\n" >> "times.txt"
	./main.o 1 numbers images/f14.ascii.pgm out.pgm
	./main.o 2 numbers images/f14.ascii.pgm out.pgm
	./main.o 4 numbers images/f14.ascii.pgm out.pgm
	./main.o 8 numbers images/f14.ascii.pgm out.pgm
	./main.o 16 numbers images/f14.ascii.pgm out.pgm
	echo "\n\n" >> "times.txt"
	echo "_______________________________ IMG SIZE: 800 800 _______________________________\n" >> "times.txt"
	./main.o 1 numbers images/dla.ascii.pgm out.pgm
	./main.o 2 numbers images/dla.ascii.pgm out.pgm
	./main.o 4 numbers images/dla.ascii.pgm out.pgm
	./main.o 8 numbers images/dla.ascii.pgm out.pgm
	./main.o 16 numbers images/dla.ascii.pgm out.pgm

blocks:
	echo "\n\n==========================================================B L O C K S==========================================================\n" >> "times.txt"
	echo "_______________________________ IMG SIZE: 256  256 _______________________________\n" >> "times.txt"
	./main.o 1 numbers images/pepper.ascii.pgm out.pgm
	./main.o 2 numbers images/pepper.ascii.pgm out.pgm
	./main.o 4 numbers images/pepper.ascii.pgm out.pgm
	./main.o 8 numbers images/pepper.ascii.pgm out.pgm
	./main.o 16 numbers images/pepper.ascii.pgm out.pgm
	echo "\n\n" >> "times.txt"
	echo "_______________________________ IMG SIZE: 640 480 _______________________________\n" >> "times.txt"
	./main.o 1 numbers images/f14.ascii.pgm out.pgm
	./main.o 2 numbers images/f14.ascii.pgm out.pgm
	./main.o 4 numbers images/f14.ascii.pgm out.pgm
	./main.o 8 numbers images/f14.ascii.pgm out.pgm
	./main.o 16 numbers images/f14.ascii.pgm out.pgm
	echo "\n\n" >> "times.txt"
	echo "_______________________________ IMG SIZE: 800 800 _______________________________\n" >> "times.txt"
	./main.o 1 numbers images/dla.ascii.pgm out.pgm
	./main.o 2 numbers images/dla.ascii.pgm out.pgm
	./main.o 4 numbers images/dla.ascii.pgm out.pgm
	./main.o 8 numbers images/dla.ascii.pgm out.pgm
	./main.o 16 numbers images/dla.ascii.pgm out.pgm


clean:
	rm -f "times.txt"
	rm -f *.o