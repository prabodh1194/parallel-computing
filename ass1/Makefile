all: clean file.o q.o
	gcc q.o file.o -o file -l c -lpthread -g
	make cleano
file.o: 
	gcc file.c -Wall -c
q.o:
	gcc q.c -Wall -c
clean:
	rm -f file q *.o
cleano:
	rm -f *.o
