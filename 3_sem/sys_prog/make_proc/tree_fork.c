#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
	pid_t pid;
	pid_t ppid, chpid;
    int wstatus;
	printf("I'm parent,  my pid = %d, my ppid = %d\n",  getpid());
	for(int i = 0; i < 5; i++)
	{
		chpid = fork();

        //if (chpid != 0)
        //    wait(&wstatus);
		if (chpid == 0)
		{
            sleep(3);
			printf("I'm child %2d, my pid = %ld, my ppid = %ld\n", i, (long)getpid(), (long)getppid());
			return 0;
		}

	}
    for(int i = 0; i < 5; i++)
	{

        wait(0);

	}

	return 0;
}
