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
    int i,size,count; MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &i);
    news data[MAX_NEWS];
    short *reporterFlag;
    size-=1;

    fflush(stdout);
    int items,j,time=TIME_STEP,off,flag;
    char filename[100];
    FILE *fp;
    char cat[20];
    MPI_Status status;
    int index[CAT_NUM];

    type();

    news *dataset = (news *)malloc(sizeof(news)*CAT_NUM*MAX_COL);
    news *buf = (news *)malloc(sizeof(news)*CAT_NUM*MAX_COL);
    reporterFlag = (short *)malloc(sizeof(short)*size);

    bzero(filename, 100);
    bzero(reporterFlag, sizeof(short)*size);
    sprintf(filename,"news%d",i);

    //read data
    char c;
    news n;
    fp = fopen(filename, "r");
    j = 0;
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
        //get next time set
        for (j = 0; j < CAT_NUM; j++) 
            index[j]=0;
        bzero(dataset, sizeof(news)*CAT_NUM*MAX_COL);
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
        //printf("%d %d %d\n",items,i,off);

        if(flag==0)
            continue;

        for(j = 1; j<=ceil(log(size)/log(2)); j++)
        {
            if(i%(int)pow(2,j) == 0 && (i+pow(2,j-1) < size))
            {
                //printf("%d Receiving from %d\n",i,i+(int)pow(2,j-1));
                if( reporterFlag[i+(int)pow(2,j-1)] == 1)
                    continue;
                MPI_Recv(buf, CAT_NUM*MAX_COL, mpi_news_type, i+pow(2,j-1), 1, MPI_COMM_WORLD, &status);
                MPI_Get_count(&status, mpi_news_type, &count);
                if(count==0)
                {
                    reporterFlag[i+(int)pow(2,j-1)] = 1;
                    continue;
                }
                dataset = mergeOperation(dataset, buf);
                //printd(dataset);
                printf("%d Received from %d\n",i,i+(int)pow(2,j-1));
            }
            else if((i-(int)pow(2,j-1)) % (int)pow(2,j)==0)
            {
                //printf("%d Sending to %d\n",i,i-(int)pow(2,j-1));
                //printd(dataset);
                MPI_Send(dataset, CAT_NUM*MAX_COL, mpi_news_type, i-pow(2,j-1), 1, MPI_COMM_WORLD);
                printf("%d Sent to %d\n",i,i-(int)pow(2,j-1));
            }
        }

        //printf("%d %d",i, size);
        if(i==0)
        {
            //printf("Sending to editor\n");
            MPI_Send(dataset, CAT_NUM*MAX_COL, mpi_news_type, size, 1, MPI_COMM_WORLD);
            printf("Sent to editor\n");
        }
        //MPI_Barrier(MPI_COMM_WORLD);
    }

    if(i==0)
        MPI_Send(NULL, 0, mpi_news_type, size, 1, MPI_COMM_WORLD);
    else
        for(j=0;j<i;j++)
            MPI_Send(NULL, 0, mpi_news_type, j, 1, MPI_COMM_WORLD);

    //MPI_Barrier(MPI_COMM_WORLD);
    MPI_Type_free(&mpi_news_type);
    MPI_Finalize();
    return 0;
}
