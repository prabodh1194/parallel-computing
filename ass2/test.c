#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include "bnp.h"
#include "helper_function.h"

int main(int argc, const char *argv[])
{
    int size, myrank, i, j;
    char filename[100];
    FILE *fp;
    char cat[20], time[20];
    int index[CAT_NUM];

    news *dataset1 = (news *)malloc(sizeof(news)*CAT_NUM*MAX_COL);
    news *dataset = (news *)malloc(sizeof(news)*CAT_NUM*MAX_COL);

    for (i = 0; i < CAT_NUM; i++) 
        index[i]=0;

    //read data
    char c;
    news n;
    fp = fopen("news0", "r");
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

    for (i = 0; i < CAT_NUM; i++) 
        index[i]=0;
    fp = fopen("news1", "r");
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
        dataset1[n.cat*MAX_COL+index[n.cat]++] = n;
    }

    dataset = mergeOperation(dataset, dataset1);

    for(i=0;i<CAT_NUM;i++)
        for(j=0;j<MAX_COL;j++)
        {
            news n = dataset[i*MAX_COL+j];
            if(n.flag==EMPTY)
                break;
            printf("Category:%s\n Time:%ld\n Location:%s\n News:%s\n\n",getCategoryName(n.cat),n.time,n.location,n.data);
        }

    return 0;
}
