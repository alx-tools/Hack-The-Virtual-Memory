#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * main - moving the program break
 *
 * Return: EXIT_FAILURE if something failed. Otherwise EXIT_SUCCESS
 */
int main(void)
{
	int i;

	write(1, "START\n", 6);
	malloc(1);
	getchar();
	write(1, "LOOP\n", 5);
	for (i = 0; i < 0x25000 / 1024; i++)
	{
		malloc(1024);
	}
	write(1, "END\n", 4);
	getchar();
	return (EXIT_SUCCESS);
}
