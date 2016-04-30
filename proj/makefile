cnf: clean
	mpicc -fopenmp -o cnf cnf.c -g -lm
	mpicc -fopenmp -o omp_cnf omp_cnf.c -g -lm
nqueens:
	mpicc -fopenmp -o nqs nqs.c -g
	mpicc -fopenmp -o omp_nqs omp_nqs.c -g
cnfm:
	mpicc -fopenmp -o cnfm cnfm.c -g -lm
clean:
	rm -rf .fuse* cnf omp_cnf cnfm nqs omp_nqs
