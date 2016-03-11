#ifndef _BNP_H
#define _BNP_H

#include <mpi.h>
#define CAT_NUM 5
#define MAX_COL 10
#define MAX_RECV 100
#define EMPTY 0
#define KEEP 1
#define DISCARD -1
#define AVAILABLE 2
#define NEWS_LENGTH 2000
#define MAX_NEWS 100
#define TIME_STEP 5

MPI_Datatype mpi_news_type;

enum category {
    SPORTS,
    POLITICS,
    WORLD,
    SCIENCE,
    BUSINESS
};

typedef struct news{
    enum category cat;
    long time;
    char data[NEWS_LENGTH];
    char location[20];
    short flag; //0=empty, 1=keep, -1=discard
} news;

void type();
#endif
