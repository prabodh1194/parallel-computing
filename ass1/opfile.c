/*
 * K = 2
 * Given two directory paths, we'll compare contents of all the files in those
 * two directories.
 * Level order tree traversal used.
 * The threads take one directory and put their contents in a queue. A
 * comparison function checks for the common elements.
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
void compareQ(struct queue *, struct queue *, struct queue *, struct queue *,char *, char *);
int compareFiles(char *file1, char *file2);

int mismatch=0, match=0, deb;

char *path_dir, *path_file, *dir1, *dir2;

struct queue q1,q2,q3,q4;

int main(int argc, char * const *argv)
{
    deb = 0;

    dir1 = argv[1];
    dir2 = argv[2];

    if(strcmp(argv[argc-1],"-d")==0)
        deb = 1;

    if(argc<3)
    {
        fprintf(stderr, "Looks like you are missing directory paths.\n Usage:: ./file <path 1> <path 2>\n");
        return -1;
    }

    if(dir1[strlen(dir1)-1]=='/')
        dir1[strlen(dir1)-1]='\0';
    if(dir2[strlen(dir2)-1]=='/')
        dir2[strlen(dir2)-1]='\0';

    //contents of two directories at same level pushed to theses queues by
    //filesystem 1 and 2 respectively.
    createQueue(&q1);
    createQueue(&q2);
    //common directories pushed to these queues.
    //The threads look at theses queues to determine next directory whose
    //contents are to be analysed.
    createQueue(&q3);
    createQueue(&q4);

    //queuing root of fs
    enqueue(&q3,"/",DT_DIR);
    enqueue(&q4,"/",DT_DIR);

    omp_set_num_threads(2);

#pragma omp parallel
    {
        int tid = omp_get_thread_num();
        //printf("%d %d\n",omp_get_thread_num(),omp_get_num_threads());
        getFiles(argv[tid+1]);
    }

    //printf("Files %d\nDir1:%s\nDir2:%s\n",fileno,dir1,dir2);

    printf("\n\nTotal Matches: %d\nTotal mismatches: %d\n",match,mismatch);

    if(mismatch==0)
        printf("Common copy\n");

    return 0;
}

void getFiles(char *path)
{
    char root[FILE_PATH_SIZE], filename[FILE_PATH_SIZE], temp[FILE_PATH_SIZE];
    DIR *dip;
    struct dirent *dit;
    struct queue *tq,*sq;

    if(omp_get_thread_num()==0)
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
        }
        //wait till, a level has not been traversed absolutely
        //q1 and q2 are filled
#pragma omp barrier
#pragma omp single
        {
            if(deb)
            {
                printf("q1\n"); print(q1); printf("\nq2\n"); print(q2); printf("\nq3\n"); print(q3);
            }

            //q1 and q2 consists of contents of a level.
            //q3 and q4 contain the common contents of a level.
            compareQ(&q1, &q2, &q3, &q4, dir1, dir2);

            if(deb)
            {
                printf("After modds\n"); printf("q1\n"); print(q1); printf("\nq2\n"); print(q2); printf("\nq3\n"); print(q3); printf("\nq4\n"); print(q4);
            }
        }
    }
}

//compare contents of q1 and q2 and push common contents back to q3 and a4 for
//level-by-level traversal
void compareQ(struct queue *q1, struct queue *q2, struct queue *q3, struct queue *q4, char *root1, char *root2)
{
    struct node *h1,*h2,*t2;
    int f=0,f2=0;

    char file1[FILE_PATH_SIZE],file2[FILE_PATH_SIZE];

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
                    //put common directories for further traversal
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
                break;
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
