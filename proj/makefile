all:
	mpicc -fopenmp -o cnf cnf.c -g -lm
	mpicc -fopenmp -o omp_cnf omp_cnf.c -g -lm
cnfm:
	mpicc -fopenmp -o cnfm cnfm.c -g -lm
clean:
	rm -rf .fuse* cnf omp_cnf cnfm
