#include <stdio.h>

void func1(void)
{
	int a;
	int b;
	int c;

	a = 98;
	b = 972;
	c = a + b;
	printf("a = %d, b = %d, c = %d\n", a, b, c);
}

void func2(void)
{
	int a;
	int b;
	int c;

	printf("a = %d, b = %d, c = %d\n", a, b, c);
}

int main(void)
{
	func1();
	func2();
	return (0);
}
