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

void createQueue(struct queue *q);
void enqueue(struct queue *q, char *d);
void dequeue(struct queue *q, char *a);
int isEmpty(struct queue *q);
void print(struct queue q);
