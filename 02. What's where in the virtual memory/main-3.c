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
	void *p;
	unsigned int i;

	printf("Address of a: %p\n", (void *)&a);
	p = malloc(98);
	if (p == NULL)
	{
		fprintf(stderr, "Can't malloc\n");
		return (EXIT_FAILURE);
	}
	printf("Allocated space in the heap: %p\n", p);
	printf("Address of function main: %p\n", (void *)main);
	printf("First bytes of the main function:\n\t");
	for (i = 0; i < 15; i++)
	{
		printf("%02x ", ((unsigned char *)main)[i]);
	}
	printf("\n");
	return (EXIT_SUCCESS);
}
