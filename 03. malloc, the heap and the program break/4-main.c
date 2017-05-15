#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * main - many calls to malloc
 *
 * Return: EXIT_FAILURE if something failed. Otherwise EXIT_SUCCESS
 */
int main(void)
{
	void *p;

	write(1, "BEFORE MALLOC #0\n", 17);
	p = malloc(1024);
	write(1, "AFTER MALLOC #0\n", 16);
	printf("%p\n", p);

	write(1, "BEFORE MALLOC #1\n", 17);
	p = malloc(1024);
	write(1, "AFTER MALLOC #1\n", 16);
	printf("%p\n", p);

	write(1, "BEFORE MALLOC #2\n", 17);
	p = malloc(1024);
	write(1, "AFTER MALLOC #2\n", 16);
	printf("%p\n", p);

	write(1, "BEFORE MALLOC #3\n", 17);
	p = malloc(1024);
	write(1, "AFTER MALLOC #3\n", 16);
	printf("%p\n", p);

	getchar();
	return (EXIT_SUCCESS);
}
