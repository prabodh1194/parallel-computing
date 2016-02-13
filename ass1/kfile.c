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

struct arg {
    char *path;
    int i;
};

void *getFiles(void *);
void compareQ(struct queue *, struct queue *, struct queue *, struct queue *,char *, char *);
int compareFiles(char *file1, char *file2);

pthread_mutex_t count_mut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cond_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_fill = PTHREAD_COND_INITIALIZER;
pthread_t *t;

int fill=0, empty=0, mismatch=0, match=0;

char *path_dir, *path_file;

struct queue *q;

int main(int argc, char **argv)
{
    int i,k,deb=0;
    struct arg *ar;

    if(strcmp(argv[argc-1],"-d")==0)
        deb = 1;

    k = argc-1-deb;

    if(k<2)
    {
        fprintf(stderr, "Looks like you are missing directory paths.\n Usage:: ./file <path 1> <path 2> ... <path k>\n");
        return -1;
    }

    q = (struct queue *)malloc(2*k*sizeof(struct queue));
    t = (pthread_t *)malloc(k*sizeof(pthread_t));

    for (i = 0; i < k; i++) 
    {
        ar = (struct arg *)malloc(sizeof(struct arg));
        int l = strlen(argv[i+1]);
        if(argv[i+1][l-1]=='/')
            argv[i+1][l-1]='\0';
        createQueue(q+2*i);
        createQueue(q+2*i+1);
        enqueue(q+2*i+1,"/",DT_DIR);
        ar->path = argv[i+1];
        ar->i = i;
        pthread_create(t+i, NULL, getFiles, (void*)ar);
    }

    while(!isEmpty(&q[1]) && !isEmpty(&q[3]))
    {
        pthread_mutex_lock(&cond_mutex);
        while(fill<k)
            pthread_cond_wait(&cond_fill, &cond_mutex);
        pthread_mutex_unlock(&cond_mutex);

        fill = 0;

        if(deb)
        {
            for (i = 0; i < k; i++) 
            {
                printf("q%d",2*i);
                printf("\n");
                print(q[2*i]);
            }
            for (i = 0; i < k; i++) 
            {
                printf("q%d",2*i+1);
                printf("\n");
                print(q[2*i+1]);
            }
        }

        compareQ(q,q+2,q+1,q+3,argv[1],argv[2]);

        printf("After modds\n");
        if(deb)
        {
            for (i = 0; i < k; i++) 
            {
                printf("q%d",2*i);
                printf("\n");
                print(q[2*i]);
            }
            for (i = 0; i < k; i++) 
            {
                printf("q%d",2*i+1);
                printf("\n");
                print(q[2*i+1]);
            }
        }

        pthread_mutex_lock(&cond_mutex);
        pthread_cond_broadcast(&cond_empty);
        pthread_mutex_unlock(&cond_mutex);
    }

    printf("Files \nDir1:%s\nDir2:%s\n",argv[1],argv[2]);

    for (i = 0; i < k; i++) 
        pthread_join(t[i], NULL);

    printf("\n\nTotal Matches: %d\nTotal mismatches: %d\n",match,mismatch);

    return 0;
}

void *getFiles(void *ptr)
{
    int i;
    char *path, root[FILE_PATH_SIZE], filename[FILE_PATH_SIZE], temp[FILE_PATH_SIZE];
    DIR *dip;
    struct dirent *dit;
    struct queue *tq,*sq;
    struct arg *ar;

    ar = (struct arg*)ptr;

    path = ar->path;
    i = ar->i;

    tq = q+2*i;
    sq = q+2*i+1;

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
            fill+=1;
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
