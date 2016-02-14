/*
 * Given two directory paths, we'll compare contents of all the files in those
 * two directories.
 */

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include "include/q.h"

#define FILE_NUMBER 100 //number of files in the directories.
#define FILE_NAME_SIZE 256 //number of permissible characters in file name in linux

void getFiles(char *);
void compareQ(char **, int);
int compareFiles(char *file1, char *file2);

int fill=0, fin=0, empty=0, mismatch=0, match=0, deb, k;

char *path_dir, *path_file, **args;

struct queue *q;

int main(int argc, char **argv)
{
    int i;

    args = argv;

    if(strcmp(argv[argc-1],"-d")==0)
        deb = 1;

    k = argc-1-deb;

    if(k<2)
    {
        fprintf(stderr, "Looks like you are missing directory paths.\n Usage:: ./file <path 1> <path 2> ... <path k>\n");
        return -1;
    }

    q = (struct queue *)malloc(2*k*sizeof(struct queue));

    for (i = 0; i < k; i++) 
    {
        int l = strlen(argv[i+1]);
        if(argv[i+1][l-1]=='/')
            argv[i+1][l-1]='\0';
        createQueue(q+2*i);
        createQueue(q+2*i+1);
        enqueue(q+(2*i+1),"/",DT_DIR);
    }

    omp_set_num_threads(k);

#pragma omp parallel
    {
        int tid = omp_get_thread_num();
        getFiles(argv[tid+1]);
    }

    printf("\n\nTotal Matches: %d\nTotal mismatches: %d\n",match,mismatch);

    return 0;
}

void getFiles(char *path)
{
    int i = omp_get_thread_num();
    char root[FILE_PATH_SIZE], filename[FILE_PATH_SIZE], temp[FILE_PATH_SIZE];
    DIR *dip;
    struct dirent *dit;
    struct queue *tq,*sq;

    tq = q+2*i;
    sq = q+2*i+1;

    while(1)
    {
        if(!isEmpty(sq)) 
        {
            dequeue(sq,root);
            sprintf(filename,"%s%s",path,root);

            if(NULL == (dip = opendir(filename)))
            {
                fprintf(stderr,"Bad path");
            }
            else
            {
                while((dit = readdir(dip)) != NULL)
                {
                    if(strcmp(dit->d_name,"..")!=0 && strcmp(dit->d_name,".")!=0)
                    {
                        sprintf(temp,"%s%s",root,dit->d_name);
                        if(dit->d_type == DT_DIR)
                            strcat(temp,"/");
                        enqueue(tq,temp,dit->d_type);
                    }
                }
            }
        }
#pragma omp barrier
#pragma omp single
        {
            if(deb)
            {
                printf("After barrier\n");
                printf("fin %d\n",fin);
                for (i = 0; i < k; i++) 
                {
                    printf("q%d",2*i); printf("\n"); print(q[2*i]);
                }
                for (i = 0; i < k; i++) 
                {
                    printf("q%d",2*i+1); printf("\n"); print(q[2*i+1]);
                }
            }

            compareQ(args, k);

            if(deb)
            {
                printf("After modds\n");
                for (i = 0; i < k; i++) 
                {
                    printf("q%d",2*i); printf("\n"); print(q[2*i]);
                }
                for (i = 0; i < k; i++) 
                {
                    printf("q%d",2*i+1); printf("\n"); print(q[2*i+1]);
                }
            }
        }
        int l,a = 1;
        for (l = 0; l < k; l++)
            a&=isEmpty(&q[2*l+1]);
        if(a)
            break;
    }
}

void compareQ(char **root, int k)
{
    struct node *h1,*h2,*t2;
    int f=0,f2=0,i,j;
    char file1[FILE_PATH_SIZE],file2[FILE_PATH_SIZE];

    for (i = 0; i < k-1; i++) 
    {
        h1 = q[2*i].front;
        while(h1!=NULL)
        {
            for(j = i+1; j<k ; j++)
            {
                t2 = NULL;
                h2 = q[2*j].front;
                f2=0;
                f=0;
                while(h2!=NULL)
                {
                    if(strcmp(h1->x,h2->x)==0 && h1->d_type==h2->d_type)
                    {
                        f2=1; //match happened
                        if(h1->d_type==DT_DIR)
                        {
                            if(!isExists(&q[2*i+1],h1->x))
                            enqueue(&q[2*i+1],h1->x,h1->d_type);
                            if(!isExists(&q[2*j+1],h2->x))
                            enqueue(&q[2*j+1],h2->x,h2->d_type);
                        }
                        else if(h1->d_type != DT_UNKNOWN)
                        {
                            sprintf(file1,"%s%s",root[i+1],h1->x);
                            sprintf(file2,"%s%s",root[j+1],h2->x);
                            if(compareFiles(file1,file2)!=1)
                            {
                                printf("Files %s & %s (in fs%d and fs%d) respectively are different in content\n",file1,file2,i+1,j+1);
                                mismatch+=1;
                            }
                            else
                                match+=1;
                        }
                        if(i+1 == k-1)
                        {
                            if(t2!=NULL)
                            {
                                t2->next = h2->next;
                                free(h2);
                                h2 = t2;
                            }
                            else
                            {
                                f = 1;
                                h2 = h2->next;
                                dequeue(&q[2*j],NULL);
                            }
                        }
                        break;
                    }
                    //construct to help advance on LL
                    if(f==0)
                    {
                        t2 = h2;
                        h2 = h2->next;
                    }
                    else
                        f=0;
                }
                if(f2==0)
                {
                    if(h1->x[strlen(h1->x)-1]=='/')
                    {
                        printf("Directory is absent in fs%d and present in fs%d: %s\n",j+1,i+1,h1->x);
                        mismatch+=1;
                    }
                    else
                    {
                        printf("File is absent in fs%d and present in fs%d: %s\n",j+1,i+1,h1->x);
                        mismatch+=1;
                    }
                }
                else
                    f2=0;
            }
            h1 = h1->next;
            dequeue(&q[2*i],NULL);
        }
    }

    for(i=0;i<k;i++)
    {
        if(!isEmpty(q+2*i))
        {
            h2 = q[2*i].front;
            while(h2!=NULL)
            {
                if(h2->x[strlen(h2->x)-1]=='/')
                {
                    printf("Directory is absent in fs%d and present in fs%d: %s\n",i,i+1,h2->x);
                    mismatch+=1;
                }
                else
                {
                    printf("File is absent in fs%d and present in fs%d: %s\n",i,i+1,h2->x);
                    mismatch+=1;
                }
                h2=h2->next;
                dequeue(&q[2*i],NULL);
            }
        }
    }
}