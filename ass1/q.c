/*Queue - Linked List implementation*/
#include<stdio.h>
#include<stdlib.h>
#include <string.h>

#define FILE_PATH_SIZE 4096

struct node
{
    char x[FILE_PATH_SIZE];
    struct node *next;
};

struct queue
{
    struct node *front, *rear;
};

void createQueue(struct queue *q)
{
    q->front = NULL;
    q->rear = NULL;
}

void enqueue(struct queue *q, char *d)
{
    struct node *node = (struct node*)malloc(sizeof(struct node));
    strcpy(node->x,d);
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
    strcpy(a , q->front->x);
    temp = q->front;
    q->front = q->front->next;
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
