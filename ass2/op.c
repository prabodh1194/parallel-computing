#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include "bnp.h"
#include "helper_function.h"

news * mergeOperation(news *a, news *b)
{
    int i,j,k,di;
    news *c,*d, *ct, *dt;
    news *e = (news *)malloc(sizeof(news )*CAT_NUM*MAX_COL);
    for(i=0;i<CAT_NUM;i++)
    {
        c = (a+i*MAX_COL);
        d = (b+i*MAX_COL);
        di=0;

        for(j=0;j<MAX_COL;j++)
        {
            if(c[j].flag!=EMPTY)
            {
                c[j].flag==KEEP;
                for(k=0;k<MAX_COL;k++)
                {
                    if(d[k].flag!=EMPTY)
                    {
                        d[k].flag=KEEP;
                        if(strcmp(c[j].location,d[k].location)==0 && strcmp(c[j].data,d[k].data)==0)
                        {
                            c[j].flag=DISCARD;
                            break;
                        }
                    }
                }
                e[i*MAX_COL+di++] = d[j];
            }
        }
        for(j=0;j<MAX_COL;j++)
        {
            if(c[j].flag==KEEP)
                e[i*MAX_COL+di++]=c[j];
        }
    }
    return e;
}

int main(int argc, const char *argv[])
{
    int i,size;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &i);
    fflush(stdout);
    int j;
    char filename[100];
    FILE *fp;
    char cat[20], time[20];
    MPI_Status status;
    int index[CAT_NUM];

    type();

    news *dataset = (news *)malloc(sizeof(news)*CAT_NUM*MAX_COL);
    news *buf = (news *)malloc(sizeof(news)*CAT_NUM*MAX_COL);

    for (j = 0; j < CAT_NUM; j++) 
        index[j]=0;

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
        dataset[n.cat*MAX_COL+index[n.cat]++] = n;
    }

    fclose(fp);

    for(j = 1; j<=ceil(log(size)/log(2)); j++)
    {
        if(i%(int)pow(2,j) == 0 && (i+pow(2,j-1) < size))
        {
            printf("Recieving%d\n",i);
            MPI_Recv(buf, CAT_NUM*MAX_COL, mpi_news_type, i+pow(2,j-1), 1, MPI_COMM_WORLD, &status);
            printf("Recieved%d\n",i);
            dataset = mergeOperation(dataset, buf);
            printd(dataset);
        }
        else if((i-(int)pow(2,j-1)) % (int)pow(2,j)==0)
        {
            printf("sending %d\n",i);
            //printd(dataset);
            MPI_Send(dataset, CAT_NUM*MAX_COL, mpi_news_type, i-pow(2,j-1), 1, MPI_COMM_WORLD);
            printf("sent %d\n",i);
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
    MPI_Type_free(&mpi_news_type);
    MPI_Finalize();
    return 0;
}
