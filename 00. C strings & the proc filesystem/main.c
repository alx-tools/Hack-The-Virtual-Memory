#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * main - uses strdup to create a new string, and prints the
 * address of the new duplcated string
 *
 * Return: EXIT_FAILURE if malloc failed. Otherwise EXIT_SUCCESS
 */
int main(void)
{
	char *s;

	s = strdup("Holberton");
	if (s == NULL)
	{
		fprintf(stderr, "Can't allocate mem with malloc\n");
		return (EXIT_FAILURE);
	}
	printf("%p\n", (void *)s);
	return (EXIT_SUCCESS);
}
