#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include "bnp.h"
#include "helper_function.h"

int main(int argc, const char *argv[])
{
    int i, size, world_size, reporters;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &i);

    if(argc!=2)
    {
        printf("Needed 2nd arg: number of reporters per editor\n");
        MPI_Finalize();
        return -1;
    }

    reporters = atoi(argv[1]);
    world_size -= reporters;
    size = world_size/reporters;

    fflush(stdout);
    int j;
    char filename[100];
    FILE *fp;
    char cat[20], time[20];
    MPI_Status status;
    int **index;
    index = (int **)malloc(sizeof(int)*CAT_NUM*size);

    type();

    news *dataset = (news *)malloc(sizeof(news)*CAT_NUM*MAX_COL);
    news *buf = (news *)malloc(sizeof(news)*CAT_NUM*MAX_COL);

    for (j = 0; j < CAT_NUM; j++) 
        index[i/size][j]=0;

    bzero(filename, 100);
    sprintf(filename,"news%d",i);

    //read data
    char c;
    news n;
    fp = fopen(filename, "r");
    for(;;)
    {
        c=getc(fp);

        if(c==EOF) 
            break;

        fp->_IO_read_ptr-=1;

        bzero(n.location,20);
        bzero(n.data, NEWS_LENGTH);
        fscanf(fp, "%s", cat);
        fscanf(fp, "%ld", &(n.time));
        fscanf(fp, "%s", n.location);

        n.cat = getCategory(cat);

        for(j=0; j<NEWS_LENGTH; j++)
        {
            c=getc(fp);
            if(c=='\n')
                break;
            n.data[j]=c;
        }
        n.data[j]='\0';
        n.flag = AVAILABLE;
        dataset[n.cat*MAX_COL+index[i/size][n.cat]++] = n;
    }

    fclose(fp);

    for(j = 1; j<=ceil(log(size)/log(2)); j++)
    {
        if(i%(int)pow(2,j) == 0 && (i+pow(2,j-1) < size))
        {
            printf("Receiving%d\n",i);
            MPI_Recv(buf, CAT_NUM*MAX_COL, mpi_news_type, i+pow(2,j-1), 1, MPI_COMM_WORLD, &status);
            printf("Received%d\n",i);
            dataset = mergeOperation(dataset, buf);
            //printd(dataset);
        }
        else if((i-(int)pow(2,j-1)) % (int)pow(2,j)==0)
        {
            printf("sending %d\n",i);
            //printd(dataset);
            MPI_Send(dataset, CAT_NUM*MAX_COL, mpi_news_type, i-pow(2,j-1), 1, MPI_COMM_WORLD);
            printf("sent %d\n",i);
        }
        //MPI_Barrier(MPI_COMM_WORLD);
    }

    if(i%size==0)
    {
        printf("Sending to editor%d\n",i);
        MPI_Send(dataset, CAT_NUM*MAX_COL, mpi_news_type, world_size+(i/size), 1, MPI_COMM_WORLD);
        printf("Sent to editor%d\n",i);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Type_free(&mpi_news_type);
    MPI_Finalize();
    return 0;
}
