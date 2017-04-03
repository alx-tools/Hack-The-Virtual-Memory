#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * main - print locations of various elements
 *
 * Return: EXIT_FAILURE if something failed. Otherwise EXIT_SUCCESS
 */
int main(void)
{
	int a;

	printf("Address of a: %p\n", (void *)&a);
	return (EXIT_SUCCESS);
}
