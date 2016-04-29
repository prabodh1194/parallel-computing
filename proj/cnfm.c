#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <string.h>
#include <strings.h>
#include <math.h>

int main(void)
{
    fflush(stdout);
    int j,i, size,l;
    MPI_Status si;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &i);

    int *a = (int *)malloc(sizeof(int)*10);

    printf("Hello %d\n",i);

    if(i == 0)
    {
        for (i = 0; i < 10; i++) 
            a[i] = i;
        MPI_Send(a, 10, MPI_INT, 1, 0, MPI_COMM_WORLD);
    }
    else
    {
        MPI_Recv(a, 5, MPI_INT, 0, 0, MPI_COMM_WORLD, &si);
        for (i = 0; i < 5; i++) 
            printf("%d ",a[i]);
    }
    
    printf("\n");

    MPI_Finalize();
    return 0;
}
