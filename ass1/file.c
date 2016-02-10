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

#define FILE_NUMBER 100 //number of files in the directories.
#define FILE_NAME_SIZE 256 //number of permissible characters in file name in linux
#define FILE_PATH_SIZE 4096 //number of permissible characters in file name in linux

void getFiles(char *path, char ***buff, int *offset);
void printContent(char **buff, int size);
void compare2(char **buff1, char **buff2, int size1, int size2);

int main(int argc, char * const *argv)
{
    char *dir1, *dir2, ***buff;
    int j,i,fileno = FILE_NUMBER, c, size1, size2;

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

    buff = (char ***)malloc(2*sizeof(char **));

    for (i = 0; i < 2; i++) 
    {
        buff[i] = (char **)malloc(sizeof(char *)*fileno);

        for(j = 0; j < fileno; j++)
            buff[i][j] = (char *)malloc(sizeof(char)*FILE_PATH_SIZE);
    }

    printf("\n\nFiles in %s\n",dir1);
    getFiles(dir1, buff, &size1);
    printContent(*buff, size1);

    printf("\n\nFiles in %s\n",dir2);
    getFiles(dir2, buff+1, &size2);
    printContent(buff[1], size2);

    //printf("Files %d\nDir1:%s\nDir2:%s\n",fileno,dir1,dir2);
    return 0;
}

/* Used http://www.thegeekstuff.com/2012/06/c-directory/ to write this function*/
void getFiles(char *path, char ***buff, int *offset)
{
    char name[FILE_PATH_SIZE];
    DIR *dp = NULL;
    struct dirent *dptr = NULL;

    if(NULL == (dp = opendir(path)))
    {
        fprintf(stderr,"Cannot directory ar given path [%s]\n",path);
        exit(-1);
    }
    else
    {
        while(NULL != (dptr = readdir(dp)))
        {
            if(path[strlen(path)-1]!='/')
                sprintf(name,"%s/%s",path,dptr->d_name);
            else
                sprintf(name,"%s%s",path,dptr->d_name);

            //printf("%s\n",name);

            if(dptr->d_type == DT_DIR && strcmp(dptr->d_name,".")!=0 && strcmp(dptr->d_name,"..")!=0)
                getFiles(name, buff, offset);
            else if(strcmp(dptr->d_name,".")!=0 && strcmp(dptr->d_name,"..")!=0)
                strcpy(*(*buff+(*offset)++),name);
        }
    }
}

void printContent(char **buff, int size)
{
    int i;
    for(i=0;i<size;i++)
    {
        printf("%s\n",*buff);
        buff++;
    }
}
