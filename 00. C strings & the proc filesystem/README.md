
![hack the vm!](https://s3-us-west-1.amazonaws.com/holbertonschool/medias/hack_the_vm_0.png)

## Intro

### Hack The Virtual Memory: Play with C strings & `/proc`

This is the first in a series of small articles / tutorials based around virtual memory. The goal is to learn some CS basics, but in a different and more practical way.

For this first piece, we'll use `/proc` to find and modify variables (in this example, an ASCII string) contained inside the virtual memory of a running process, and learn some cool things along the way.

## Environment

All scripts and programs have been tested on the following system:

- Ubuntu 14.04 LTS
  - Linux ubuntu 4.4.0-31-generic #50~14.04.1-Ubuntu SMP Wed Jul 13 01:07:32 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux
- gcc
  - gcc (Ubuntu 4.8.4-2ubuntu1~14.04.3) 4.8.4
- Python 3:
  - Python 3.4.3 (default, Nov 17 2016, 01:08:31) 
  - \[GCC 4.8.4\] on linux

## Prerequisites

In order to fully understand this article, you need to know:

- The basics of the C programming language
- Some Python
- The very basics of the Linux filesystem and the shell

## Virtual Memory

In computing, virtual memory is a memory management technique that is implemented using both hardware and software. It maps memory addresses used by a program, called virtual addresses, into physical addresses in computer memory. Main storage (as seen by a process or task) appears as a contiguous address space, or collection of contiguous segments. The operating system manages virtual address spaces and the assignment of real memory to virtual memory. Address translation hardware in the CPU, often referred to as a memory management unit or MMU, automatically translates virtual addresses to physical addresses. Software within the operating system may extend these capabilities to provide a virtual address space that can exceed the capacity of real memory and thus reference more memory than is physically present in the computer.

The primary benefits of virtual memory include freeing applications from having to manage a shared memory space, increased security due to memory isolation, and being able to conceptually use more memory than might be physically available, using the technique of paging.

You can read more about the virtual memory on [Wikipedia](https://en.wikipedia.org/wiki/Virtual_memory).

In the next article, we'll go into more details and do some fact checking on what lies inside the virtual memory and where. For now, here are some key points you should know before you read on:

- Each process has its own virtual memory
- The amount of virtual memory depends on your system's architecture
- Each OS handles virtual memory differently, but for most modern operating systems, the virtual memory of a process looks like this:

![virtual memory](https://s3-us-west-1.amazonaws.com/holbertonschool/medias/virtual_memory.png)

In the high memory addresses you can find (this is a non exhaustive list, there's much more to be found, but that's not today's topic):

- The command line arguments and environment variables
- The stack, growing "downwards". This may seem counter-intuitive, but this is the way the stack is implemented in virtual memory

In the low memory addresses you can find:

- Your executable (it's a little more complicated than that, but this is enough to understand the rest of this article)
- The heap, growing "upwards"

The heap is a portion of memory that is dynamically allocated (i.e. containing memory allocated using `malloc`).

Also, keep in mind that **virtual memory is not the same as RAM**.

## C program

Let's start with this simple C program:

```c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * main - uses strdup to create a new string, and prints the
 * address of the new duplcated string
 *
 * Return: EXIT_FAILURE if malloc failed. Otherwise EXIT_SUCCESS
 */
int main(void)
{
	char *s;

	s = strdup("Holberton");
	if (s == NULL)
	{
		fprintf(stderr, "Can't allocate mem with malloc\n");
		return (EXIT_FAILURE);
	}
	printf("%p\n", (void *)s);
	return (EXIT_SUCCESS);
}
```

### strdup

_Take a moment to think before going further. How do you think `strdup` creates a copy of the string "Holberton"? How can you confirm that?_

.

.

.

`strdup` has to create a new string, so it first has to reserve space for it. The function `strdup` is probably using `malloc`. A quick look at its man page can confirm:

```man
DESCRIPTION
       The  strdup()  function returns a pointer to a new string which is a duplicate of the string s.
       Memory for the new string is obtained with malloc(3), and can be freed with free(3).
```

_Take a moment to think before going further. Based on what we said earlier about virtual memory, where do you think the duplicate string will be located? At a high or low memory address?_

.

.

.

Probably in the lower addresses (in the heap). Let's compile and run our small C program to test our hypothesis:

```shell
julien@holberton:~/holberton/w/hackthevm0$ gcc -Wall -Wextra -pedantic -Werror main.c -o holberton
julien@holberton:~/holberton/w/hackthevm0$ ./holberton 
0x1822010
julien@holberton:~/holberton/w/hackthevm0$ 
```

Our duplicated string is located at the address `0x1822010`. Great. But is this a low or a high memory address?

### How big is the virtual memory of a process

The size of the virtual memory of a process depends on your system architecture. In this example I am using a 64-bit machine, so theoretically the size of each process' virtual memory is 2^64 bytes. In theory, the highest memory address possible is `0xffffffffffffffff` (1.8446744e+19), and the lowest is `0x0`.

`0x1822010` is small compared to `0xffffffffffffffff`, so the duplicated string is probably located at a lower memory address. We will be able to confirm this when we will be looking at the `proc` filesystem).

## The proc filesystem

From `man proc`: 

```
The proc filesystem is a pseudo-filesystem which provides an interface to kernel data structures.  It is commonly mounted at `/proc`.  Most of it is read-only, but some files allow kernel variables to be changed.
```

If you list the contents of your `/proc` directory, you will probably see a lot of files. We will focus on two of them:

- `/proc/[pid]/mem`
- `/proc/[pid]/maps`

### mem

From `man proc`:

```man
      /proc/[pid]/mem
              This file can be used to access the pages of a process's memory
	      through open(2), read(2), and lseek(2).
```

Awesome! So, can we access and modify the entire virtual memory of any process?

### maps

From `man proc`:

```man
      /proc/[pid]/maps
              A  file containing the currently mapped memory regions and their access permissions.
	      See mmap(2) for some further information about memory mappings.

              The format of the file is:

       address           perms offset  dev   inode       pathname
       00400000-00452000 r-xp 00000000 08:02 173521      /usr/bin/dbus-daemon
       00651000-00652000 r--p 00051000 08:02 173521      /usr/bin/dbus-daemon
       00652000-00655000 rw-p 00052000 08:02 173521      /usr/bin/dbus-daemon
       00e03000-00e24000 rw-p 00000000 00:00 0           [heap]
       00e24000-011f7000 rw-p 00000000 00:00 0           [heap]
       ...
       35b1800000-35b1820000 r-xp 00000000 08:02 135522  /usr/lib64/ld-2.15.so
       35b1a1f000-35b1a20000 r--p 0001f000 08:02 135522  /usr/lib64/ld-2.15.so
       35b1a20000-35b1a21000 rw-p 00020000 08:02 135522  /usr/lib64/ld-2.15.so
       35b1a21000-35b1a22000 rw-p 00000000 00:00 0
       35b1c00000-35b1dac000 r-xp 00000000 08:02 135870  /usr/lib64/libc-2.15.so
       35b1dac000-35b1fac000 ---p 001ac000 08:02 135870  /usr/lib64/libc-2.15.so
       35b1fac000-35b1fb0000 r--p 001ac000 08:02 135870  /usr/lib64/libc-2.15.so
       35b1fb0000-35b1fb2000 rw-p 001b0000 08:02 135870  /usr/lib64/libc-2.15.so
       ...
       f2c6ff8c000-7f2c7078c000 rw-p 00000000 00:00 0    [stack:986]
       ...
       7fffb2c0d000-7fffb2c2e000 rw-p 00000000 00:00 0   [stack]
       7fffb2d48000-7fffb2d49000 r-xp 00000000 00:00 0   [vdso]

              The address field is the address space in the process that the mapping occupies.
	      The perms field is a set of permissions:

                   r = read
                   w = write
                   x = execute
                   s = shared
                   p = private (copy on write)

              The offset field is the offset into the file/whatever;
	      dev is the device (major:minor); inode is the inode on that device.   0  indicates
              that no inode is associated with the memory region,
	      as would be the case with BSS (uninitialized data).

              The  pathname field will usually be the file that is backing the mapping.
	      For ELF files, you can easily coordinate with the offset field
              by looking at the Offset field in the ELF program headers (readelf -l).

              There are additional helpful pseudo-paths:

                   [stack]
                          The initial process's (also known as the main thread's) stack.

                   [stack:<tid>] (since Linux 3.4)
                          A thread's stack (where the <tid> is a thread ID).
			  It corresponds to the /proc/[pid]/task/[tid]/ path.

                   [vdso] The virtual dynamically linked shared object.

                   [heap] The process's heap.

              If the pathname field is blank, this is an anonymous mapping as obtained via the mmap(2) function.
	      There is no easy  way  to  coordinate
              this back to a process's source, short of running it through gdb(1), strace(1), or similar.

              Under Linux 2.0 there is no field giving pathname.
```

This means that we can look at the `/proc/[pid]/mem` file to locate the heap of a running process. If we can read from the heap, we can locate the string we want to modify. And if we can write to the heap, we can replace this string with whatever we want.

### pid

A process is an instance of a program, with a unique process ID. This process ID (PID) is used by many functions and system calls to interact with and manipulate processes.

We can use the program `ps` to get the PID of a running process (`man ps`).

## C program

We now have everything we need to write a script or program that finds a string in the heap of a running process and then replaces it with another string (of the same length or shorter). We will work with the following simple program that infinitely loops and prints a "strduplicated" string.

```c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/**              
 * main - uses strdup to create a new string, loops forever-ever
 *                
 * Return: EXIT_FAILURE if malloc failed. Other never returns
 */
int main(void)
{
     char *s;
     unsigned long int i;

     s = strdup("Holberton");
     if (s == NULL)
     {
          fprintf(stderr, "Can't allocate mem with malloc\n");
          return (EXIT_FAILURE);
     }
     i = 0;
     while (s)
     {
          printf("[%lu] %s (%p)\n", i, s, (void *)s);
          sleep(1);
          i++;
     }
     return (EXIT_SUCCESS);
}
```

Compiling and running the above source code should give you this output, and loop indefinitely until you kill the process.

```
julien@holberton:~/holberton/w/hackthevm0$ gcc -Wall -Wextra -pedantic -Werror loop.c -o loop
julien@holberton:~/holberton/w/hackthevm0$ ./loop 
[0] Holberton (0xfbd010)
[1] Holberton (0xfbd010)
[2] Holberton (0xfbd010)
[3] Holberton (0xfbd010)
[4] Holberton (0xfbd010)
[5] Holberton (0xfbd010)
[6] Holberton (0xfbd010)
[7] Holberton (0xfbd010)
...
```

_If you would like, pause the reading now and try to write a script or program that finds a string in the heap of a running process before reading further._

.

.

.

### looking at /proc

Let's run our `loop` program.

```
julien@holberton:~/holberton/w/hackthevm0$ ./loop 
[0] Holberton (0x10ff010)
[1] Holberton (0x10ff010)
[2] Holberton (0x10ff010)
[3] Holberton (0x10ff010)
...
```

The first thing we need to find is the PID of the process.

```shell
julien@holberton:~/holberton/w/hackthevm0$ ps aux | grep ./loop | grep -v grep
julien     4618  0.0  0.0   4332   732 pts/14   S+   17:06   0:00 ./loop
```

In the above example, the PID is 4618 (it will be different each time we run it, and it is probably a different number if you are trying this on your own computer). As a result, the `maps` and `mem` files we want to look at are located in the `/proc/4618` directory:

- `/proc/4618/maps`
- `/proc/4618/mem`

A quick `ls -la` in the directory should give you something like this:

```shell
julien@ubuntu:/proc/4618$ ls -la
total 0
dr-xr-xr-x   9 julien julien 0 Mar 15 17:07 .
dr-xr-xr-x 257 root   root   0 Mar 15 10:20 ..
dr-xr-xr-x   2 julien julien 0 Mar 15 17:11 attr
-rw-r--r--   1 julien julien 0 Mar 15 17:11 autogroup
-r--------   1 julien julien 0 Mar 15 17:11 auxv
-r--r--r--   1 julien julien 0 Mar 15 17:11 cgroup
--w-------   1 julien julien 0 Mar 15 17:11 clear_refs
-r--r--r--   1 julien julien 0 Mar 15 17:07 cmdline
-rw-r--r--   1 julien julien 0 Mar 15 17:11 comm
-rw-r--r--   1 julien julien 0 Mar 15 17:11 coredump_filter
-r--r--r--   1 julien julien 0 Mar 15 17:11 cpuset
lrwxrwxrwx   1 julien julien 0 Mar 15 17:11 cwd -> /home/julien/holberton/w/funwthevm
-r--------   1 julien julien 0 Mar 15 17:11 environ
lrwxrwxrwx   1 julien julien 0 Mar 15 17:11 exe -> /home/julien/holberton/w/funwthevm/loop
dr-x------   2 julien julien 0 Mar 15 17:07 fd
dr-x------   2 julien julien 0 Mar 15 17:11 fdinfo
-rw-r--r--   1 julien julien 0 Mar 15 17:11 gid_map
-r--------   1 julien julien 0 Mar 15 17:11 io
-r--r--r--   1 julien julien 0 Mar 15 17:11 limits
-rw-r--r--   1 julien julien 0 Mar 15 17:11 loginuid
dr-x------   2 julien julien 0 Mar 15 17:11 map_files
-r--r--r--   1 julien julien 0 Mar 15 17:11 maps
-rw-------   1 julien julien 0 Mar 15 17:11 mem
-r--r--r--   1 julien julien 0 Mar 15 17:11 mountinfo
-r--r--r--   1 julien julien 0 Mar 15 17:11 mounts
-r--------   1 julien julien 0 Mar 15 17:11 mountstats
dr-xr-xr-x   5 julien julien 0 Mar 15 17:11 net
dr-x--x--x   2 julien julien 0 Mar 15 17:11 ns
-r--r--r--   1 julien julien 0 Mar 15 17:11 numa_maps
-rw-r--r--   1 julien julien 0 Mar 15 17:11 oom_adj
-r--r--r--   1 julien julien 0 Mar 15 17:11 oom_score
-rw-r--r--   1 julien julien 0 Mar 15 17:11 oom_score_adj
-r--------   1 julien julien 0 Mar 15 17:11 pagemap
-r--------   1 julien julien 0 Mar 15 17:11 personality
-rw-r--r--   1 julien julien 0 Mar 15 17:11 projid_map
lrwxrwxrwx   1 julien julien 0 Mar 15 17:11 root -> /
-rw-r--r--   1 julien julien 0 Mar 15 17:11 sched
-r--r--r--   1 julien julien 0 Mar 15 17:11 schedstat
-r--r--r--   1 julien julien 0 Mar 15 17:11 sessionid
-rw-r--r--   1 julien julien 0 Mar 15 17:11 setgroups
-r--r--r--   1 julien julien 0 Mar 15 17:11 smaps
-r--------   1 julien julien 0 Mar 15 17:11 stack
-r--r--r--   1 julien julien 0 Mar 15 17:07 stat
-r--r--r--   1 julien julien 0 Mar 15 17:11 statm
-r--r--r--   1 julien julien 0 Mar 15 17:07 status
-r--------   1 julien julien 0 Mar 15 17:11 syscall
dr-xr-xr-x   3 julien julien 0 Mar 15 17:11 task
-r--r--r--   1 julien julien 0 Mar 15 17:11 timers
-rw-r--r--   1 julien julien 0 Mar 15 17:11 uid_map
-r--r--r--   1 julien julien 0 Mar 15 17:11 wchan
```

### /proc/pid/maps

As we have seen earlier, the `/proc/pid/maps` file is a text file, so we can directly read it. The content of the `maps` file of our process looks like this:

```shell
julien@ubuntu:/proc/4618$ cat maps
00400000-00401000 r-xp 00000000 08:01 1070052                            /home/julien/holberton/w/funwthevm/loop
00600000-00601000 r--p 00000000 08:01 1070052                            /home/julien/holberton/w/funwthevm/loop
00601000-00602000 rw-p 00001000 08:01 1070052                            /home/julien/holberton/w/funwthevm/loop
010ff000-01120000 rw-p 00000000 00:00 0                                  [heap]
7f144c052000-7f144c20c000 r-xp 00000000 08:01 136253                     /lib/x86_64-linux-gnu/libc-2.19.so
7f144c20c000-7f144c40c000 ---p 001ba000 08:01 136253                     /lib/x86_64-linux-gnu/libc-2.19.so
7f144c40c000-7f144c410000 r--p 001ba000 08:01 136253                     /lib/x86_64-linux-gnu/libc-2.19.so
7f144c410000-7f144c412000 rw-p 001be000 08:01 136253                     /lib/x86_64-linux-gnu/libc-2.19.so
7f144c412000-7f144c417000 rw-p 00000000 00:00 0 
7f144c417000-7f144c43a000 r-xp 00000000 08:01 136229                     /lib/x86_64-linux-gnu/ld-2.19.so
7f144c61e000-7f144c621000 rw-p 00000000 00:00 0 
7f144c636000-7f144c639000 rw-p 00000000 00:00 0 
7f144c639000-7f144c63a000 r--p 00022000 08:01 136229                     /lib/x86_64-linux-gnu/ld-2.19.so
7f144c63a000-7f144c63b000 rw-p 00023000 08:01 136229                     /lib/x86_64-linux-gnu/ld-2.19.so
7f144c63b000-7f144c63c000 rw-p 00000000 00:00 0 
7ffc94272000-7ffc94293000 rw-p 00000000 00:00 0                          [stack]
7ffc9435e000-7ffc94360000 r--p 00000000 00:00 0                          [vvar]
7ffc94360000-7ffc94362000 r-xp 00000000 00:00 0                          [vdso]
ffffffffff600000-ffffffffff601000 r-xp 00000000 00:00 0                  [vsyscall]
```

Circling back to what we said earlier, we can see that the stack (`[stack]`) is located in high memory addresses and the heap (`[heap]`) in the lower memory addresses.

### [heap]

Using the `maps` file, we can find all the information we need to locate our string:

```
010ff000-01120000 rw-p 00000000 00:00 0                                  [heap]
```

The heap:

- Starts at address `0x010ff000` in the virtual memory of the process
- Ends at memory address: `0x01120000`
- Is readable and writable (`rw`)

A quick look back to our (still running) `loop` program:

```
...
[1024] Holberton (0x10ff010)
...
```

-> `0x010ff000` < `0x10ff010` < `0x01120000`. This confirms that our string is located in the heap. More precisely, it is located at index `0x10` of the heap. If we open the `/proc/pid/mem/` file (in this example `/proc/4618/mem`) and seek to the memory address `0x10ff010`, we can write to the heap of the running process, overwriting the "Holberton" string!

Let's write a script or program that does just that. Choose your favorite language and let's do it!

_If you would like, stop reading now and try to write a script or program that finds a string in the heap of a running process, before reading further. The next paragraph will give away the source code of the answer!_

.

.

.

### Overwriting the string in the virtual memory

We'll be using Python 3 for writing the script, but you could write this in any language. Here is the code:

```python
#!/usr/bin/env python3
'''             
Locates and replaces the first occurrence of a string in the heap
of a process    
            
Usage: ./read_write_heap.py PID search_string replace_by_string
Where:           
- PID is the pid of the target process
- search_string is the ASCII string you are looking to overwrite
- replace_by_string is the ASCII string you want to replace
  search_string with
'''

import sys

def print_usage_and_exit():
    print('Usage: {} pid search write'.format(sys.argv[0]))
    sys.exit(1)

# check usage  
if len(sys.argv) != 4:
    print_usage_and_exit()

# get the pid from args
pid = int(sys.argv[1])
if pid <= 0:
    print_usage_and_exit()
search_string = str(sys.argv[2])
if search_string  == "":
    print_usage_and_exit()
write_string = str(sys.argv[3])
if search_string  == "":
    print_usage_and_exit()

# open the maps and mem files of the process
maps_filename = "/proc/{}/maps".format(pid)
print("[*] maps: {}".format(maps_filename))
mem_filename = "/proc/{}/mem".format(pid)
print("[*] mem: {}".format(mem_filename))

# try opening the maps file
try:
    maps_file = open('/proc/{}/maps'.format(pid), 'r')
except IOError as e:
    print("[ERROR] Can not open file {}:".format(maps_filename))
    print("        I/O error({}): {}".format(e.errno, e.strerror))
    sys.exit(1)

for line in maps_file:
    sline = line.split(' ')
    # check if we found the heap
    if sline[-1][:-1] != "[heap]":
        continue
    print("[*] Found [heap]:")

    # parse line
    addr = sline[0]
    perm = sline[1]
    offset = sline[2]
    device = sline[3]
    inode = sline[4]
    pathname = sline[-1][:-1]
    print("\tpathname = {}".format(pathname))
    print("\taddresses = {}".format(addr))
    print("\tpermisions = {}".format(perm))
    print("\toffset = {}".format(offset))
    print("\tinode = {}".format(inode))

    # check if there is read and write permission
    if perm[0] != 'r' or perm[1] != 'w':
        print("[*] {} does not have read/write permission".format(pathname))
        maps_file.close()
        exit(0)

    # get start and end of the heap in the virtual memory
    addr = addr.split("-")
    if len(addr) != 2: # never trust anyone, not even your OS :)
        print("[*] Wrong addr format")
        maps_file.close()
        exit(1)
    addr_start = int(addr[0], 16)
    addr_end = int(addr[1], 16)
    print("\tAddr start [{:x}] | end [{:x}]".format(addr_start, addr_end))

    # open and read mem
    try:
        mem_file = open(mem_filename, 'rb+')
    except IOError as e:
        print("[ERROR] Can not open file {}:".format(mem_filename))
        print("        I/O error({}): {}".format(e.errno, e.strerror))
        maps_file.close()
        exit(1)

    # read heap  
    mem_file.seek(addr_start)
    heap = mem_file.read(addr_end - addr_start)

    # find string
    try:
        i = heap.index(bytes(search_string, "ASCII"))
    except Exception:
        print("Can't find '{}'".format(search_string))
        maps_file.close()
        mem_file.close()
        exit(0)
    print("[*] Found '{}' at {:x}".format(search_string, i))

    # write the new string
    print("[*] Writing '{}' at {:x}".format(write_string, addr_start + i))
    mem_file.seek(addr_start + i)
    mem_file.write(bytes(write_string, "ASCII"))

    # close files
    maps_file.close()
    mem_file.close()

    # there is only one heap in our example
    break

```

Note: You will need to run this script as root, otherwise you won't be able to read or write to the `/proc/pid/mem` file, even if you are the owner of the process.

#### Running the script

```
julien@holberton:~/holberton/w/hackthevm0$ sudo ./read_write_heap.py 4618 Holberton "Fun w vm!"
[*] maps: /proc/4618/maps
[*] mem: /proc/4618/mem
[*] Found [heap]:
	pathname = [heap]
	addresses = 010ff000-01120000
	permisions = rw-p
	offset = 00000000
	inode = 0
	Addr start [10ff000] | end [1120000]
[*] Found 'Holberton' at 10
[*] Writing 'Fun w vm!' at 10ff010
julien@holberton:~/holberton/w/hackthevm0$ 
```

Note that this address corresponds to the one we found manually:

- The heap lies from addresses `0x010ff000` to `0x01120000` in the virtual memory of the running process
- Our string is at index `0x10` in the heap, so at the memory address `0x10ff010`

If we go back to our `loop` program, it should now print "fun w vm!"

```
...
[2676] Holberton (0x10ff010)
[2677] Holberton (0x10ff010)
[2678] Holberton (0x10ff010)
[2679] Holberton (0x10ff010)
[2680] Holberton (0x10ff010)
[2681] Holberton (0x10ff010)
[2682] Fun w vm! (0x10ff010)
[2683] Fun w vm! (0x10ff010)
[2684] Fun w vm! (0x10ff010)
[2685] Fun w vm! (0x10ff010)
...
```

![hack the virtual memory of a process: mind blowing	!](https://s3-us-west-1.amazonaws.com/holbertonschool/medias/blown-mind-explosion-gif.gif)

## Outro

### Questions? Feedback?

If you have questions or feedback don't hesitate to ping us on Twitter at [@holbertonschool](https://twitter.com/holbertonschool) or [@julienbarbier42](https://twitter.com/julienbarbier42).
_Haters, please send your comments to `/dev/null`._

Happy Hacking!

### Thank you for reading!

As always, no-one is perfect (except [Chuck](http://codesqueeze.com/the-ultimate-top-25-chuck-norris-the-programmer-jokes/) of course), so don't hesitate to [contribute](https://github.com/holbertonschool/Hack-The-Virtual-Memory) or send me your comments.

### Files

[This repo](https://github.com/holbertonschool/Hack-The-Virtual-Memory/tree/master/00.%20C%20strings%20%26%20the%20proc%20filesystem) contains the source code for all programs shown in this tutorial:

- `main.c`: the first C program that prints the location of the string and exits
- `loop.c`: the second C program that loops indefinitely
- `read_write_heap.py`: the script used to modify the string in the running C program

### What's next?

In the next piece we'll do almost the same thing, but instead we'll access the memory of a running Python 3 script. It won't be that straightfoward. We'll take this as an excuse to look at some Python 3 internals. If you are curious, try to do it yourself, and find out why the above `read_write_heap.py` script won't work to modify a Python 3 ASCII string.

See you next time and Happy Hacking!

_Many thanks to [Kristine](https://twitter.com/codechick1), [Tim](https://twitter.com/wintermanc3r) for English proof-reading & [Guillaume](https://twitter.com/guillaumesalva) for PEP8 proof-reading :)_
