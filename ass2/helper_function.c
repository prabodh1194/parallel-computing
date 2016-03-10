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

void printd(news *n1)
{
    int i,j;
    for(i=0;i<CAT_NUM;i++)
        for(j=0;j<MAX_COL;j++)
        {
            news n = n1[i*MAX_COL+j];
            if(n.flag==EMPTY || n.data[0]=='\0')
                break;
            printf("Category:%s\n Time:%ld\n Location:%s\n News:%s\n Flag:%d\n\n",getCategoryName(n.cat),n.time,n.location,n.data,n.flag);
        }
}
