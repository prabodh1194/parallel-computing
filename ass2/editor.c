#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include "bnp.h"
#include "helper_function.h"

int main(void)
{

    int rank, size;
    MPI_Init(NULL, NULL);
    news *buf = (news *)malloc(sizeof(news)*CAT_NUM*MAX_COL);
    MPI_Status status;
    type();

    MPI_Recv(buf, CAT_NUM*MAX_COL, mpi_news_type, 0, 1, MPI_COMM_WORLD, &status);
    printf("Received at editor\n");
    printd(buf);

    MPI_Type_free(&mpi_news_type);
    MPI_Finalize();
    return 0;
}
