#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>

#define THREAD 4

FILE *fp;
typedef int BOOL;
enum{FALSE, TRUE};

int hash[1000];

void printBoard(int *a, int n)
{
    int i, j, key, temp;
    char name[30];
    key= rand()%1000;

    while(hash[key]==1)
        key= (key+1)%1000;
    hash[key]=1;

    for(i=0; i<n; i++){
        temp= a[i];
        for(j=0; j<n; j++){
            if(j==temp)
                fprintf(fp, "1 ");
            else fprintf(fp, "0 ");
        }
        fprintf(fp, "\n");
    }
    fprintf(fp, "\n");
    fflush(fp);
}

BOOL stillLegal(int *board, int r)
{
    BOOL safe = TRUE;
    int i;
    // Check vertical
    for ( i = 0; i < r; ++i)
        if (board[i] == board[r]) safe = FALSE;
    // Check diagonals
    int ld = board[r];  //left diagonal columns
    int rd = board[r];  // right diagonal columns
    for ( i = r-1; i >= 0; --i) {
        --ld; ++rd;
        if (board[i] == ld || board[i] == rd) safe = FALSE;
    }

    return safe;
}

void NQueensD1(int *board, int row, int N)
{
    int i, j, k; 
    if (row == N){
        printBoard(board, N);
    }

    else {
        for (i = 0; i < N; ++i) {
            board[row] = i;
            if (stillLegal(board, row)) {
                // make copy of current board position
                //int *bnew = new int[N];
                int *bnew = NULL;
                bnew= (int *)malloc(sizeof(int)*N);

                for ( j = 0; j <= row; ++j) bnew[j] = board[j];

                NQueensD1(bnew, row+1, N);

                if(bnew!=NULL)
                    free(bnew);
                bnew=NULL;
            }
        }
    }
    return;
}

int main(int argc, char **args)
{
    omp_set_num_threads(THREAD);

    int N, i, size, rank;
    char file[12];

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    N = atoi(args[1]);

    sprintf(file,"f%d",rank);

    fp = fopen(file,"w");

    for(i=0; i<1000; i++)
        hash[i]=0;

#pragma omp parallel
    {
#pragma omp single
        {

            for(i = rank; i < N; i+=size)
            {
                int *board = NULL;
                board= (int *)malloc(sizeof(int)*N);
#pragma omp task
                {
                    board[0] = i;
                    NQueensD1(board, 1, N);
                }
#pragma omp taskwait
                if(board != NULL)
                    free(board);
                board = NULL;
            }

        }
    }

    //printf("Please check the text files generated. Each of the files generated contains a valid configuration for the board.\nIf no new file is generated, it means there is no possible valid configuration of the board\n");

    MPI_Finalize();
    fclose(fp);
    return 1;
}
