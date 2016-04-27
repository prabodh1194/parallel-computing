#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <string.h>
#include <strings.h>
#include <math.h>

int main(void)
{
    fflush(stdout);
    int j,i, size,l,si;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &i);

    l = floor(log(size)/log(2));

    printf("Hello %d %d\n",i,l);

    for(j = 0; j<l; j++)
    {
        si = 1&i;

        if(si == 0)
            printf("~%d ",l-j);
        else
            printf("%d ",l-j);

        i = i>>1;
    }
    printf("\n");

    MPI_Finalize();
    return 0;
}
