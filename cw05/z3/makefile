CC = gcc
FLAGS = -std=c99 -Wall

main: main.c
	$(CC) $(FLAGS) main.c -o main

producer: producer.c
	$(CC) $(FLAGS) producer.c -o producer

consumer: consumer.c
	$(CC) $(FLAGS) consumer.c -o consumer

compile: main.c producer.c consumer.c
	make clean
	make main
	make producer
	make consumer
	touch consumer.txt

clean: 
	rm -f main
	rm -f producer
	rm -f consumer
	rm -f pipe
	rm -rf files
	rm -f consumer.txt