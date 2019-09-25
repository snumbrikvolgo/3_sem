#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
	pid_t chpid;
    int wstatus;

	for(int i = 0; i < 5; i++)
	{
		chpid = fork();

		if (chpid != 0)
		{
			sleep(3);
            		wait(0);

			return 0;
		}
		printf("I'm child %d, my pid = %d, my ppid = %d\n", i, getpid(), getppid());

	}

	return 0;
}
