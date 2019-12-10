#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>

#include <sys/shm.h>


struct timespec times = {2,0};

#define PLUS(SEM_NAME) \
{struct sembuf command = {SEM_NAME, 1, 0}; if (semtimedop(semid, &command,1, &times ) < 0){perror("SEM PLUS "); return errno;}}
#define PLUS_K(SEM_NAME, K) \
{struct sembuf command = {SEM_NAME, K, 0}; if(semop(semid, &command, 1) < 0){perror("SEM PLUS_K "); return errno;}}
#define MINUS(SEM_NAME) \
{struct sembuf command = {SEM_NAME, -1, 0}; if(semop(semid, &command, 1) < 0){perror("SEM MINUS "); return errno;}}
#define MINUS_K(SEM_NAME, K) \
{struct sembuf command = {SEM_NAME, -K, 0}; if(semop(semid, &command, 1) < 0){perror("SEM MINUS_K "); return errno;}}
#define WAIT(SEM_NAME) \
{struct sembuf command = {SEM_NAME, 0, 0}; if(semop(semid, &command, 1) < 0){perror("SEM WAIT "); return errno;}}


int main(int argc, char* argv[])
{
    int fd[2];
    int number = 0;
    char c = 0;

    int semid = semget(IPC_PRIVATE, 1, 0777);
	if (semid < 0){
		perror("Cannot get semaphore id\n");
		return errno;
	}


    if (pipe(fd) < 0){
        perror("Cannot make pipe\n");
        return errno;
    }

    if (!fork()) {
        close(fd[0]);
        while(1){

        write(fd[1], &c, 1);
        PLUS(0);
    }
        return 0;


    }
    if (!fork()) {

        while(1)
        {
        if (sem_timedwait(0, &times) < 0)
        {   printf("error\n");
            break;
        }
        number++;

        }
        printf("%d\n",number);
        return 0;
    }
    wait(NULL);
    wait(NULL);


	if (semctl(semid, 0 , IPC_RMID, 0) < 0){
		perror("Cannot delete semid\n");
		return errno;
	}


    return 0;
}
