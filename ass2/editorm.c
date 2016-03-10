#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include "bnp.h"
#include "helper_function.h"
#include <unistd.h>

int main(int argc, char **argv)
{

    int reporters, rank, size, world_size,i,j;

    MPI_Init(NULL, NULL);

    if(argc!=2)
    {
        printf("Needed 2nd arg: number of reporters per editor\n");
        MPI_Finalize();
        return -1;
    } 

    reporters = atoi(argv[1]);
    news *buf = (news *)malloc(sizeof(news)*CAT_NUM*MAX_COL);
    news *dataset = (news *)malloc(sizeof(news)*CAT_NUM*MAX_COL);
    MPI_Status status;
    MPI_Group group;
    type();

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    world_size-=reporters;
    size = world_size/reporters;

    printf("At editor %d %d\n",(rank-world_size)*size, rank);

    printf("Receiving at editor %d\n",rank);
    MPI_Recv(dataset, CAT_NUM*MAX_COL, mpi_news_type, (rank-world_size)*size, 1, MPI_COMM_WORLD, &status);
    //printd(dataset);
    printf("Received at editor %d\n",rank);

    rank-=world_size;
    i = rank;

    MPI_Barrier(MPI_COMM_WORLD);

    for(j = 1; j<=ceil(log(reporters)/log(2)); j++)
    {
        if(i%(int)pow(2,j) == 0 && (i+pow(2,j-1) < reporters))
        {
            printf("Receiving%d\n",world_size+i);
            MPI_Recv(buf, CAT_NUM*MAX_COL, mpi_news_type, world_size+i+pow(2,j-1), 1, MPI_COMM_WORLD, &status);
            printf("Received%d\n",world_size+i);
            dataset = mergeOperation(dataset, buf);
            //printd(dataset);
        }
        else if((i-(int)pow(2,j-1)) % (int)pow(2,j)==0)
        {
            printf("sending %d\n",world_size+i);
            //printd(dataset);
            MPI_Send(dataset, CAT_NUM*MAX_COL, mpi_news_type, world_size+i-pow(2,j-1), 1, MPI_COMM_WORLD);
            printf("sent %d\n",world_size+i);
        }
        //MPI_Barrier(MPI_COMM_WORLD);
    }

    if(rank == 0)
        printd(dataset);
    MPI_Type_free(&mpi_news_type);
    MPI_Finalize();
    return 0;
}