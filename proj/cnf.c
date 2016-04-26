#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>

void print(int **cnf, int l, int c)
{
    int i,j;

    for (i = 0; i < c; i++) 
    {
        for (j = 0; j < l+2; j++) 
            printf("%d ",cnf[i][j]);
        printf("\n");
    }
}

void simplify(int **cnf, int *ls, int c, int len, int x)
{
    //print(cnf, len, c);

    int i, l, flag = 1;
    fflush(stdout);

    l = x>>1;

    if(l>=len)
        return;

    for (i = 0; i < c; i++)
    {
        if(cnf[i][0] == 1)
            continue;
        if(cnf[i][l+2] == x)
            cnf[i][0] = 1;
        else if(cnf[i][l+2] == (~(x&1)&~(~x&~1)))
        {
            cnf[i][1]-=1;
            cnf[i][l+2] = -1;
            if(cnf[i][1] == 0)
                return; // 0-clause found
        }
        flag = flag & cnf[i][0];
    }
    ls[l] = x;
    if(flag)
    {
        for (i = 0; i < len; i++) 
            if(ls[i]!=-1)
            {
                if(ls[i]%2 == 1)
                    printf("~");
                printf("%d ",1+(ls[i]>>1));
            }
        printf("\n");
        return;
    }
    l+=1;
    int *temp;
    int **ccnf = (int **)malloc(sizeof(int *)*c);
    int *cx = (int *)malloc(sizeof(int)*len);
    for (i = 0; i < c; i++)
    {
        ccnf[i] = (int *)malloc(sizeof(int)*(len+2));
        memcpy(*(ccnf+i), *(cnf+i), sizeof(int)*(len+2));
    }
    memcpy(cx, ls, sizeof(int)*len);

    simplify(ccnf, cx, c, len, l<<1);
    simplify(cnf, ls, c, len, l<<1|1);

    for(i = 0; i < c; i++)
        free(ccnf[i]);
    free(cx);
    free(ccnf);
}

void getInput(int **cnf, int l, int c)
{
    int i,j,li,li1;
    char cl[3];

    for (i = 0; i < c; i++) 
    {
        for (j = 0; j < l; j++) 
        {
            scanf("%s", cl);
            if(cl[0] == '!' || cl[0] == '~')
            {
                li = atoi(cl+1)-1;
                li1 = (li<<1)|1;
            }
            else
            {
                li = atoi(cl)-1;
                li1 = li<<1;
            }
            cnf[i][li+2] = li1;
            if(getc(stdin) == '\n')
                break;
            else
                stdin->_IO_read_ptr--;
        }
        cnf[i][0] = 0;
        cnf[i][1] = j+1;
    }
}

int main(int argc, const char *argv[])
{
    if(argc!=3)
    {
        printf("Usage: ./program <literals> <clauses>");
        return -1;
    }

    int **cnf, clauses, literals, i, *x, res, j;

    clauses = atoi(argv[2]);
    literals = atoi(argv[1]);
    cnf = (int **)malloc(sizeof(int *)*clauses);
    x = (int *)malloc(sizeof(int)*literals);

    bzero(x, sizeof(int)*literals);

    for(i = 0; i<clauses; i++)
    {
        cnf[i] = (int *)malloc(sizeof(int)*(literals+2));
        for(j = 0; j<literals; j++)
            cnf[i][j+2] = -1;
    }

    for(i = 0; i< literals; i++)
        x[i] = -1;

    getInput(cnf, literals, clauses);
    int **ccnf = (int **)malloc(sizeof(int *)*clauses);
    int *cx = (int *)malloc(sizeof(int)*literals);
    for (i = 0; i < clauses; i++)
    {
        ccnf[i] = (int *)malloc(sizeof(int)*(literals+2));
        memcpy(ccnf[i], cnf[i], sizeof(int)*(literals+2));
    }
    memcpy(cx, x, sizeof(int)*literals);

    simplify(ccnf, cx, clauses, literals, 0);
    simplify(cnf, x, clauses, literals, 1);

    for(i = 0; i < clauses; i++)
    {
        free(ccnf[i]);
        free(cnf[i]);
        //free(cnf[i]);
    }
    free(ccnf);
    free(cx);
    free(x);
    free(cnf);

    return 0;
}
