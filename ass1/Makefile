all: clean file.o q.o
	gcc q.o file.o -o file -l c -lpthread
	make cleano
file.o: 
	gcc file.c -Wall -c -g
q.o:
	gcc q.c -Wall -c -g
clean:
	rm -f file q *.o .fuse_hidden*
cleano:
	rm -f *.o
