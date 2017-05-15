#include <stdio.h>
#include <stdlib.h>

/**
 * main - 1 call to malloc
 *
 * Return: EXIT_FAILURE if something failed. Otherwise EXIT_SUCCESS
 */
int main(void)
{
	malloc(1);
	getchar();
	return (EXIT_SUCCESS);
}
