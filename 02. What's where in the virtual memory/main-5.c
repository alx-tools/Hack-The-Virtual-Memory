#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * main - print locations of various elements
 *
 * Return: EXIT_FAILURE if something failed. Otherwise EXIT_SUCCESS
 */
int main(int ac, char **av, char **env)
{
	int a;
	void *p;
	int i;
	int size;

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
	printf("Address of the array of arguments: %p\n", (void *)av);
	printf("Addresses of the arguments:\n\t");
	for (i = 0; i < ac; i++)
	{
		printf("[%s]:%p ", av[i], av[i]);
	}
	printf("\n");
	printf("Address of the array of environment variables: %p\n", (void *)env);
	printf("Address of the first environment variables:\n");
	for (i = 0; i < 3; i++)
	{
		printf("\t[%p]:\"%s\"\n", env[i], env[i]);
	}
	/* size of the env array */
	i = 0;
	while (env[i] != NULL)
	{
		i++;
	}
	i++; /* the NULL pointer */
	size = i * sizeof(char *);
	printf("Size of the array env: %d elements -> %d bytes (0x%x)\n", i, size, size);
	return (EXIT_SUCCESS);
}
