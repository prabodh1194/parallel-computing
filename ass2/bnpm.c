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
    int items,j,time=TIME_STEP,off;
    char filename[100];
    FILE *fp;
    char cat[20];
    MPI_Status status;
    int index[CAT_NUM];
    news data[MAX_NEWS];

    type();

    news *dataset = (news *)malloc(sizeof(news)*CAT_NUM*MAX_COL);
    news *buf = (news *)malloc(sizeof(news)*CAT_NUM*MAX_COL);

    bzero(filename, 100);
    sprintf(filename,"news%d",i);

    //read data
    char c;
    news n;
    fp = fopen(filename, "r");
    items = 0;
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
        data[items++] = n;
    }

    fclose(fp);

    //done loading dataset

    off = 0;
    while(off<items)
    {
        for (j = 0; j < CAT_NUM; j++) 
            index[j]=0;

        for(;;)
        {
            if(off>=items)
                break;
            n = data[off++];
            if(n.time<=time)
                dataset[n.cat*MAX_COL+index[n.cat]++]=n;
        }
        time+=TIME_STEP;

        for(j = 1; j<=ceil(log(size)/log(2)); j++)
        {
            if(i%(int)pow(2,j) == 0 && (i+pow(2,j-1) < size))
            {
                //printf("Reporter: %d Receiving from %d\n",i,i+(int)pow(2,j-1));
                MPI_Recv(buf, CAT_NUM*MAX_COL, mpi_news_type, i+pow(2,j-1), 1, MPI_COMM_WORLD, &status);
                printf("Reporter: %d Received from %d\n",i,i+(int)pow(2,j-1));
                dataset = mergeOperation(dataset, buf);
                //printd(dataset);
            }
            else if((i-(int)pow(2,j-1)) % (int)pow(2,j)==0)
            {
                //printf("Reporter: %d Sending to %d\n",i,i-(int)pow(2,j-1));
                //printd(dataset);
                MPI_Send(dataset, CAT_NUM*MAX_COL, mpi_news_type, i-pow(2,j-1), 1, MPI_COMM_WORLD);
                printf("Reporter: %d Sent to %d\n",i,i-(int)pow(2,j-1));
            }
            //MPI_Barrier(MPI_COMM_WORLD);
        }

        if(i%size==0)
        {
            //printf("Reporter %d Sending to editor%d\n",i, world_size+(i/size));
            MPI_Send(dataset, CAT_NUM*MAX_COL, mpi_news_type, world_size+(i/size), 1, MPI_COMM_WORLD);
            printf("Reporter %d Sent to editor%d\n",i, world_size+(i/size));
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Type_free(&mpi_news_type);
    MPI_Finalize();
    return 0;
}
