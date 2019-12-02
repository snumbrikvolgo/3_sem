
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <dirent.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>


int main(int argc, char** argv)
{
    struct stat s;
    for (int i = 1; i < argc; i++)
    {
        if (stat(argv[i], &s) < 0){
            perror("No such file or directory\n");
            return errno;
        }
        else {
                if (S_ISREG(s.st_mode))
                    printf("%s\n", argv[i]);
            }
    }
    for (int i = 1; i < argc; i++)
    {
        if (stat(argv[i], &s) < 0){
            perror("No such file or directory\n");
            return errno;
        }
        else{
                if (S_ISDIR(s.st_mode))
                {

                    DIR* d = opendir(argv[i]);
                    if (d == NULL){
                        perror("Cannot open dir\n");
                        return errno;
                    }
                    printf("%s:\n", argv[i]);
                    struct dirent* e = readdir(d);
                    while (e != NULL)
                    {
                        printf("%s\n",e -> d_name);
                        e = readdir(d);
                    }

                }
            }
    }

 return 0;
}
