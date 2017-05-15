#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * pmem - print mem
 * @p: memory address to start printing from
 * @bytes: number of bytes to print
 *
 * Return: nothing
 */
void pmem(void *p, unsigned int bytes)
{
	unsigned char *ptr;
	unsigned int i;

	ptr = (unsigned char *)p;
	for (i = 0; i < bytes; i++)
	{
		if (i != 0)
		{
			printf(" ");
		}
		printf("%02x", *(ptr + i));
	}
	printf("\n");
}

/**
 * main - confirm the source code
 *
 * Return: EXIT_FAILURE if something failed. Otherwise EXIT_SUCCESS
 */
int main(void)
{
	void *p;
	int i;
	size_t size_of_the_chunk;
	size_t size_of_the_previous_chunk;
	void *chunks[10];

	for (i = 0; i < 10; i++)
	{
		p = malloc(1024 * (i + 1));
		chunks[i] = (void *)((char *)p - 0x10);
		printf("%p\n", p);
	}
	free((char *)(chunks[3]) + 0x10);
	free((char *)(chunks[7]) + 0x10);
	for (i = 0; i < 10; i++)
	{
		p = chunks[i];
		printf("chunks[%d]: ", i);
		pmem(p, 0x10);
		size_of_the_chunk = *((size_t *)((char *)p + 8)) - 1;
		size_of_the_previous_chunk = *((size_t *)((char *)p));
		printf("chunks[%d]: %p, size = %li, prev = %li\n",
			  i, p, size_of_the_chunk, size_of_the_previous_chunk);
	}
	return (EXIT_SUCCESS);
}
