op: clean
	mpicc helper_function.c bnp.c op.c -o op -g -lm
test: clean
	gcc helper_function.c test.c -o test -g
clean:
	rm -rf .fuse_hidden* op test
