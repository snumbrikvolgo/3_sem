#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <readline/readline.h>
#include <sys/wait.h>
#include <assert.h>




char** make_args(char* line)
{
	int argc = 1;

	char* cur = line;
	while(1)
	{
		while (*cur == ' ')
			cur++;

		cur = strchr (cur, ' ');
		if (!cur)
			break;
		argc++;
		cur++;
	}

	char** argv = (char**) malloc ((argc + 1) * sizeof(*argv));
	cur = strtok(line, " ");
	//printf("%s \n", cur);
	for (int i = 0; i < argc; i++)
	{
		argv[i] = cur;

		cur = strtok(NULL, " ");
		//printf("%s %d\n", cur, i );
	}

	argv[argc] = NULL;
	return argv;
}

int main()
{
	while(1) {
		int wstatus = 0;
		char* input = readline("$ ");
		if (!input)
			return 0;

		char** argv = make_args(input);

		if (!fork()) {
			if (execvp(argv[0], argv) < 0)
				perror(argv[0]);

			return errno;
		}

		wait(&wstatus);

		free(argv);
		free(input);
	}
}
