/*
 * K 
 * Given K directory paths, we'll compare contents of all the files in those
 * K directories.
 * Level order tree traversal used.
 * The threads take one directory and put their contents in a queue. A
 * comparison function checks for the common elements in a particular directory
 * of a level
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
void compareQ(char **, int, char ***);
int compareFiles(char *file1, char *file2);

pthread_mutex_t count_mut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cond_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_fill = PTHREAD_COND_INITIALIZER;
pthread_t *t;

int fill=0, fin=0, empty=0, mismatch=0, match=0;

char *path_dir, *path_file, **fsk;

struct queue *q;

int main(int argc, char **argv)
{
    int i,j,k,deb=0;
    struct arg *ar;

    if(strcmp(argv[argc-1],"-d")==0)
        deb = 1;

    k = argc-1-deb;

    if(k<2)
    {
        fprintf(stderr, "Looks like you are missing directory paths.\n Usage:: ./file <path 1> <path 2> ... <path k>\n");
        return -1;
    }

    fsk = (char **)malloc(k*sizeof(char *));
    for (i = 0; i < k; i++) 
    {
        fsk[i] = (char *)malloc(k*sizeof(char));
        for (j = 0; j < k; j++) 
            fsk[i][j]=1;            
    }

    q = (struct queue *)malloc(2*k*sizeof(struct queue));
    t = (pthread_t *)malloc(k*sizeof(pthread_t));

    for (i = 0; i < k; i++) 
    {
        ar = (struct arg *)malloc(sizeof(struct arg));
        int l = strlen(argv[i+1]);
        if(argv[i+1][l-1]=='/')
            argv[i+1][l-1]='\0';
    //contents of the directories at same level pushed to theses queues 
    //having even indices, and content it shares with other directory is pushed
    //in odd index of q
        createQueue(q+2*i);
        createQueue(q+2*i+1);
    //queuing root of fs
        enqueue(q+(2*i+1),"/",DT_DIR);
        ar->path = argv[i+1];
        ar->i = i;
        pthread_create(t+i, NULL, getFiles, (void*)ar);
    }

    while(1)
    {
        //wait till, a level has not been traversed absolutely
        pthread_mutex_lock(&cond_mutex);
        while(fill+fin<k)
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

        //compare the contents of queus with even indices and put common
        //directories in the ones with odd index
        compareQ(argv, k, &fsk);

        if(deb)
        {
            printf("After modds\n");
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

        //the queues have been consumed
        pthread_mutex_lock(&cond_mutex);
        pthread_cond_broadcast(&cond_empty);
        pthread_mutex_unlock(&cond_mutex);

        //if all queues which hold common content, i.e., ones with odd indices
        //are empty and hence no more further traversal is possible
        int a=1;
        for(i=0;i<k;i++)
            a&=isEmpty(q+2*i+1);

        if(a)
            break;
    }

    for (i = 0; i < k; i++) 
        pthread_join(t[i], NULL);

    //count matching filesystems
    int fsm = 0, one = 0;
    for(i=0;i<k;i++)
    {
        one = 0;
        printf("\n");
        for(j=0;j<k;j++)
        {
            if(deb)
                if(i!=j)
                    printf("%d\t",fsk[i][j]);
                else
                    printf("0\t");
            if(fsk[i][j]==1 && i!=j)
                one = 1;
        }
        if(one)
            fsm+=1;
    }

    printf("\n\nTotal Matches: %d\nTotal mismatches: %d\nFilesystems matching:%d/%d\n",match,mismatch,fsm,k);

    k/=2;
    if(k%2!=0)
        k+=1;

    if(mismatch==0)
        printf("Common Copy\n");
    else if(fsm>=k)
        printf("Majority Copy\n");

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
        tq->size=0;

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
            //count all the queues that have been filled for comparison later
            pthread_mutex_lock(&cond_mutex);
            fill+=1;
            pthread_cond_signal(&cond_fill);
            pthread_mutex_unlock(&cond_mutex);
        }
        //wait for the queues to empty
        pthread_mutex_lock(&cond_mutex);
        while(!isEmpty(tq))
            pthread_cond_wait(&cond_empty, &cond_mutex);
        pthread_mutex_unlock(&cond_mutex);
    }
    //certain traversal can complete before the others, hence indicate the ones
    //that have finished.
    pthread_mutex_lock(&cond_mutex);
    fin+=1;
    pthread_mutex_unlock(&cond_mutex);
    pthread_exit(0);
}

void compareQ(char **root, int k, char ***fsk)
{
    struct node *h1,*h2,*t2;
    int f=0,f2=0,i,j;
    char file1[FILE_PATH_SIZE],file2[FILE_PATH_SIZE];

    //select one queue and match it against all the others.
    //store the common directories in the queues so that further traversals can
    //happen to ensure common FSs

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
                            if(!isExists(&q[2*i+1], h1->x))
                                enqueue(&q[2*i+1],h1->x,h1->d_type);
                            if(!isExists(&q[2*j+1], h2->x))
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
                                (*fsk)[i][j]=2;
                                (*fsk)[j][i]=2;
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
                //no match happened
                if(f2==0)
                {
                    (*fsk)[i][j]=2;
                    (*fsk)[j][i]=2;
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
                if(q[2*i].size!=q[2*j].size)
                {
                    (*fsk)[i][j] = (*fsk)[j][i] = 2;
                }
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
                (*fsk)[i][i-1]=2;
                (*fsk)[i-1][i]=2;
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
