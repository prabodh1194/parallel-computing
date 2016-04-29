#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <mpi.h>
#include <omp.h>
#include <math.h>

short sat = 0;
FILE *fp1;
int LEVEL = 0;
int MAX_LEVEL = 0;

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

void simplify(int **cnf, int *ls, int c, int len, int x, int flag2)
{
    //printf("%d\n",omp_get_thread_num());
    //print(cnf, len, c);
    int i, l, flag = 1;
    fflush(fp1);

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
        sat = 1;
        char *out = (char *)malloc(sizeof(char)*len*3);
        bzero(out,sizeof(char)*len*3);
        for (i = 0; i < len; i++) 
            if(ls[i]!=-1)
            {
                if(ls[i]%2 == 1)
                    sprintf(out,"%s!",out);
                sprintf(out,"%s%d ",out,1+(ls[i]>>1));
            }
        fprintf(fp1,"%s\n",out);
        free(out);
        return;
    }

    if(flag2)
        return;
    l+=1;

        int **ccnf = (int **)malloc(sizeof(int *)*c);
        int *cx = (int *)malloc(sizeof(int)*len);
        for (i = 0; i < c; i++)
        {
            ccnf[i] = (int *)malloc(sizeof(int)*(len+2));
            memcpy(*(ccnf+i), *(cnf+i), sizeof(int)*(len+2));
        }
        memcpy(cx, ls, sizeof(int)*len);
        if(LEVEL<MAX_LEVEL)
        {
#pragma omp atomic
            LEVEL+=1;
#pragma omp task
            simplify(ccnf, cx, c, len, l<<1, 0);
#pragma omp task
            simplify(cnf, ls, c, len, l<<1|1, 0);
        }
        else
        {
            simplify(ccnf, cx, c, len, l<<1, 0);
            simplify(cnf, ls, c, len, l<<1|1, 0);
        }
#pragma omp taskwait
        for(i = 0; i < c; i++)
            free(ccnf[i]);
        free(cx);
        free(ccnf); 

}

void getInput(int **cnf, int l, int c, const char *file)
{
    int i,j,li,li1;
    char cl[3];

    FILE *fp = fopen(file, "r");

    for (i = 0; i < c; i++) 
    {
        for (j = 0; j < l; j++) 
        {
            fscanf(fp,"%s", cl);
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
            if(getc(fp) == '\n')
                break;
            else
                fp->_IO_read_ptr--;
        }
        cnf[i][0] = 0;
        cnf[i][1] = j+1;
    }
    fclose(fp);
}

int main(int argc, const char *argv[])
{
    fflush(stdout);
    if(argc!=4)
    {
        printf("Usage: ./program <literals> <clauses> <i/p file name");
        return -1;
    }

    int clauses, literals, i, res, j, size, l, si, rank;
    char file[20];

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    l = floor(log(size)/log(2));

    sprintf(file, "f%d", rank);
    fp1 = fopen(file,"w");

    //number of clauses
    clauses = atoi(argv[2]);
    //number of variables
    literals = atoi(argv[1]);

    double t1 = omp_get_wtime();

#pragma omp parallel
    {
#pragma omp single
        {
            int **cnf = (int **)malloc(sizeof(int *)*clauses);
            int *x = (int *)malloc(sizeof(int)*literals);

            for(i = 0; i<clauses; i++)
            {
                cnf[i] = (int *)malloc(sizeof(int)*(literals+2));
                for(j = 0; j<literals; j++)
                    cnf[i][j+2] = -1;
            }

            for(i = 0; i< literals; i++)
                x[i] = -1;

            getInput(cnf, literals, clauses, argv[3]);

            for(j = 0; j<l; j++)
            {
                si = 1&rank;

                if(si == 0)
                    simplify(cnf, x, clauses, literals, (l-j-1)<<1|1, 1);
                else
                    simplify(cnf, x, clauses, literals, (l-j-1)<<1, 1);

                rank = rank>>1;
            }


            int **ccnf = (int **)malloc(sizeof(int *)*clauses);
            int *cx = (int *)malloc(sizeof(int)*literals);
            for (i = 0; i < clauses; i++)
            {
                ccnf[i] = (int *)malloc(sizeof(int)*(literals+2));
                memcpy(ccnf[i], cnf[i], sizeof(int)*(literals+2));
            }
            memcpy(cx, x, sizeof(int)*literals);

#pragma omp task
            simplify(ccnf, cx, clauses, literals, l<<1, 0);
#pragma omp task
            simplify(cnf, x, clauses, literals, l<<1|1, 0);
#pragma omp taskwait
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
        }
    }

    double t2 = omp_get_wtime();

    printf("%lf",(t2-t1));

    if(!sat)
        printf("UNSATISFIABLE\n");

    fclose(fp1);
    MPI_Finalize();
    return 0;
}
