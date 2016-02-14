/*Queue - Linked List implementation*/
#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "include/q.h"
#include <unistd.h>
#include "include/sha1.h"

void createQueue(struct queue *q)
{
    q->front = NULL;
    q->rear = NULL;
}

void enqueue(struct queue *q, char *d, unsigned char d_t)
{
    struct node *node = (struct node*)malloc(sizeof(struct node));
    strcpy(node->x,d);
    node->d_type = d_t;
    node->next = NULL;

    if(q->front==NULL)
    {
        q->front = node;
        q->rear = node;
    }
    else
    {
        q->rear->next = node;
        q->rear = node;
    }
}

void dequeue(struct queue *q, char *a)
{
    struct node *temp;
    if(a!=NULL)
        strcpy(a , q->front->x);
    temp = q->front;
    q->front = q->front->next;
    if(q->front == NULL)
        q->rear = NULL;
    free(temp);
}

int isEmpty(struct queue *q)
{
    if(q->front==NULL)
        return 1;
    else
        return 0;
}

void print(struct queue q)
{
    struct node *t;
    t = q.front;

    printf("\n");

    while(t!=NULL)
    {
        printf("%s\t",t->x);
        t=t->next;
    }
}

int compareFiles(char *file1, char *file2)
{
    struct stat buf1, buf2;
    char *read_buf;
    unsigned char op[20],op1[20];

    //compare file size
    stat(file1, &buf1);
    stat(file2, &buf2);

    if(buf1.st_size != buf2.st_size)
        return -1;

    read_buf = (char *)malloc(sizeof(char)*buf1.st_size);

    bzero(op,20);
    int fd1 = open(file1, O_RDONLY);
    read(fd1, read_buf, buf1.st_size);
    mbedtls_sha1(read_buf, buf1.st_size, op);

    bzero(op1,20);
    int fd2 = open(file2, O_RDONLY);
    read(fd2, read_buf, buf2.st_size);
    mbedtls_sha1(read_buf, buf2.st_size, op1);

    lseek(fd1, 0, SEEK_SET);
    lseek(fd2, 0, SEEK_SET);

    if(strncmp(op,op1,20)!=0)
    {
        close(fd1);close(fd2);
        return -1;
    }

    off_t n=0;

    int readsize,SIZE = 4096;

    char read_buf1[SIZE+1];
    char read_buf2[SIZE+1];

    while(1)
    {
        readsize=read(fd1, read_buf1, SIZE);
        n+=readsize;
        read(fd2, read_buf2, SIZE);
        if(strncmp(read_buf1,read_buf2,readsize)!=0)
        {
            close(fd1);
            close(fd2);
            return -1;
        }
        if(n>=buf1.st_size)
            break;
        bzero(read_buf1, SIZE+1);
        bzero(read_buf2, SIZE+1);
    }

    return 1;
}

/*
   int main(void)
   {
   char a[23];
   struct queue q;
   createQueue(&q);
   enqueue(&q, "1");
   print(q);
   enqueue(&q,"2");
   print(q);
   enqueue(&q,"3");
   print(q);
   dequeue(&q,a);
   print(q);
   return 0;
   }
   */
