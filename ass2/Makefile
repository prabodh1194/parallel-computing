bnps: clean
	mpicc helper_function.c bnp.c bnps.c -o bnps -g -lm
	mpicc helper_function.c bnp.c editor.c -o editor -g -lm
bnpm: clean
	mpicc helper_function.c bnp.c bnpm.c -o bnpm -g -lm
	mpicc helper_function.c bnp.c editorm.c -o editorm -g -lm
test: clean
	gcc helper_function.c test.c -o test -g
clean:
	rm -rf .fuse_hidden* bnps test editor editorm bnpm
