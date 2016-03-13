//implemented the BNPM. As of submission,
//essentially an extension of bnps.c
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
    int i, world_size;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &i);
    short *reporterFlag;

    if(argc!=2)
    {
        printf("Needed 2nd arg: number of reporters per editor\n");
        MPI_Finalize();
        return -1;
    }

    fflush(stdout);
    int items,j,time=TIME_STEP,off,flag, size, reporters, count, offset;
    char filename[100];
    FILE *fp;
    char cat[20];
    MPI_Status status;
    int index[CAT_NUM];
    news data[MAX_NEWS];

    reporters = atoi(argv[1]);//number of editors
    world_size -= reporters;
    size = world_size/reporters;//number of reporters per editor. facing issues if size is odd
    offset = size*(i/size);

    //get datatype for the struct
    type();

    //current reporters dataset
    news *dataset = (news *)malloc(sizeof(news)*CAT_NUM*MAX_COL);
    //incoming dataset from other reporters
    news *buf = (news *)malloc(sizeof(news)*CAT_NUM*MAX_COL);
    //flag if reporter processes are alive or not
    reporterFlag = (short *)malloc(sizeof(short)*size);

    bzero(filename, 100);
    bzero(reporterFlag, sizeof(short)*size);
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

    off = 0;flag=0;
    while(off<items)
    {
        for (j = 0; j < CAT_NUM; j++) 
            index[j]=0;
        bzero(dataset, sizeof(news)*CAT_NUM*MAX_COL);

        //read chunks of news from populated dataset
        for(;;)
        {
            if(off>=items)
                break;
            n = data[off++];
            if(n.time<=time)
            {
                dataset[n.cat*MAX_COL+index[n.cat]++]=n;
                flag=1;
            }
            else
            {
                off-=1;
                break;
            }
        }
        time+=TIME_STEP;
        printf("%d %d %d %d\n",items,i,off,offset);

        for(j = 1; j<=ceil(log(size)/log(2)); j++)
        {
            if((i-offset)%(int)pow(2,j) == 0 && (i+pow(2,j-1)-offset < size))
            {
                //printf("Reporter: %d Receiving from %d\n",i,i+(int)pow(2,j-1));
                if( reporterFlag[i+(int)pow(2,j-1)-offset] == 1)
                {
                    continue;
                }
                MPI_Recv(buf, CAT_NUM*MAX_COL, mpi_news_type, i+pow(2,j-1), 1, MPI_COMM_WORLD, &status);
                MPI_Get_count(&status, mpi_news_type, &count);
                if(count==0)
                {
                    reporterFlag[i+(int)pow(2,j-1)-offset] = 1;
                    continue;
                }
                dataset = mergeOperation(dataset, buf);
                //printd(dataset);
                printf("Reporter: %d Received from %d\n",i,i+(int)pow(2,j-1));
            }
            else if(((i-offset)-(int)pow(2,j-1)) % (int)pow(2,j)==0)
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

    //printf("Ending%d %d\n",i,size*(i/size));
    //ending reporter sends signal to editor
    if(i%size==0)
        MPI_Send(NULL, 0, mpi_news_type, world_size+(i/size), 1, MPI_COMM_WORLD);
    else
        //ending reporter signals reporters who might be waiting for it
        for(j=size*(i/size);j<i;j++)
        {
            MPI_Request request;
            MPI_Isend(NULL, 0, mpi_news_type, j, 1, MPI_COMM_WORLD, &request);
        }

    //printf("Ending %d\n",size*(i/size));
    MPI_Type_free(&mpi_news_type);
    MPI_Finalize();
    return 0;
}
