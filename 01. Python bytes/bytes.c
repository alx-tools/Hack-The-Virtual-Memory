#include "Python.h"

/**
 * print_python_bytes - prints info about a Python 3 bytes object
 * @p: a pointer to a Python 3 bytes object
 *
 * Return: Nothing
 */
void print_python_bytes(PyObject *p)
{
	/* The pointer with the correct type. */
	PyBytesObject *s;
	unsigned int i;

	printf("[.] bytes object info\n");
	/* casting the PyObject pointer to a PyBytesObject pointer */
	s = (PyBytesObject *)p;
	/* never trust anyone, check that this is actually
	   a PyBytesObject object. */
	if (s && PyBytes_Check(s))
	{
		/* a pointer holds the memory address of the first byte
		   of the data it points to */
		printf("  address of the object: %p\n", (void *)s);
		/* op_size is in the ob_base structure, of type PyVarObject. */
		printf("  size: %ld\n", s->ob_base.ob_size);
		/* ob_sval is the array of bytes ending with the value 0:
		   ob_sval[ob_size] == 0. */
		printf("  trying string: %s\n", s->ob_sval);
		printf("  address of the data: %p\n", (void *)(s->ob_sval));
		printf("  bytes:");
		/* printing each byte at a time, in case this is not
		   a "string". bytes doesn't have to be strings.
		   ob_sval contains space for 'ob_size+1' elements.
		   ob_sval[ob_size] == 0. */
		for (i = 0; i < s->ob_base.ob_size + 1; i++)
		{
			printf(" %02x", s->ob_sval[i] & 0xff);
		}
		printf("\n");
	}
	/* if this is not a PyBytesObject print an error message. */
	else
	{
		fprintf(stderr, "  [ERROR] Invalid Bytes Object\n");
	}
}
