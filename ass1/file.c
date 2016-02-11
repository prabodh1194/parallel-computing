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
#include "q.h"

#define FILE_NUMBER 100 //number of files in the directories.
#define FILE_NAME_SIZE 256 //number of permissible characters in file name in linux

void *getFiles(void *);
int compareFiles(char *file1, char *file2);

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cond_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_fill = PTHREAD_COND_INITIALIZER;
pthread_t t1, t2;

int mismatch=0, match=0;

char *path_dir, *path_file;

struct queue q1,q2,q3;

int main(int argc, char * const *argv)
{
    char *dir1, *dir2;
    int fileno = FILE_NUMBER, c;


    dir1 = argv[1];
    dir2 = argv[2];

    if(argc<3 || dir1[0]=='-' || dir2[0]=='-')
    {
        fprintf(stderr, "Looks like you are missing directory paths.\n Usage:: ./file <path 1> <path 2> [-f]\n");
        return -1;
    }

    while((c = getopt(argc, argv, "f:"))!=-1)
        switch(c)
        {
            case 'f':
                fileno = atoi(optarg);
                break;
            case '?':
                if(optopt == 'f')
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                else if(isprint(optopt))
                    fprintf(stderr, "Unknown option '-%c'.]n", optopt);
                else
                    fprintf(stderr, "Unknown option character '\\x%x'.\n",optopt);
                return 1;
            default:
                abort();
        }

    createQueue(&q1);
    createQueue(&q2);
    createQueue(&q3);

    enqueue(&q3,"/");

    pthread_create(&t1, NULL, getFiles, (void*)dir1);
    pthread_create(&t2, NULL, getFiles, (void*)dir2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("%d %d\n",match,mismatch);

    //printf("Files %d\nDir1:%s\nDir2:%s\n",fileno,dir1,dir2);
    return 0;
}

/* Used http://www.thegeekstuff.com/2012/06/c-directory/ to write this function*/
void *getFiles(void *ptr)
{
    char path[FILE_PATH_SIZE];
    while(!isEmpty(&q3))
    {
        dequeue(&q3,path);
        if(pthread_equal(t1,pthread_self())==0)
        {
            pthread_mutex_lock(&cond_mutex);
            while(!isEmpty(&q1))
                pthread_cond_wait(&cond_empty, &cond_mutex);
            pthread_mutex_unlock(&cond_mutex);
        }
    }
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
