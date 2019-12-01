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

int check_end(int semid);

#define PLUS(SEM_NAME) \
{struct sembuf command = {SEM_NAME, 1, 0}; if (semop(semid, &command,1) < 0){perror("SEM PLUS "); return errno;}}
#define PLUS_K(SEM_NAME, K) \
{struct sembuf command = {SEM_NAME, K, 0}; if(semop(semid, &command, 1) < 0){perror("SEM PLUS_K "); return errno;}}
#define MINUS(SEM_NAME) \
{struct sembuf command = {SEM_NAME, -1, 0}; if(semop(semid, &command, 1) < 0){perror("SEM MINUS "); return errno;}}
#define MINUS_K(SEM_NAME, K) \
{struct sembuf command = {SEM_NAME, -K, 0}; if(semop(semid, &command, 1) < 0){perror("SEM MINUS_K "); return errno;}}
#define WAIT(SEM_NAME) \
{struct sembuf command = {SEM_NAME, 0, 0}; if(semop(semid, &command, 1) < 0){perror("SEM WAIT "); return errno;}}

int Passenger(int semid, int i, int numPlace)
{
	while(1)
	{
		printf("Passenger %2d wants to get on the boat\n", i);
		MINUS(SEM_BOAT);

		if (check_end(semid))
			break;

		MINUS(SEM_TRAP);
		printf("Passenger %2d: is on trap\n", i);
		PLUS(SEM_TRAP);

		printf("Passenger %2d: entered the boat\n", i);
		WAIT(SEM_IN);

		//должен ждать, пока не закончится трип
		MINUS(SEM_TRAP);
		PLUS(SEM_BOAT);
		PLUS(SEM_TRAP);

		printf("Passenger %2d: is again on the beach\n", i);
	}
	PLUS(SEM_BOAT);
	printf("Passenger %2d: leaves the beach\n", i);
	return 0;


}
int Boat(int semid, int numVoyages, int numPlace)
{
	printf("Boat: goes to the beach\n");
	PLUS_K(SEM_BOAT, numPlace);

	for (int i = 1; i <= numVoyages; i++)
	{
		printf("Boat: ready for trip\n");
		printf("Boat: opens the ladder\n");

		PLUS(SEM_IN);
		PLUS(SEM_TRAP);
		usleep(1000);

		printf("Boat: closes the ladder\n");

		MINUS(SEM_TRAP);
		MINUS(SEM_IN);
		usleep(1000);
	}

	PLUS(SEM_EXIT);
	PLUS(SEM_IN);
	PLUS(SEM_TRAP);
	PLUS_K(SEM_BOAT, numPlace);

	printf("Boat: finish work\n");
	return 0;
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
		exit(Boat(semid, NumVoyages, NumPlace));

	for (int i = 1; i <= NumPeople; i++){
		if (!fork())
			exit(Passenger(semid, i, NumPlace));
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


int check_end(int semid)
{
	struct sembuf check = {SEM_EXIT, 0, IPC_NOWAIT};
	int res = semop(semid, &check, 1);
	if (res == -1 && errno == EAGAIN)
		return 1;

	return 0;
}
