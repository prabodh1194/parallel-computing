/*
 * Given two directory paths, we'll compare contents of all the files in those
 * two directories.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#define FILE 100 //number of files in the directories.

int main(int argc, char * const *argv)
{
    char *dir1, *dir2;
    int fileno = FILE, c;

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

    printf("Files %d\n",fileno);
    return 0;
}
