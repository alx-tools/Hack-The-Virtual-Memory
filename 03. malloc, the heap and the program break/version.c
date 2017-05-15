#include <stdio.h>
#include <gnu/libc-version.h>

/**
 * main - prints the version of the glibc
 *
 * Return: 0
 */
int main (void)
{
	puts(gnu_get_libc_version());
	return (0);
}
