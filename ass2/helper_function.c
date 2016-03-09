#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bnp.h"

int getCategory(char *a)
{
    if(strcmp(a, "sports")==0)
        return sports;
    else if(strcmp(a, "politics")==0)
        return politics;
    else
        return -1;
}

char * getCategoryName(int a)
{
    if(a == sports)
        return "sports\0";
    else if(a == politics)
        return "politics\0";
    else
        return '\0';
}

void printd(news *n1)
{
    int i,j;
    for(i=0;i<CAT_NUM;i++)
        for(j=0;j<MAX_COL;j++)
        {
            news n = n1[i*MAX_COL+j];
            if(n.flag==EMPTY)
                break;
            printf("Category:%s\n Time:%ld\n Location:%s\n News:%s\n Flag:%d\n\n",getCategoryName(n.cat),n.time,n.location,n.data,n.flag);
        }
}
