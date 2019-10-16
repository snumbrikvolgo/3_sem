#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct msgbuf {
	long mtype;       /* message type, must be > 0 */
	char mtext[1024];    /* message data */
};

void Runner(int id, int N, int i)
{
	struct msgbuf message = {};
	snprintf(message.mtext, 1024, "Runner %d:exists\n", i);
	printf("Runner %2d:Exists\n", i);

	message.mtype = i;

	if (msgsnd(id, &message, 1024, 0) < 0)
	{
		perror("Runner cannot send message about existion\n");
		return errno;
	}

	if (msgrcv(id, &message, 1024, N + i, 0) < 0)
	{
		perror("Runner cannot get the stick");
		return errno;
	}

	printf("Runner %2d:Go, next\n", i);

	message.mtype = N + i + 1;

	if (msgsnd(id, &message, 1024 , 0) < 0)
	{
		perror("Runner cannot pass the stick");
		return errno;
	}

	return;
}

void Judge(int id, int N)
{
	printf("Judge    :I'm judge\n");
	struct msgbuf message = {};

	for (long i = 1; i <= N; i++)
	{
		if (msgrcv(id, &message, 1024, 0, 0) < 0)
		{
			perror("Judge cannot receive the message about registrants\n");
			return errno;
		}
		printf("Judge    :I've registered runner %ld\n", i);
	}

	snprintf(message.mtext, 1024, "Judge    :Start\n");
	printf("Judge    :First one, start!\n");

	message.mtype = N + 1;

	if (msgsnd(id, &message, 1024, 0) < 0)
	{
		perror("Judge cannot send message about start\n");
		return errno;
	}

	if (msgrcv(id, &message, 1024, 2*N + 1, 0) < 0)
	{
		perror("Competition cannot be finished\n");
		return errno;
	}
	printf("Judge    :Got the stick from the last runner\n");
	printf("Judge    :You've finished the Competition!\n");

	return;
}

int main(int argc, char* argv[])
{
	struct msqid_ds queue = {};

	int id = msgget(IPC_PRIVATE, IPC_CREAT | 0777);

	if (id < 0)
	{
		perror("Invalid id of the message queue\n");
		return errno;
	}

	for (long i = 1; i <= atoi(argv[1]); i++)
		if (!fork())
		{
			Runner(id, atoi(argv[1]), i);
			return 0;
		}

	if(!fork())
	{
		Judge(id, atoi(argv[1]));
		return 0;
	}

	for (int i = 0; i <= atoi(argv[1]); i++)
		wait(NULL);

	if (msgctl(id, IPC_RMID, &queue) < 0)
	{
		perror("The queue cannot be deleted\n");
		return errno;
	}

	return 0;
}
