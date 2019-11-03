#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <string.h>
#include <sys/shm.h>

int queue[7] = {0, 1, 2, 3, 3, 4, 5};
char* words[5] = {"do", " ", "pi", "z", "a"};

int P(int semid, struct sembuf* command, int number)
{
    (*command).sem_num = number;
    (*command).sem_op  = 1;
    (*command).sem_flg = 0;

    if (semop(semid, command, 1) < 0){
        perror("Cannot do the operation P with semaphores\n");
        return errno;
    }

    return 0;
}

int V(int semid, struct sembuf* command, int number)
{
    (*command).sem_num = number;
    (*command).sem_op  = -1;
    (*command).sem_flg = 0;

    if (semop(semid, command, 1) < 0){
        perror("Cannot do the operation V with semaphores\n");
        return errno;
    }

    return 0;
}

int Cook (int semid, struct sembuf* command,char * string, int i, int N, int* n_pizza, int* q_index)
{
    while(1)
    {
        //printf("In cook %d n_pizza == %d\n", i, *n_pizza);
        V(semid, command, i);
        if ((*n_pizza) == N)
            break;
        printf("Cook %d: puts \"%s\" \n", i, words[i]);
        P(semid, command, queue[(*q_index) % 7]);
        //printf("In cook %d q_index == %d\n", i, *q_index);

        (*q_index)++;
    }

    return 0;
}

int Chief(int semid,struct sembuf* command, char* string, int N, int* n_pizza, int* q_index)
{
    while(1)
    {
        if ((*n_pizza) == N)
            break;
        V(semid, command, 5);
        printf("Cheif: checks pizza\n\n");
        (*n_pizza)++;
        *q_index = 0;
        P(semid, command, 0);
    }
    P(semid, command, 0);
    P(semid, command, 1);
    P(semid, command, 2);
    P(semid, command, 3);
    P(semid, command, 4);

    return 0;
}
int main(int argc, char** argv)
{
    int N = atoi(argv[1]);

    int shmid1 = shmget(IPC_PRIVATE, 12, 0777);
	if (shmid1 < 0){
		perror("Cannot create shared memory\n");
		return errno;
	}
    int shmid2 = shmget(IPC_PRIVATE, 4, 0777);
	if (shmid2 < 0){
		perror("Cannot create shared memory\n");
		return errno;
	}
    int shmid3 = shmget(IPC_PRIVATE, 4, 0777);
	if (shmid3 < 0){
		perror("Cannot create shared memory\n");
		return errno;
	}

	char* string = shmat(shmid1, NULL, 0);
    int* q_index = shmat(shmid2, NULL, 0);
    int* n_pizza = shmat(shmid3, NULL,0);

	if (string == NULL){
		perror("Cannot attach memory\n");
		return errno;
	}
    if (n_pizza == NULL){
		perror("Cannot attach memory\n");
		return errno;
	}
    if (q_index == NULL){
		perror("Cannot attach memory\n");
		return errno;
	}
    *q_index = 0;
    *n_pizza = 0;

    int semid = semget(IPC_PRIVATE, 6, 0700);
	if (semid < 0){
		perror("Cannot create semaphore\n");
		return errno;
	}

    struct sembuf command = {0, 0 ,0};

    P(semid, &command, 0);
    
    for (int i = 0; i < 5; i++)
    {
        if (!fork())
        {
            Cook(semid, &command, string, i, N, n_pizza, q_index);
            return 0;
        }
    }
    if(!fork())
    {
        Chief(semid, &command, string, N, n_pizza, q_index);
        return 0;
    }
    for (int i = 0; i < 6; i++)
		wait(NULL);

    if (semctl(semid,0, IPC_RMID) < 0){
		perror("Cannot delete semaphores\n");
		return errno;
	}
	if (shmdt(string) < 0){
		perror("Cannot dettach memory\n");
		return errno;
	}
    if (shmdt(n_pizza) < 0){
		perror("Cannot dettach memory\n");
		return errno;
	}
    if (shmdt(q_index) < 0){
		perror("Cannot dettach memory\n");
		return errno;
	}

	if (shmctl(shmid1, IPC_RMID, 0) < 0){
		perror("Cannot delete shared memory\n");
		return  errno;
	}
    if (shmctl(shmid2, IPC_RMID, 0) < 0){
		perror("Cannot delete shared memory\n");
		return  errno;
	}
    if (shmctl(shmid3, IPC_RMID, 0) < 0){
		perror("Cannot delete shared memory\n");
		return  errno;
	}
	return 0;

}
