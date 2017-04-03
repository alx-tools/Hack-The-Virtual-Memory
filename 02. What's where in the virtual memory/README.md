![hack the virtual memory](https://s3-us-west-1.amazonaws.com/holbertonschool/medias/htvn2.png)

## Hack The Virtual Memory, chapter 2: Drawing the VM diagram

We previously talked about what you could find in the virtual memory of a process, and where you could find it. Today, we will try to "reconstruct" (part of) the following diagram by making our process print addresses of various elements of the program.

![the virtual memory](https://s3-us-west-1.amazonaws.com/holbertonschool/medias/virtual_memory.png)

## Prerequisites

In order to fully understand this article, you will need to know:

- The basics of the C programming language
- A little bit of assembly (but not required)
- The very basics of the Linux filesystem and the shell
- We will also use the `/proc/[pid]/maps` file (see `man proc` or read our first article [Hack The Virtual Memory, chapter 0: C strings & /proc](https://blog.holbertonschool.com/hack-the-virtual-memory-c-strings-proc/))

## Environment

All scripts and programs have been tested on the following system:

- Ubuntu
  - Linux ubuntu 4.4.0-31-generic #50~14.04.1-Ubuntu SMP Wed Jul 13 01:07:32 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux
  - **Everything we will write will be true for this system, but may be different on another system**

Tools used:
  
- gcc
  - gcc (Ubuntu 4.8.4-2ubuntu1~14.04.3) 4.8.4
- objdump
  - GNU objdump (GNU Binutils for Ubuntu) 2.24
- udcli
  - udis86 1.7.2
- bc
  - bc 1.06.95

## The stack

The first thing we want to locate in our diagram is the stack. We know that in C, local variables are located on the stack. So if we print the address of a local variable, it should give us an idea on where we would find the stack in the virtual memory. Let's use this program (`main-1.c`) to find out:

```c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * main - print locations of various elements
 *
 * Return: EXIT_FAILURE if something failed. Otherwise EXIT_SUCCESS
 */
int main(void)
{
	int a;

	printf("Address of a: %p\n", (void *)&a);
	return (EXIT_SUCCESS);
}
```

```shell
julien@holberton:~/holberton/w/hackthevm2$ gcc -Wall -Wextra -pedantic -Werror main-0.c -o 0
julien@holberton:~/holberton/w/hackthevm2$ ./0
Address of a: 0x7ffd14b8bd9c
julien@holberton:~/holberton/w/hackthevm2$ 
```

This will be our first point of reference when we will compare other elements' addresses.

## The heap

The heap is used when you malloc space for your variables. Let's add a line to use malloc and see where the memory address returned by `malloc` is located (`main-1.c`):

```c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * main - print locations of various elements
 *
 * Return: EXIT_FAILURE if something failed. Otherwise EXIT_SUCCESS
 */
int main(void)
{
	int a;
	void *p;

	printf("Address of a: %p\n", (void *)&a);
	p = malloc(98);
	if (p == NULL)
	{
		fprintf(stderr, "Can't malloc\n");
		return (EXIT_FAILURE);
	}
	printf("Allocated space in the heap: %p\n", p);
	return (EXIT_SUCCESS);
}
```

```
julien@holberton:~/holberton/w/hackthevm2$ gcc -Wall -Wextra -pedantic -Werror main-1.c -o 1
julien@holberton:~/holberton/w/hackthevm2$ ./1 
Address of a: 0x7ffd4204c554
Allocated space in the heap: 0x901010
julien@holberton:~/holberton/w/hackthevm2$ 
```

It's now clear that the heap (`0x901010`) is way below the stack (`0x7ffd4204c554`). At this point we can already draw this diagram:

![heap and stack](https://s3-us-west-1.amazonaws.com/holbertonschool/medias/virtual_memory_stack_heap.png)

## The executable

Your program is also in the virtual memory. If we print the address of the `main` function, we should have an idea of where the program is located compared to the stack and the heap. Let's see if we find it below the heap as expected (`main-2.c`):

```c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * main - print locations of various elements
 *
 * Return: EXIT_FAILURE if something failed. Otherwise EXIT_SUCCESS
 */
int main(void)
{
	int a;
	void *p;

	printf("Address of a: %p\n", (void *)&a);
	p = malloc(98);
	if (p == NULL)
	{
		fprintf(stderr, "Can't malloc\n");
		return (EXIT_FAILURE);
	}
	printf("Allocated space in the heap: %p\n", p);
	printf("Address of function main: %p\n", (void *)main);
	return (EXIT_SUCCESS);
}
```

```shell
julien@holberton:~/holberton/w/hackthevm2$ gcc -Wall -Wextra -Werror main-2.c -o 2
julien@holberton:~/holberton/w/hackthevm2$ ./2 
Address of a: 0x7ffdced37d74
Allocated space in the heap: 0x2199010
Address of function main: 0x40060d
julien@holberton:~/holberton/w/hackthevm2$ 
```

It seems that our program (`0x40060d`) is located below the heap (`0x2199010`), just as expected.
But let's make sure that this is the actual code of our program, and not some sort of pointer to another location. Let's disassemble our program `2` with [objdump](https://en.wikipedia.org/wiki/Objdump) to look at the "memory address" of the `main` function:

```shell
julien@holberton:~/holberton/w/hackthevm2$ objdump -M intel -j .text -d 2 | grep '<main>:' -A 5
000000000040060d <main>:
  40060d:	55                   	push   rbp
  40060e:	48 89 e5             	mov    rbp,rsp
  400611:	48 83 ec 10          	sub    rsp,0x10
  400615:	48 8d 45 f4          	lea    rax,[rbp-0xc]
  400619:	48 89 c6             	mov    rsi,rax
```

`000000000040060d <main>` -> we find the exact same address (`0x40060d`). If you still have any doubts, you can print the first bytes located at this address, to make sure they match the output of `objdump` (`main-3.c`):

```c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * main - print locations of various elements
 *
 * Return: EXIT_FAILURE if something failed. Otherwise EXIT_SUCCESS
 */
int main(void)
{
	int a;
	void *p;
	unsigned int i;

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
	return (EXIT_SUCCESS);
}
```

```shell
julien@holberton:~/holberton/w/hackthevm2$ gcc -Wall -Wextra -Werror main-3.c -o 3
julien@holberton:~/holberton/w/hackthevm2$ objdump -M intel -j .text -d 3 | grep '<main>:' -A 5
000000000040064d <main>:
  40064d:	55                   	push   rbp
  40064e:	48 89 e5             	mov    rbp,rsp
  400651:	48 83 ec 10          	sub    rsp,0x10
  400655:	48 8d 45 f0          	lea    rax,[rbp-0x10]
  400659:	48 89 c6             	mov    rsi,rax
julien@holberton:~/holberton/w/hackthevm2$ ./3 
Address of a: 0x7ffeff0f13b0
Allocated space in the heap: 0x8b3010
Address of function main: 0x40064d
First bytes of the main function:
	55 48 89 e5 48 83 ec 10 48 8d 45 f0 48 89 c6 
julien@holberton:~/holberton/w/hackthevm2$ echo "55 48 89 e5 48 83 ec 10 48 8d 45 f0 48 89 c6" | udcli -64 -x -o 40064d
000000000040064d 55               push rbp                
000000000040064e 4889e5           mov rbp, rsp            
0000000000400651 4883ec10         sub rsp, 0x10           
0000000000400655 488d45f0         lea rax, [rbp-0x10]     
0000000000400659 4889c6           mov rsi, rax            
julien@holberton:~/holberton/w/hackthevm2$
```

-> We can see that we print the same address and the same content. We are now triple sure this is our `main` function.

_You can download the Udis86 Disassembler Library [here](http://udis86.sourceforge.net/)._

Here is the updated diagram, based on what we have learned:

![stack, heap and executable](https://s3-us-west-1.amazonaws.com/holbertonschool/medias/virtual_memory_stack_heap_executable.png)

## Command line arguments and environment variables

The `main` function can take arguments:

- The command line arguments
  - the first argument of the `main` function (usually named `argc` or `ac`) is the number of command line arguments
  - the second argument of the `main` function (usually named `argv` or `av`) is an array of pointers to the arguments (C strings)
- The environment variables
  - the third argument of the `main` function (usally named `env` or `envp`) is an array of pointers to the environment variables (C strings)

Let's see where those elements stand in the virtual memory of our process (`main-4.c`):

```c
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
	printf("Address of the first environment variable: %p\n", (void *)(env[0]));
        return (EXIT_SUCCESS);
}
```

```shell
julien@holberton:~/holberton/w/hackthevm2$ gcc -Wall -Wextra -Werror main-4.c -o 4
julien@holberton:~/holberton/w/hackthevm2$ ./4 Hello Holberton School!
Address of a: 0x7ffe7d6d8da0
Allocated space in the heap: 0xc8c010
Address of function main: 0x40069d
First bytes of the main function:
	55 48 89 e5 48 83 ec 30 89 7d ec 48 89 75 e0 
Address of the array of arguments: 0x7ffe7d6d8e98
Addresses of the arguments:
	[./4]:0x7ffe7d6da373 [Hello]:0x7ffe7d6da377 [Holberton]:0x7ffe7d6da37d [School!]:0x7ffe7d6da387 
Address of the array of environment variables: 0x7ffe7d6d8ec0
Address of the first environment variables:
	[0x7ffe7d6da38f]:"XDG_VTNR=7"
	[0x7ffe7d6da39a]:"XDG_SESSION_ID=c2"
	[0x7ffe7d6da3ac]:"CLUTTER_IM_MODULE=xim"
julien@holberton:~/holberton/w/hackthevm2$ 
```

These elements are above the stack as expected, but now we know the exact order: `stack` (`0x7ffe7d6d8da0`) < `argv` (`0x7ffe7d6d8e98`) < `env` (`0x7ffe7d6d8ec0`) < arguments (from `0x7ffe7d6da373` to `0x7ffe7d6da387` + `8` (`8` = size of the string `"school"` + `1` for the `'\0'` char)) < environment variables (starting at `0x7ffe7d6da38f`).

Actually, we can also see that all the command line arguments are next to each other in the memory, and also right next to the environment variables.

### Are the `argv` and `env` arrays next to each other?

The array `argv` is 5 elements long (there were 4 arguments from the command line + 1 `NULL` element at the end (`argv` always ends with `NULL` to mark the end of the array)). Each element is a pointer to a `char` and since we are on a 64-bit machine, a pointer is `8` bytes (if you want to make sure, you can use the C operator `sizeof()` to get the size of a pointer). As a result our `argv` array is of size `5 * 8` = `40`. `40` in base `10` is `0x28` in base `16`. If we add this value to the address of the beginning of the array `0x7ffe7d6d8e98`, we get... `0x7ffe7d6d8ec0` (The address of the `env` array)! So the two arrays are next to each other in memory.

### Is the first command line argument stored right after the `env` array? 

In order to check this we need to know the size of the `env` array. We know that it ends with a `NULL` pointer, so in order to get the number of its elements we simply need to loop through it, checking if the "current" element is `NULL`. Here's the updated C code (`main-5.c`):

```c
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
```

```shell
julien@holberton:~/holberton/w/hackthevm2$ ./5 Hello Betty Holberton!
Address of a: 0x7ffc77598acc
Allocated space in the heap: 0x2216010
Address of function main: 0x40069d
First bytes of the main function:
	55 48 89 e5 48 83 ec 40 89 7d dc 48 89 75 d0 
Address of the array of arguments: 0x7ffc77598bc8
Addresses of the arguments:
	[./5]:0x7ffc7759a374 [Hello]:0x7ffc7759a378 [Betty]:0x7ffc7759a37e [Holberton!]:0x7ffc7759a384 
Address of the array of environment variables: 0x7ffc77598bf0
Address of the first environment variables:
	[0x7ffc7759a38f]:"XDG_VTNR=7"
	[0x7ffc7759a39a]:"XDG_SESSION_ID=c2"
	[0x7ffc7759a3ac]:"CLUTTER_IM_MODULE=xim"
Size of the array env: 62 elements -> 496 bytes (0x1f0)
julien@holberton:~/holberton/w/hackthevm2$ bc
bc 1.06.95
Copyright 1991-1994, 1997, 1998, 2000, 2004, 2006 Free Software Foundation, Inc.
This is free software with ABSOLUTELY NO WARRANTY.
For details type `warranty'. 
obase=16
ibase=16
1F0+7FFC77598BF0
7FFC77598DE0
quit
julien@holberton:~/holberton/w/hackthevm2$ 
```

-> `7FFC77598DE0` != (but still <) `0x7ffc7759a374`. So the answer is no :)

### Wrapping up

Let's update our diagram with what we learned.

![virtual memory with command line arguments and environment variables](https://s3-us-west-1.amazonaws.com/holbertonschool/medias/virtual_memory_args_env.png)

## Is the stack realy growing downwards?

Let's call a function and figure this out! If this is true, then the variables of the calling function will be higher in memory than those from the called function (`main-6.c`).

```c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**                                                                                                      
 * f - print locations of various elements                                                               
 *                                                                                                       
 * Returns: nothing                                                                                      
 */
void f(void)
{
     int a;
     int b;
     int c;

     a = 98;
     b = 1024;
     c = a * b;
     printf("[f] a = %d, b = %d, c = a * b = %d\n", a, b, c);
     printf("[f] Adresses of a: %p, b = %p, c = %p\n", (void *)&a, (void *)&b, (void *)&c);
}

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
     f();
     return (EXIT_SUCCESS);
}
```

```shell
julien@holberton:~/holberton/w/hackthevm2$ gcc -Wall -Wextra -Werror main-6.c -o 6
julien@holberton:~/holberton/w/hackthevm2$ ./6
Address of a: 0x7ffdae53ea4c
Allocated space in the heap: 0xf32010
Address of function main: 0x4006f9
First bytes of the main function:
	55 48 89 e5 48 83 ec 40 89 7d dc 48 89 75 d0 
Address of the array of arguments: 0x7ffdae53eb48
Addresses of the arguments:
	[./6]:0x7ffdae54038b 
Address of the array of environment variables: 0x7ffdae53eb58
Address of the first environment variables:
	[0x7ffdae54038f]:"XDG_VTNR=7"
	[0x7ffdae54039a]:"XDG_SESSION_ID=c2"
	[0x7ffdae5403ac]:"CLUTTER_IM_MODULE=xim"
Size of the array env: 62 elements -> 496 bytes (0x1f0)
[f] a = 98, b = 1024, c = a * b = 100352
[f] Adresses of a: 0x7ffdae53ea04, b = 0x7ffdae53ea08, c = 0x7ffdae53ea0c
julien@holberton:~/holberton/w/hackthevm2$ 
```

-> True! (address of var `a` in function `f`) `0x7ffdae53ea04` < `0x7ffdae53ea4c` (address of var `a` in function `main`)

We now update our diagram:

![stack is growing downwards](https://s3-us-west-1.amazonaws.com/holbertonschool/medias/virtual_memory_stack.png)

## `/proc`

Let's double check everything we found so far with `/proc/[pid]/maps` (`man proc` or refer to the first article in this series to learn about the `proc` filesystem if you don't know what it is).

Let's add a `getchar()` to our program so that we can look at its "`/proc`" (`main-7.c`):

```c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**                                                                                                      
 * f - print locations of various elements                                                               
 *                                                                                                       
 * Returns: nothing                                                                                      
 */
void f(void)
{
     int a;
     int b;
     int c;

     a = 98;
     b = 1024;
     c = a * b;
     printf("[f] a = %d, b = %d, c = a * b = %d\n", a, b, c);
     printf("[f] Adresses of a: %p, b = %p, c = %p\n", (void *)&a, (void *)&b, (void *)&c);
}

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
     f();
     getchar();
     return (EXIT_SUCCESS);
}
```

```shell
julien@holberton:~/holberton/w/hackthevm2$ gcc -Wall -Wextra -Werror main-7.c -o 7
julien@holberton:~/holberton/w/hackthevm2$ ./7 Rona is a Legend SRE
Address of a: 0x7fff16c8146c
Allocated space in the heap: 0x2050010
Address of function main: 0x400739
First bytes of the main function:
	55 48 89 e5 48 83 ec 40 89 7d dc 48 89 75 d0 
Address of the array of arguments: 0x7fff16c81568
Addresses of the arguments:
	[./7]:0x7fff16c82376 [Rona]:0x7fff16c8237a [is]:0x7fff16c8237f [a]:0x7fff16c82382 [Legend]:0x7fff16c82384 [SRE]:0x7fff16c8238b 
Address of the array of environment variables: 0x7fff16c815a0
Address of the first environment variables:
	[0x7fff16c8238f]:"XDG_VTNR=7"
	[0x7fff16c8239a]:"XDG_SESSION_ID=c2"
	[0x7fff16c823ac]:"CLUTTER_IM_MODULE=xim"
Size of the array env: 62 elements -> 496 bytes (0x1f0)
[f] a = 98, b = 1024, c = a * b = 100352
[f] Adresses of a: 0x7fff16c81424, b = 0x7fff16c81428, c = 0x7fff16c8142c

```

```shell
julien@holberton:~$ ps aux | grep "./7" | grep -v grep
julien     5788  0.0  0.0   4336   628 pts/8    S+   18:04   0:00 ./7 Rona is a Legend SRE
julien@holberton:~$ cat /proc/5788/maps
00400000-00401000 r-xp 00000000 08:01 171828                             /home/julien/holberton/w/hackthevm2/7
00600000-00601000 r--p 00000000 08:01 171828                             /home/julien/holberton/w/hackthevm2/7
00601000-00602000 rw-p 00001000 08:01 171828                             /home/julien/holberton/w/hackthevm2/7
02050000-02071000 rw-p 00000000 00:00 0                                  [heap]
7f68caa1c000-7f68cabd6000 r-xp 00000000 08:01 136253                     /lib/x86_64-linux-gnu/libc-2.19.so
7f68cabd6000-7f68cadd6000 ---p 001ba000 08:01 136253                     /lib/x86_64-linux-gnu/libc-2.19.so
7f68cadd6000-7f68cadda000 r--p 001ba000 08:01 136253                     /lib/x86_64-linux-gnu/libc-2.19.so
7f68cadda000-7f68caddc000 rw-p 001be000 08:01 136253                     /lib/x86_64-linux-gnu/libc-2.19.so
7f68caddc000-7f68cade1000 rw-p 00000000 00:00 0 
7f68cade1000-7f68cae04000 r-xp 00000000 08:01 136229                     /lib/x86_64-linux-gnu/ld-2.19.so
7f68cafe8000-7f68cafeb000 rw-p 00000000 00:00 0 
7f68cafff000-7f68cb003000 rw-p 00000000 00:00 0 
7f68cb003000-7f68cb004000 r--p 00022000 08:01 136229                     /lib/x86_64-linux-gnu/ld-2.19.so
7f68cb004000-7f68cb005000 rw-p 00023000 08:01 136229                     /lib/x86_64-linux-gnu/ld-2.19.so
7f68cb005000-7f68cb006000 rw-p 00000000 00:00 0 
7fff16c62000-7fff16c83000 rw-p 00000000 00:00 0                          [stack]
7fff16d07000-7fff16d09000 r--p 00000000 00:00 0                          [vvar]
7fff16d09000-7fff16d0b000 r-xp 00000000 00:00 0                          [vdso]
ffffffffff600000-ffffffffff601000 r-xp 00000000 00:00 0                  [vsyscall]
julien@holberton:~$ 
```

Let's check a few things:

- The stack starts at `7fff16c62000` and ends at `7fff16c83000`. Our variables are all inside this region (`0x7fff16c8146c`, `0x7fff16c81424`, `0x7fff16c81428`, `0x7fff16c8142c`)
- The heap starts at `02050000` and ends at `02071000`. Our allocated memory is in there (`0x2050010`)
- Our code (the `main` function) is located at address `0x400739`, so in the following region:
  `00400000-00401000 r-xp 00000000 08:01 171828                             /home/julien/holberton/w/hackthevm2/7`
  It comes from the file `/home/julien/holberton/w/hackthevm2/7` (our executable) and this region has execution permissions, which also makes sense.
- The arguments and environment variables (from `0x7fff16c81568` to `0x7fff16c8238f` + `0x1f0`) are located in the region starting at `7fff16c62000` and ending at `7fff16c83000`... the stack! :) So they are IN the stack, not outside the stack.

This also brings up more questions:

- Why is our executable "divided" into three different regions with different permissions? What is inside these two regions?
  - `00600000-00601000 r--p 00000000 08:01 171828                             /home/julien/holberton/w/hackthevm2/7`
  - `00601000-00602000 rw-p 00001000 08:01 171828                             /home/julien/holberton/w/hackthevm2/7`
- What are all those other regions?
- Why our allocated memory does not start at the very beginning of the heap (`0x2050010` vs `02050000`)? What are those first 16 bytes used for?

There is also another fact that we haven't checked: Is the heap actually growing upwards?

We'll find out another day! But before we end this chapter, let's update our diagram with everything we've learned:

![the virtual memory](https://s3-us-west-1.amazonaws.com/holbertonschool/medias/virtual_memory_args_stack.png)

## Outro

We have learned a ton of things by simply printing informations from our executables! But we still have open questions that we will explore in a future chapter to complete our diagram of the virtual memory. In the meantime, you should try to find out yourself.

### Questions? Feedback?

If you have questions or feedback don't hesitate to ping us on Twitter at [@holbertonschool](https://twitter.com/holbertonschool) or [@julienbarbier42](https://twitter.com/julienbarbier42).
_Haters, please send your comments to `/dev/null`._

Happy Hacking!

### Thank you for reading!

As always, no-one is perfect (except [Chuck](http://codesqueeze.com/the-ultimate-top-25-chuck-norris-the-programmer-jokes/) of course), so don't hesitate to contribute or send me your comments if you find anything I missed.

### Files

This repo contains the source code (`main-X.c` files) for programs created in this tutorial.

### Read more about the virtual memory

Follow [@holbertonschool](https://twitter.com/holbertonschool) or [@julienbarbier42](https://twitter.com/julienbarbier42) on Twitter to get the next chapters! This was the third chapter in our series on the virtual memory. If you missed the previous ones, here are the links to them:

- Chapter 0: [Hack The Virtual Memory: C strings & /proc](https://blog.holbertonschool.com/hack-the-virtual-memory-c-strings-proc/)
- Chapter 1: [Hack The Virtual Memory: Python bytes](https://blog.holbertonschool.com/hack-the-virtual-memory-python-bytes/)

_Many thanks to [Tim](https://twitter.com/wintermanc3r) for English proof-reading!_ :)
