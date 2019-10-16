#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <readline/readline.h>
#include <sys/wait.h>
#include <assert.h>

#define MAX_STRING 1024

int main(int argc, char** argv)
{
    int wstatus = 0;

    int fd[2];
    if (pipe(fd) < 0) {
        perror("Error: can't call pipe\n");
        return errno;
    }

    struct timespec mt1, mt2;
    long proc_time = 0;

	clock_gettime (CLOCK_REALTIME, &mt1);

	if (!fork()) {
        dup2(fd[1], 1);
        close(fd[1]);
        close(fd[0]);

        if (execvp(argv[1], argv + 1) < 0) {
            perror("Error: can't call exec\n");
            return errno;
        }
	}

    close(fd[1]);

    int n_lines = 0;
    int n_words = 0;
    int n_bytes = 0;
    int in_word = 0;
    int next_word = 0;

    while(1) {
        char buf[MAX_STRING + 1];
        buf[MAX_STRING] = '\0';
        int ch_read = read(fd[0], buf, MAX_STRING);
        buf[ch_read] = '\0';
        if(ch_read < 0) {
            perror("Error: can't read data!\n");
            break;
        }

        if (ch_read == 0) break;
        int i = 0;

        //printf("ch read %d", ch_read);
        while(buf[i]) {

            if(isspace(buf[i]))
                {
                    if (next_word) {
                        // putchar('\n');
                        n_words++;
                        next_word = 0;
                    }
                    while (isspace(buf[i])){
                        if (buf[i] == '\n')
                            n_lines++;
                        i++;
                    }
                    //printf("eto probel\n");
                    in_word = 0;
                    // printf("\'%c\'\n", buf[i]);
                }
            else if (!isspace(buf[i]))
            {
                //printf("eto slovo\n");
                while (!isspace(buf[i]) && buf[i]){
                    // putchar(buf[i]);
                    i++;
                }
                // putchar('\n');
                if (buf[i]){
                    in_word = 1;
                } else
                    next_word = 1;
            }

            if (in_word)
            {
                // putchar('\n');
                //printf("cur %c \n", buf[i]);
                n_words++;
                in_word=0;
                next_word = 0;
            }
            //i++;
        }

        n_bytes += ch_read;

        if(write(1, buf, ch_read) < 0) {
            perror("Error: can't write data!\n");
            break;
        }
        // putchar('\n');
    }


	wait(&wstatus);
	clock_gettime (CLOCK_REALTIME, &mt2);

	proc_time = 1000000 * (mt2.tv_sec - mt1.tv_sec)+(mt2.tv_nsec - mt1.tv_nsec);

	printf("time:%ld\n", proc_time);
    printf("lines:%d\n"
                  "words:%d\n"
                  "bytes:%d\n", n_lines, n_words, n_bytes);
    close(fd[0]);
    // printf ("%d\n", isspace('\0'));
    return 0;

}
