CC = gcc -Wall 

test: static
		make static
		./output 10 txt_files/big1.txt:txt_files/big2.txt txt_files/med1.txt:txt_files/med2.txt txt_files/lil1.txt:txt_files/lil2.txt txt_files/big1.txt:txt_files/med1.txt txt_files/lil1.txt:txt_files/med2.txt
		./output 3 txt_files/big1.txt:txt_files/lil1.txt txt_files/med1.txt:txt_files/big2.txt
		./output 6 txt_files/big2.txt:txt_files/lil1.txt txt_files/med1.txt:txt_files/big3.txt
		./output 13 txt_files/big4.txt:txt_files/lil1.txt txt_files/med2.txt:txt_files/big4.txt  txt_files/med2.txt:txt_files/big1.txt txt_files/med1.txt:txt_files/big4.txt txt_files/lil1.txt:txt_files/big4.txt txt_files/med1.txt:txt_files/lil1.txt 
		make clean-all
static:
	make clean-b4
	$(CC) -c  lib.c -o lib.o
	ar rcs lib.a lib.o
	$(CC) main.c lib.o -o output 


clean-b4:
	rm -f raport.txt
clean-all:
		rm -f *.o
		rm -f *.a
		rm -f *.so
		rm -f output
		rm -f tmp.txt