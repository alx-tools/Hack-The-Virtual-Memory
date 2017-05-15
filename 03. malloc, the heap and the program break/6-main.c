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
 * main - using the 0x10 bytes to jump to next malloc'ed chunks
 *
 * Return: EXIT_FAILURE if something failed. Otherwise EXIT_SUCCESS
 */
int main(void)
{
	void *p;
	int i;
	void *heap_start;
	size_t size_of_the_block;

	heap_start = sbrk(0);
	write(1, "START\n", 6);
	for (i = 0; i < 10; i++)
	{
		p = malloc(1024 * (i + 1)); 
		*((int *)p) = i;
		printf("%p: [%i]\n", p, i);
	}
	p = heap_start;
	for (i = 0; i < 10; i++)
	{
		pmem(p, 0x10);
		size_of_the_block = *((size_t *)((char *)p + 8)) - 1;
		printf("%p: [%i] - size = %li\n",
			  (void *)((char *)p + 0x10),
			  *((int *)((char *)p + 0x10)),
			  size_of_the_block);
		p = (void *)((char *)p + size_of_the_block);
	}
	write(1, "END\n", 4);
	return (EXIT_SUCCESS);
}
