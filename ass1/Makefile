CC=vtcc

all: clean file kfile opfile opkfile
	make cleano

file: clean file.o q.o sha1.o
	$(CC) sha1.o q.o file.o -o file -g -l c -lpthread
kfile: clean kfile.o q.o sha1.o
	$(CC) sha1.o q.o kfile.o -o kfile -g -l c -lpthread
opfile: clean opfile.o q.o sha1.o
	$(CC) sha1.o q.o opfile.o -o opfile -l c -g -fopenmp
opkfile: clean opkfile.o q.o sha1.o
	$(CC) sha1.o q.o opkfile.o -o opkfile -l c -g -fopenmp

file.o: 
	$(CC) file.c -Wall -c -g
kfile.o: 
	$(CC) kfile.c -Wall -c -g
opfile.o:
	$(CC) opfile.c -Wall -c -fopenmp -g
opkfile.o:
	$(CC) opkfile.c -Wall -c -fopenmp -g
q.o:
	$(CC) q.c -Wall -c -g
sha1.o:
	$(CC) sha1.c -Wall -c

clean:
	rm -f file kfile opfile opkfile q *.o .fuse_hidden*
cleano:
	rm -f *.o
