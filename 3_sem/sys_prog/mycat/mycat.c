#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>

#define MAX_STRING 30


long size_of_file(int text) ///current position - done
{
	long current = 0;
	current = lseek(text, 0, SEEK_CUR);

	long number_of_symbols = 0;

	number_of_symbols = lseek(text, 0, SEEK_END);
	lseek(text, current, SEEK_SET);

	return number_of_symbols;
}

void show_file (char* name)
{
	int symbols = 0;
	int fd = open(name, O_RDONLY);
	long sz = size_of_file(fd);
	char buffer[MAX_STRING];
	int n = 0;
	while (symbols < sz)
	{
		n = read(fd, buffer, MAX_STRING);
		symbols += n;
		write(1, buffer, n);
	}

	if (fd < 0)
		perror ("No such file or directory\n");
	close (fd);
}

int main (int argc, char* argv[])
{
	int n = -1;
	char string[MAX_STRING];

	if (argc == 1)
	{
		while (n != 0)
		{
			n = read(0, string, MAX_STRING);
			write(1, string, n);
		}

		return 0;
	}
	else
	{
		for (int i = 1; i < argc; i++)
		{
			show_file(argv[i]);
		}
	}

	return 0;
}
