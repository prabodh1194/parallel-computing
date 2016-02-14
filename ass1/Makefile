all: clean file kfile opfile
	make cleano

file: clean file.o q.o
	gcc q.o file.o -o file -g -l c -lpthread
kfile: clean kfile.o q.o
	gcc q.o kfile.o -o kfile -g -l c -lpthread
opfile: clean opfile.o q.o
	gcc q.o opfile.o -o opfile -l c -g -fopenmp

file.o: 
	gcc file.c -Wall -c -g
kfile.o: 
	gcc kfile.c -Wall -c -g
opfile.o:
	gcc opfile.c -Wall -c -fopenmp -g
q.o:
	gcc q.c -Wall -c -g

clean:
	rm -f file kfile opfile q *.o .fuse_hidden*
cleano:
	rm -f *.o
