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
 * main - the 0x10 lost bytes
 *
 * Return: EXIT_FAILURE if something failed. Otherwise EXIT_SUCCESS
 */
int main(void)
{
	void *p;
	int i;
	
	for (i = 0; i < 10; i++)
	{
		p = malloc(1024 * (i + 1));
		printf("%p\n", p);
		printf("bytes at %p:\n", (void *)((char *)p - 0x10));
		pmem((char *)p - 0x10, 0x10);
	}
	return (EXIT_SUCCESS);
}
