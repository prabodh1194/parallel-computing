#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bnp.h"

int getCategory(char *a)
{
    if(strcmp(a, "SPORTS\0")==0) return SPORTS;
    else if(strcmp(a, "POLITICS\0")==0) return POLITICS;
    else if(strcmp(a, "WORLD\0")==0) return WORLD;
    else if(strcmp(a, "SCIENCE\0")==0) return SCIENCE;
    else if(strcmp(a, "BUSINESS\0")==0) return BUSINESS;
    else return -1;
}

char * getCategoryName(int a)
{
    if(a == SPORTS) return "SPORTS\0";
    else if(a == POLITICS) return "POLITICS\0";
    else if(a == WORLD) return "WORLD\0";
    else if(a == SCIENCE) return "SCIENCE\0";
    else if(a == BUSINESS) return "BUSINESS\0";
    else return '\0';
}

//for a given category, news from one buffer is picked and then matched with
//every news item os same category in second buffer over news content and
//location of oeigin. the new merged data is returned.
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
                c[j].flag=KEEP;
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
            }
        }
        for(j=0;j<MAX_COL;j++)
        {
            if(c[j].flag==KEEP)
            {
                c[j].flag=AVAILABLE;
                e[i*MAX_COL+di++]=c[j];
            }
            if(d[j].flag==AVAILABLE || d[j].flag==KEEP)
            {
                d[j].flag=AVAILABLE;
                e[i*MAX_COL+di++]=d[j];
            }
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
