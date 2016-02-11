#define FILE_PATH_SIZE 4096

struct node
{
    char x[FILE_PATH_SIZE];
    unsigned char d_type;
    struct node *next;
};

struct queue
{
    struct node *front, *rear;
};

void createQueue(struct queue *);
void enqueue(struct queue *, char *, unsigned char);
void dequeue(struct queue *, char *);
int isEmpty(struct queue *);
void print(struct queue);
