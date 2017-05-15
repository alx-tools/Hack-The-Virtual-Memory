#include <stdio.h>
#include <stdlib.h>

/**
 * main - prints the malloc returned address
 *
 * Return: EXIT_FAILURE if something failed. Otherwise EXIT_SUCCESS
 */
int main(void)
{
	void *p;

	p = malloc(1);
	printf("%p\n", p);
	getchar();
	return (EXIT_SUCCESS);
}
