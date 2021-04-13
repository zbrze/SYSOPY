CC = gcc -Wall
PID = $(shell ps -ef |grep [.]/catcher | head -1| awk '{print $$2}')
all: 
	make clean
	make sender
	make catcher
	@echo "\n\nKILL TEST\n"
	./catcher sigrt &
	./sender 10 sigrt `pidof catcher` 
	

sender:
	$(CC) sender.c -o sender

catcher: 
	$(CC) catcher.c -o catcher
clean:
	rm -f *.o
	rm -f catcher
	rm -f sender
	