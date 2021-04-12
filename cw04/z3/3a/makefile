CC = gcc -Wall
PID = $(shell ps -ef |grep [.]/catcher | head -1| awk '{print $$2}')
all: 
	make clean
	make sender
	make catcher
	@echo "\n\n-----------------------KILL TEST-----------------------\n"
	./catcher kill &
	./sender 100 kill `pidof catcher` 
	@echo "\n\n\n-----------------------SIGIRT TEST-----------------------\n"
	./catcher sigrt &
	./sender 100 sigrt `pidof catcher` 
	@echo "\n\n\n-----------------------SIGQUEUE TEST-----------------------\n"
	./catcher sigqueue &
	./sender 100 sigqueue `pidof catcher` 

sender:
	$(CC) sender.c -o sender

catcher: 
	$(CC) catcher.c -o catcher
clean:
	rm -f *.o
	rm -f catcher
	rm -f sender
	