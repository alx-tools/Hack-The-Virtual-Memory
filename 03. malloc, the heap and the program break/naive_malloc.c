#include <stdlib.h>
#include <unistd.h>

/**
 * naive_malloc - dynamically allocates memory on the heap using sbrk
 * @size: number of bytes to allocate
 *
 * Return: the memory address newly allocated, or NULL on error
 *
 * Note: don't do this at home :)
 */
void *naive_malloc(size_t size)
{
	void *previous_break;

	previous_break = sbrk(size);
	/* check for error */
	if (previous_break == (void *) -1)
	{
		/* on error malloc returns NULL */
		return (NULL);
	}
	return (previous_break);
}
