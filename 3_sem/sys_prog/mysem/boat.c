#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>

enum sems{
    SEM_BOAT = 0,
    SEM_TRAP = 1,
    SEM_IN   = 2,
    SEM_EXIT = 3,
};

#define PLUS(SEM_NAME) \
    {struct sembuf command = {SEM_NAME, 1, 0}; if (semop(semid, &command,1)==-1){perror("SEM V "); return;}}
#define PLUS_K(SEM_NAME, K) \
    {struct sembuf command = {SEM_NAME, K, 0}; if(semop(semid, &command, 1)==-1){perror("SEM VK "); return;}}
#define MINUS(SEM_NAME) \
    {struct sembuf command = {SEM_NAME, -1, 0}; if(semop(semid, &command, 1)==-1){perror("SEM P "); return;}}
#define MINUS_K(SEM_NAME, K) \
    {struct sembuf command = {SEM_NAME, -K, 0}; if(semop(semid, &command, 1)==-1){perror("SEM PK "); return;}}
#define WAIT(SEM_NAME) \
    {struct sembuf command = {SEM_NAME, 0, 0}; if(semop(semid, &command, 1)==-1){perror("SEM WAIT "); return;}}


    int check(int semid){
        struct sembuf check = {SEM_EXIT, 0, IPC_NOWAIT};
        int res = semop(semid, &check, 1);
        if (res == -1 && errno == EAGAIN) {
            printf("Break***********************\n");
            return 1;
        }
        else {
            //my_assert("semop", res != -1);
            return 0;
        }
    }


void Passenger(int semid, int i)
{
    //struct sembuf command = {-1, 0, 0};
while(1)
{
    printf("Passenger %d wants to get on the boat\n", i);
    //on_trap
    if(check(semid)) break;
    MINUS(SEM_BOAT);
    if(check(semid)) break;
    MINUS(SEM_TRAP);
    if(check(semid)) break;

    PLUS(SEM_TRAP);
    if(check(semid)) break;
    printf("Passanger %d is on the boat\n", i);
    //WAIT(SEM_IN);
    printf("Passanger %d is in trip\n", i);
    if(check(semid)) break;
    printf("Passanger %d leaves the boat\n", i);
    if(check(semid)) break;
    MINUS(SEM_TRAP);
    if(check(semid)) break;

    PLUS(SEM_BOAT);
    if(check(semid)) break;
    PLUS(SEM_TRAP);
    if(check(semid)) break;
    //on boat, wait for everyone
}
    PLUS(SEM_BOAT);
    printf("Passenger %d goes home\n", i);
    return;

}
void Boat(int semid, int numVoyages, int numPlace)
{
    //struct sembuf command = {-1, 0, 0};
    PLUS_K(SEM_BOAT, numPlace);
    printf("The boat is ready to transfer\n");

    for (int i = 1; i <= numVoyages; i++)
    {
        printf("The boat is ready to be loaded\n");

        PLUS(SEM_IN);
        printf("The boat descends the ladder\n");
        PLUS(SEM_TRAP);
        usleep(1000);
        printf("The boat goes for %d trip\n", i);
        MINUS(SEM_TRAP);

        MINUS(SEM_IN);
        usleep(1000);

    }

    PLUS(SEM_EXIT);
    PLUS(SEM_IN);

    PLUS(SEM_TRAP);
    PLUS_K(SEM_BOAT, numPlace);


    printf("The boat goes home\n");

    return;
}
int main(int argc, char** argv)
{
    if (argc != 4) return 0;

    int NumPeople  = atoi(argv[1]);
    int NumPlace   = atoi(argv[2]);
    int NumVoyages = atoi(argv[3]);

    int semid = semget(IPC_PRIVATE, 4, 0777);
    if (semid < 0){
        perror("Cannot get semaphore id\n");
        return errno;
    }

    if (!fork())
        Boat(semid, NumVoyages, NumPlace);

    for (int i = 1; i <= NumPeople; i++){
        if (!fork())
            Passenger(semid, i);
    }

    for (int i = 0; i <= NumPeople; i++){
        wait(NULL);
    }

    if (semctl(semid, 0 , IPC_RMID, 0) < 0){
        perror("Cannot delete semid\n");
        return errno;
    }

    return 0;
}
