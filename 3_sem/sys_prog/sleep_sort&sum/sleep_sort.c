#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>


int main(int argc, char* argv[])
{
	pid_t chpid;
	int wstatus;
	int sum = 0;
	int step = (argc - 2) / atoi(argv[1]);
	int lower = argc - step;
	int upper = argc;

	for(int i = 2; i < argc; i++)
	{
		chpid = fork();

		if (chpid == 0)
		{
			usleep(atoi(argv[i]) * 10000);
			printf("%d ", atoi(argv[i]));
			return 0;
		}

	}

	for (int i = 0; i < atoi(argv[1]); i++)
	{
		chpid = fork();
		int part = 0;
		if (chpid == 0)
		{
			for (lower; lower < upper; lower++)
			{
				part+= atoi(argv[lower]);
			}
			return part;
		}

		upper = lower;
		lower -= step;
	}

	for(int i = 1; i < argc + atoi(argv[1]); i++)
	{
		wait(&wstatus);
		sum += WEXITSTATUS(wstatus);
	}
	printf("\nsum == %d \n", sum);
	return 0;
}
