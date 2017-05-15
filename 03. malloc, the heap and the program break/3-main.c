#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * main - let's find out which syscall malloc is using
 *
 * Return: EXIT_FAILURE if something failed. Otherwise EXIT_SUCCESS
 */
int main(void)
{
	void *p;

	write(1, "BEFORE MALLOC\n", 14);
	p = malloc(1);
	write(1, "AFTER MALLOC\n", 13);
	printf("%p\n", p);
	getchar();
	return (EXIT_SUCCESS);
}
