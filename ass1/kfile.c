/*
 * Given two directory paths, we'll compare contents of all the files in those
 * two directories.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include "include/q.h"

#define FILE_NUMBER 100 //number of files in the directories.
#define FILE_NAME_SIZE 256 //number of permissible characters in file name in linux

void *getFiles(void *);
void compareQ(struct queue *, struct queue *, struct queue *, struct queue *,char *, char *);
int compareFiles(char *file1, char *file2);

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cond_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_fill = PTHREAD_COND_INITIALIZER;
pthread_t t1, t2;

int mismatch=0, match=0;

char *path_dir, *path_file;

struct queue q1,q2,q3,q4;

int main(int argc, char **argv)
{
    char *dir1, *dir2;
    int i,k;

    k = argc-1;

    dir1 = argv[1];
    dir2 = argv[2];

    if(k<2)
    {
        fprintf(stderr, "Looks like you are missing directory paths.\n Usage:: ./file <path 1> <path 2> ... <path k>\n");
        return -1;
    }

    for (i = 0; i < k; i++) 
    {
        int l = strlen(argv[i+1]);
        if(argv[i+1][l-1]=='/')
            argv[l-1]='\0';

    }

    createQueue(&q1);
    createQueue(&q2);
    createQueue(&q3);
    createQueue(&q4);

    enqueue(&q3,"/",DT_DIR);
    enqueue(&q4,"/",DT_DIR);

    pthread_create(&t1, NULL, getFiles, (void*)argv[1]);
    pthread_create(&t2, NULL, getFiles, (void*)argv[2]);

    while(!isEmpty(&q3) && !isEmpty(&q4))
    {
        pthread_mutex_lock(&cond_mutex);
        while(isEmpty(&q1) || isEmpty(&q2))
            pthread_cond_wait(&cond_fill, &cond_mutex);
        pthread_mutex_unlock(&cond_mutex);


        /*
           printf("q1\n");
           print(q1);
           printf("\nq2\n");
           print(q2);
           printf("\nq3\n");
           print(q3);
           */

        compareQ(&q1, &q2, &q3, &q4, dir1, dir2);

        /*
           printf("After modds\n");
           printf("q1\n");
           print(q1);
           printf("\nq2\n");
           print(q2);
           printf("\nq3\n");
           print(q3);
           printf("\nq4\n");
           print(q4);
           */

        pthread_mutex_lock(&cond_mutex);
        pthread_cond_broadcast(&cond_empty);
        pthread_mutex_unlock(&cond_mutex);
    }

    //printf("Files %d\nDir1:%s\nDir2:%s\n",fileno,dir1,dir2);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("\n\nTotal Matches: %d\nTotal mismatches: %d\n",match,mismatch);

    return 0;
}

void *getFiles(void *ptr)
{
    char *path;
    char root[FILE_PATH_SIZE];
    char filename[FILE_PATH_SIZE];
    char temp[FILE_PATH_SIZE];
    DIR *dip;
    struct dirent *dit;
    struct queue *tq,*sq;

    path = (char *)ptr;

    if(pthread_equal(t1,pthread_self())!=0)
    {
        //printf("\n1\n");
        tq = &q1;
        sq = &q3;
    }
    else
    {
        //printf("\n2\n");
        tq = &q2;
        sq = &q4;
    }

    while(!isEmpty(sq))
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
            pthread_mutex_lock(&cond_mutex);
            pthread_cond_signal(&cond_fill);
            pthread_mutex_unlock(&cond_mutex);
        }
        pthread_mutex_lock(&cond_mutex);
        while(!isEmpty(tq))
            pthread_cond_wait(&cond_empty, &cond_mutex);
        pthread_mutex_unlock(&cond_mutex);
    }
    pthread_exit(0);
}

void compareQ(struct queue *q1, struct queue *q2, struct queue *q3, struct queue *q4, char *root1, char *root2)
{
    struct node *h1,*h2,*t1,*t2;
    int f=0,f2=0;

    char file1[FILE_PATH_SIZE],file2[FILE_PATH_SIZE];

    t1 = NULL;
    h1 = q1->front;

    while(h1!=NULL)
    {
        t2 = NULL;
        h2 = q2->front;
        while(h2!=NULL)
        {
            if(strcmp(h1->x,h2->x)==0 && h1->d_type==h2->d_type)
            {
                f2=1;
                if(h1->d_type==DT_DIR)
                {
                    enqueue(q3,h1->x,h1->d_type);
                    enqueue(q4,h2->x,h2->d_type);
                }
                else if(h1->d_type != DT_UNKNOWN)
                {
                    sprintf(file1,"%s%s",root1,h1->x);
                    sprintf(file2,"%s%s",root2,h2->x);
                    if(compareFiles(file1,file2)!=1)
                    {
                        printf("Files %s & %s are different in content\n",file1,file2);
                        mismatch+=1;
                    }
                    else
                        match+=1;
                }
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
                    dequeue(q2,NULL);
                }
            }
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
                printf("Directory is new fs 1: %s\n",h1->x);
                mismatch+=1;
            }
            else
            {
                printf("File is new fs 1: %s\n",h1->x);
                mismatch+=1;
            }
        }
        else
            f2=0;
        t1 = h1;
        h1 = h1->next;
        dequeue(q1,NULL);
    }
    h2 = q2->front;
    while(h2!=NULL)
    {
        if(h2->x[strlen(h2->x)-1]=='/')
        {
            printf("Directory is new fs 2: %s\n",h2->x);
            mismatch+=1;
        }
        else
        {
            printf("File is new fs 2: %s\n",h2->x);
            mismatch+=1;
        }
        h2=h2->next;
        dequeue(q2,NULL);
    }
    q2 = NULL;
}

int compareFiles(char *file1, char *file2)
{
    struct stat buf1, buf2;

    //compare file size
    stat(file1, &buf1);
    stat(file2, &buf2);

    if(buf1.st_size != buf2.st_size)
        return -1;
    return 1;
}