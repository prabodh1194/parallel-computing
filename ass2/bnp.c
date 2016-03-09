//program to show the implementation of basic derived data types of MPI
#include <stdio.h>
#include <mpi.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "bnp.h"

void type()
{

    //number of items inside structure news
    const int nitems = 5;

    int blocklengths[5] = {1, 1, NEWS_LENGTH, 20, 1};

    //data types present inside news in order
    MPI_Datatype types[5] = {MPI_INT, MPI_LONG, MPI_CHAR, MPI_CHAR, MPI_SHORT};

    //name of derived data type

    //array to store starting address of each item inside news
    MPI_Aint offsets[5];

    //offset of each item in news with respect to base address of Test
    offsets[0] = offsetof(news, cat);
    offsets[1] = offsetof(news, time);
    offsets[2] = offsetof(news, data);
    offsets[3] = offsetof(news, location);
    offsets[4] = offsetof(news, flag);

    //create the new derived data type
    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_news_type);

    //commit the new data type
    MPI_Type_commit(&mpi_news_type);



    //free the derived data type
}

/*
int main(int argc, const char *argv[])
{
    MPI_Init(NULL, NULL);
    const int tag = 0;
    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    type();

    //get rank of current process
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(rank == 0){
        news send;
        send.cat = 1;
        send.time = 2.0;
        send.flag = 2;
        bzero(send.data,20);
        bzero(send.location,20);
        strcpy(send.data,"hhellohellohelloello\0");
        strcpy(send.location,"DELHI\0");
        const int dest = 1;
        MPI_Send(&send, 1, mpi_news_type, dest, tag, MPI_COMM_WORLD);
        printf("\nRank %d sending %d, %ld, %s, %s\n", rank, send.cat, send.time, send.location, send.data);
    }
    if(rank == 1){
        MPI_Status status;
        const int src = 0;
        news recv;
        bzero(recv.data,10);
        bzero(recv.location,10);
        MPI_Recv(&recv, 1, mpi_news_type, src, tag, MPI_COMM_WORLD, &status);
        printf("\nRank %d recving %d, %ld, %s, %s, %d\n", rank, recv.cat, recv.time, recv.location, recv.data, recv.flag);
    }

    MPI_Type_free(&mpi_news_type);
    MPI_Finalize();
    return 0;
}
*/
