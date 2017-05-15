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
 * main - moving the program break
 *
 * Return: EXIT_FAILURE if something failed. Otherwise EXIT_SUCCESS
 */
int main(void)
{
	void *p;
	size_t size_of_the_chunk;
	char prev_used;

	p = malloc(0);
	printf("%p\n", p);
	pmem((char *)p - 0x10, 0x10);
	size_of_the_chunk = *((size_t *)((char *)p - 8));
	prev_used = size_of_the_chunk & 1;
	size_of_the_chunk -= prev_used;
	printf("chunk size = %li bytes\n", size_of_the_chunk);
	return (EXIT_SUCCESS);
}
