![hack the vm!](https://s3-us-west-1.amazonaws.com/holbertonschool/medias/hack_the_vm_0.png)

## Hack The Virtual Memory, Chapter 1: Python bytes

For this second chapter, we'll do almost the same thing as for [chapter 0: C strings & /proc](https://blog.holbertonschool.com/hack-the-virtual-memory-c-strings-proc/), but instead we'll access the virtual memory of a running Python 3 script. It won't be as straightfoward.
Let's take this as an excuse to look at some Python 3 internals!

## Prerequisites

_This article is based on everything we learned in the previous chapter. Please read (and understand) [chapter 0: C strings & /proc](https://blog.holbertonschool.com/hack-the-virtual-memory-c-strings-proc/) before reading this one._

In order to fully understand this article, you need to know:

- The basics of the C programming language
- Some Python
- The very basics of the Linux filesystem and the shell
- The very basics of the `/proc` filesystem (see [chapter 0: C strings & /proc](https://blog.holbertonschool.com/hack-the-virtual-memory-c-strings-proc/) for an intro on this topic)

## Environment

All scripts and programs have been tested on the following system:

- Ubuntu
  - Linux ubuntu 4.4.0-31-generic #50~14.04.1-Ubuntu SMP Wed Jul 13 01:07:32 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux
- gcc
  - gcc (Ubuntu 4.8.4-2ubuntu1~14.04.3) 4.8.4
- Python 3:
  - Python 3.4.3 (default, Nov 17 2016, 01:08:31) 
  - \[GCC 4.8.4\] on linux

## Python script

We'll first use this script (`main.py`) and try to modify the "string" `Holberton` in the virtual memory of the process running it.

```python
#!/usr/bin/env python3
'''
Prints a b"string" (bytes object), reads a char from stdin
and prints the same (or not :)) string again
'''

import sys

s = b"Holberton"
print(s)
sys.stdin.read(1)
print(s)
```

## About the bytes object

### bytes vs str

As you can see, we are using a bytes object (we use a `b` in front of our string literal) to store our string. This type will store the characters of the string as bytes (vs potentially multibytes - you can read the `unicodeobject.h` to learn more about how Python 3 encodes strings). This ensures that the string will be a succession of ASCII-values in the virtual memory of the process running the script.

_Technically `s` is not a Python string_ (but it doesn't matter in our context):

```shell
julien@holberton:~/holberton/w/hackthevm1$ python3
Python 3.4.3 (default, Nov 17 2016, 01:08:31) 
[GCC 4.8.4] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> s = "Betty"
>>> type(s)
<class 'str'>
>>> s = b"Betty"
>>> type(s)
<class 'bytes'>
>>> quit()
```

### Everything is an object

Everything in Python is an object: integers, strings, bytes, functions, everything. So the line `s = b"Holberton"` should create an object of type `bytes`, and store the string `b"Holberton` somewhere in memory. Probably in the heap since it has to reserve space for the object and the bytes referenced by or stored in the object (at this point we don't know about the exact implementation).

## Running read_write_heap.py against the Python script

_Note: `read_write_heap.py` is a script we wrote in the previous chapter [chapter 0: C strings & /proc](https://blog.holbertonschool.com/hack-the-virtual-memory-c-strings-proc/)_

Let's run the above script and then run our `read_write_heap.py` script:

```shell
julien@holberton:~/holberton/w/hackthevm1$ ./main.py 
b'Holberton'

```

At this point `main.py` is waiting for the user to hit `Enter`. That corresponds to the line `sys.stdin.read(1)` in our code.
Let's run `read_write_heap.py`:

```shell
julien@holberton:~/holberton/w/hackthevm1$ ps aux | grep main.py | grep -v grep
julien     3929  0.0  0.7  31412  7848 pts/0    S+   15:10   0:00 python3 ./main.py
julien@holberton:~/holberton/w/hackthevm1$ sudo ./read_write_heap.py 3929 Holberton "~ Betty ~"
[*] maps: /proc/3929/maps
[*] mem: /proc/3929/mem
[*] Found [heap]:
	pathname = [heap]
	addresses = 022dc000-023c6000
	permisions = rw-p
	offset = 00000000
	inode = 0
	Addr start [22dc000] | end [23c6000]
[*] Found 'Holberton' at 8e192
[*] Writing '~ Betty ~' at 236a192
julien@holberton:~/holberton/w/hackthevm1$ 
```

Easy! As expected, we found the string on the heap and replaced it. Now when we will hit `Enter` in the `main.py` script, it will print `b'~ Betty ~'`:

```shell

b'Holberton'
julien@holberton:~/holberton/w/hackthevm1$ 
```

Wait, WAT?!

![WAT!]()

We found the string "Holberton" and replaced it, but it was not the correct string?
Before we go down the rabbit hole, we have one more thing to check. Our script stops when it finds the first occurence of the string. Let's run it several times to see if there are more occurences of the same string in the heap.

```shell
julien@holberton:~/holberton/w/hackthevm1$ ./main.py 
b'Holberton'

```

```shell

julien@holberton:~/holberton/w/hackthevm1$ ps aux | grep main.py | grep -v grep
julien     4051  0.1  0.7  31412  7832 pts/0    S+   15:53   0:00 python3 ./main.py
julien@holberton:~/holberton/w/hackthevm1$ sudo ./read_write_heap.py 4051 Holberton "~ Betty ~"
[*] maps: /proc/4051/maps
[*] mem: /proc/4051/mem
[*] Found [heap]:
	pathname = [heap]
	addresses = 00bf4000-00cde000
	permisions = rw-p
	offset = 00000000
	inode = 0
	Addr start [bf4000] | end [cde000]
[*] Found 'Holberton' at 8e162
[*] Writing '~ Betty ~' at c82162
julien@holberton:~/holberton/w/hackthevm1$ sudo ./read_write_heap.py 4051 Holberton "~ Betty ~"
[*] maps: /proc/4051/maps
[*] mem: /proc/4051/mem
[*] Found [heap]:
	pathname = [heap]
	addresses = 00bf4000-00cde000
	permisions = rw-p
	offset = 00000000
	inode = 0
	Addr start [bf4000] | end [cde000]
Can't find 'Holberton'
julien@holberton:~/holberton/w/hackthevm1$ 
```

Only one occurence. So where is the string "Holberton" that is used by the script? Where is our Python bytes object in memory? Could it be in the stack? Let's replace "[heap]" by "[stack]"\* in our `read_write_heap.py` script to create the `read_write_stack.py`:

(*) _see [previous article](https://blog.holbertonschool.com/hack-the-virtual-memory-c-strings-proc/), the stack region is called "[stack]" on the `/proc/[pid]/maps` file_

```python
#!/usr/bin/env python3
'''
Locates and replaces the first occurrence of a string in the stack
of a process

Usage: ./read_write_stack.py PID search_string replace_by_string
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
    # check if we found the stack
    if sline[-1][:-1] != "[stack]":
        continue
    print("[*] Found [stack]:")

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

    # get start and end of the stack in the virtual memory
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

    # read stack
    mem_file.seek(addr_start)
    stack = mem_file.read(addr_end - addr_start)

    # find string
    try:
        i = stack.index(bytes(search_string, "ASCII"))
    except Exception:
        print("Can't find '{}'".format(search_string))
        maps_file.close()
        mem_file.close()
        exit(0)
    print("[*] Found '{}' at {:x}".format(search_string, i))

    # write the new stringprint("[*] Writing '{}' at {:x}".format(write_string, addr_start + i))
    mem_file.seek(addr_start + i)
    mem_file.write(bytes(write_string, "ASCII"))

    # close filesmaps_file.close()
    mem_file.close()

    # there is only one stack in our example
    break
```

The above script (`read_write_heap.py`) does exactly the same thing than the previous one (`read_write_stack.py`), the same way. Except we're looking at the stack, instead of the heap. Let's try to find our string in the stack:

```shell
julien@holberton:~/holberton/w/hackthevm1$ ./main.py
b'Holberton'

```

```shell
julien@holberton:~/holberton/w/hackthevm1$ ps aux | grep main.py | grep -v grep
julien     4124  0.2  0.7  31412  7848 pts/0    S+   16:10   0:00 python3 ./main.py
julien@holberton:~/holberton/w/hackthevm1$ sudo ./read_write_stack.py 4124 Holberton "~ Betty ~"
[sudo] password for julien: 
[*] maps: /proc/4124/maps
[*] mem: /proc/4124/mem
[*] Found [stack]:
	pathname = [stack]
	addresses = 7fff2997e000-7fff2999f000
	permisions = rw-p
	offset = 00000000
	inode = 0
	Addr start [7fff2997e000] | end [7fff2999f000]
Can't find 'Holberton'
julien@holberton:~/holberton/w/hackthevm1$ 
```

So our string is not in the heap and not in the stack: Where is it? It's time to dig into Python 3 internals and locate the string using what we will learn. Brace yourselves, the fun will begin now :)

## Locating the string in the virtual memory

_Note: It is important to note that there are many implementations of Python 3. In this article, we are using the original and most commonly used: CPython (coded in C). What we are about to say about Python 3 will only be true for this implementation_.

### id

There is a simple way to know where the object (be careful, the object is **not** the string) is in the virtual memory. CPython has a specific implementation of the [id()](https://docs.python.org/3.4/library/functions.html#id) builtin: `id()` will return the address of the object in memory.

If we add a line to the Python script to print the id of our object, we should get its address (`main_id.py`):

```python
#!/usr/bin/env python3
'''
Prints:
- the address of the bytes object
- a b"string" (bytes object)
reads a char from stdin
and prints the same (or not :)) string again
'''

import sys

s = b"Holberton"
print(hex(id(s)))
print(s)
sys.stdin.read(1)
print(s)
```

```shell
julien@holberton:~/holberton/w/hackthevm1$ ./main_id.py
0x7f343f010210
b'Holberton'

```

-> `0x7f343f010210`. Let's look at `/proc/` to understand where exactly our object is located.

```shell
julien@holberton:/usr/include/python3.4$ ps aux | grep main_id.py | grep -v grep
julien     4344  0.0  0.7  31412  7856 pts/0    S+   16:53   0:00 python3 ./main_id.py
julien@holberton:/usr/include/python3.4$ cat /proc/4344/maps
00400000-006fa000 r-xp 00000000 08:01 655561                             /usr/bin/python3.4
008f9000-008fa000 r--p 002f9000 08:01 655561                             /usr/bin/python3.4
008fa000-00986000 rw-p 002fa000 08:01 655561                             /usr/bin/python3.4
00986000-009a2000 rw-p 00000000 00:00 0 
021ba000-022a4000 rw-p 00000000 00:00 0                                  [heap]
7f343d797000-7f343de79000 r--p 00000000 08:01 663747                     /usr/lib/locale/locale-archive
7f343de79000-7f343df7e000 r-xp 00000000 08:01 136303                     /lib/x86_64-linux-gnu/libm-2.19.so
7f343df7e000-7f343e17d000 ---p 00105000 08:01 136303                     /lib/x86_64-linux-gnu/libm-2.19.so
7f343e17d000-7f343e17e000 r--p 00104000 08:01 136303                     /lib/x86_64-linux-gnu/libm-2.19.so
7f343e17e000-7f343e17f000 rw-p 00105000 08:01 136303                     /lib/x86_64-linux-gnu/libm-2.19.so
7f343e17f000-7f343e197000 r-xp 00000000 08:01 136416                     /lib/x86_64-linux-gnu/libz.so.1.2.8
7f343e197000-7f343e396000 ---p 00018000 08:01 136416                     /lib/x86_64-linux-gnu/libz.so.1.2.8
7f343e396000-7f343e397000 r--p 00017000 08:01 136416                     /lib/x86_64-linux-gnu/libz.so.1.2.8
7f343e397000-7f343e398000 rw-p 00018000 08:01 136416                     /lib/x86_64-linux-gnu/libz.so.1.2.8
7f343e398000-7f343e3bf000 r-xp 00000000 08:01 136275                     /lib/x86_64-linux-gnu/libexpat.so.1.6.0
7f343e3bf000-7f343e5bf000 ---p 00027000 08:01 136275                     /lib/x86_64-linux-gnu/libexpat.so.1.6.0
7f343e5bf000-7f343e5c1000 r--p 00027000 08:01 136275                     /lib/x86_64-linux-gnu/libexpat.so.1.6.0
7f343e5c1000-7f343e5c2000 rw-p 00029000 08:01 136275                     /lib/x86_64-linux-gnu/libexpat.so.1.6.0
7f343e5c2000-7f343e5c4000 r-xp 00000000 08:01 136408                     /lib/x86_64-linux-gnu/libutil-2.19.so
7f343e5c4000-7f343e7c3000 ---p 00002000 08:01 136408                     /lib/x86_64-linux-gnu/libutil-2.19.so
7f343e7c3000-7f343e7c4000 r--p 00001000 08:01 136408                     /lib/x86_64-linux-gnu/libutil-2.19.so
7f343e7c4000-7f343e7c5000 rw-p 00002000 08:01 136408                     /lib/x86_64-linux-gnu/libutil-2.19.so
7f343e7c5000-7f343e7c8000 r-xp 00000000 08:01 136270                     /lib/x86_64-linux-gnu/libdl-2.19.so
7f343e7c8000-7f343e9c7000 ---p 00003000 08:01 136270                     /lib/x86_64-linux-gnu/libdl-2.19.so
7f343e9c7000-7f343e9c8000 r--p 00002000 08:01 136270                     /lib/x86_64-linux-gnu/libdl-2.19.so
7f343e9c8000-7f343e9c9000 rw-p 00003000 08:01 136270                     /lib/x86_64-linux-gnu/libdl-2.19.so
7f343e9c9000-7f343eb83000 r-xp 00000000 08:01 136253                     /lib/x86_64-linux-gnu/libc-2.19.so
7f343eb83000-7f343ed83000 ---p 001ba000 08:01 136253                     /lib/x86_64-linux-gnu/libc-2.19.so
7f343ed83000-7f343ed87000 r--p 001ba000 08:01 136253                     /lib/x86_64-linux-gnu/libc-2.19.so
7f343ed87000-7f343ed89000 rw-p 001be000 08:01 136253                     /lib/x86_64-linux-gnu/libc-2.19.so
7f343ed89000-7f343ed8e000 rw-p 00000000 00:00 0 
7f343ed8e000-7f343eda7000 r-xp 00000000 08:01 136373                     /lib/x86_64-linux-gnu/libpthread-2.19.so
7f343eda7000-7f343efa6000 ---p 00019000 08:01 136373                     /lib/x86_64-linux-gnu/libpthread-2.19.so
7f343efa6000-7f343efa7000 r--p 00018000 08:01 136373                     /lib/x86_64-linux-gnu/libpthread-2.19.so
7f343efa7000-7f343efa8000 rw-p 00019000 08:01 136373                     /lib/x86_64-linux-gnu/libpthread-2.19.so
7f343efa8000-7f343efac000 rw-p 00000000 00:00 0 
7f343efac000-7f343efcf000 r-xp 00000000 08:01 136229                     /lib/x86_64-linux-gnu/ld-2.19.so
7f343f000000-7f343f1b6000 rw-p 00000000 00:00 0 
7f343f1c5000-7f343f1cc000 r--s 00000000 08:01 918462                     /usr/lib/x86_64-linux-gnu/gconv/gconv-modules.cache
7f343f1cc000-7f343f1ce000 rw-p 00000000 00:00 0 
7f343f1ce000-7f343f1cf000 r--p 00022000 08:01 136229                     /lib/x86_64-linux-gnu/ld-2.19.so
7f343f1cf000-7f343f1d0000 rw-p 00023000 08:01 136229                     /lib/x86_64-linux-gnu/ld-2.19.so
7f343f1d0000-7f343f1d1000 rw-p 00000000 00:00 0 
7ffccf1fd000-7ffccf21e000 rw-p 00000000 00:00 0                          [stack]
7ffccf23c000-7ffccf23e000 r--p 00000000 00:00 0                          [vvar]
7ffccf23e000-7ffccf240000 r-xp 00000000 00:00 0                          [vdso]
ffffffffff600000-ffffffffff601000 r-xp 00000000 00:00 0                  [vsyscall]
julien@holberton:/usr/include/python3.4$ 
```

-> Our object is stored in the following memory region: `7f343f000000-7f343f1b6000 rw-p 00000000 00:00 0`, which is not the heap, and not the stack. That confirms what we saw earlier. But that doesn't mean that the string itself is stored in the same memory region. For instance, the `bytes` object could store a pointer to the string, and not a copy of a string. Of course, at this point we could search for our string in this memory region, but we want to understand and be positive we're looking in the right area, and not use "brute force" to find the solution. It's time to learn more about `bytes` objects. 

### `bytesobject.h`

We are using the C implementation of Python (CPython), so let's look at the header file for bytes objects.

_Note: If you don't have the Python 3 header files, you can use this command on `Ubuntu`: `sudo apt-get install python3-dev` to downloadd them on your system. If you are using the exact same environment as me (see the "Environment" section above), you should then be able to see the Python 3 header files in the `/usr/include/python3.4/` directory._

From `bytesobject.h`:

```c
typedef struct {
    PyObject_VAR_HEAD
    Py_hash_t ob_shash;
    char ob_sval[1];

    /* Invariants:
     *     ob_sval contains space for 'ob_size+1' elements.
     *     ob_sval[ob_size] == 0.
     *     ob_shash is the hash of the string or -1 if not computed yet.
     */
} PyBytesObject;
```

What does that tell us?

- A Python 3 `bytes` object is represented internally using a variable of type `PyBytesObject`
- `ob_sval` holds the entire string
- The string ends with `0`
- `ob_size` stores the length of the string (to find `ob_size`, look at the definition of the macro `PyObject_VAR_HEAD` in `objects.h`. We'll look at it later)

So in our example, if we were able to print the `bytes` object, we should see this:

- `ob_sval`: "Holberton" -> Bytes values: `48` `6f` `6c` `62` `65` `72` `74` `6f` `6e` `00`
- `ob_size`: 9

Based on what we did learn previously, this means that the string is "inside" the `bytes` object. So inside the same memory region \o/

What if we didn't know about the way `id` was implemented in CPython? There is actually another way that we can use to find where the string is: looking at the actual object in memory.

### Looking at the `bytes` object in memory

If we want to look directly at the `PyBytesObject` variable, we will need to create a C function, and call this C function from Python. There are different ways to call a C function from Python. We will use the simplest one: using a dynamic library.

#### Creating our C function

So the idea is to create a C function that is called from Python with the object as a parameter, and then "explore" this object to get the exact address of the string (as well as other information about the object).

The function prototype should be: `void print_python_bytes(PyObject *p);`, where `p` is a pointer to our object (so `p` stores the address of our object in the virtual memory). It doesn't have to return anything.

##### `object.h`

You probably have noticed that we don't use a parameter of type `PyBytesObject`. To understand why, let's have a look at the `object.h` header file and see what we can learn from it:

```c
/* Object and type object interface */

/*
Objects are structures allocated on the heap.  Special rules apply to
the use of objects to ensure they are properly garbage-collected.
Objects are never allocated statically or on the stack; they must be
...
*/
```

- "Objects are never allocated statically or on the stack" -> ok, now we know why it was not on the stack.
- "Objects are structures allocated on the heap" -> wait... WAT? We searched for the string in the heap and it was NOT there... I'm confused! We'll discuss this later, in another article :)

What else can we read in this file:

```c
/*
...
Objects do not float around in memory; once allocated an object keeps
the same size and address.  Objects that must hold variable-size data
...
*/
```

- "Objects do not float around in memory; once allocated an object keeps the same size and address". Good to know. That means that if we modify the correct string, it will always be modfied, and the addresses will never change
- "once allocated" -> allocation? but not using the heap? I'm confused! We'll discuss this later in another article :)

```c
/*
...
Objects are always accessed through pointers of the type 'PyObject *'.
The type 'PyObject' is a structure that only contains the reference count
and the type pointer.  The actual memory allocated for an object
contains other data that can only be accessed after casting the pointer
to a pointer to a longer structure type.  This longer type must start
with the reference count and type fields; the macro PyObject_HEAD should be
used for this (to accommodate for future changes).  The implementation
of a particular object type can cast the object pointer to the proper
type and back.
...
*/
```

- "Objects are always accessed through pointers of the type 'PyObject \*'" -> that is why we have to have to take a pointer of type `PyObject` (vs `PyBytesObject`) as the parameter of our function
- "The actual memory allocated for an object contains other data that can only be accessed after casting the pointer to a pointer to a longer structure type." -> So we will have to cast our function parameter to `PyBytesObject *` in order to access all its information. This is possible because the beginning of the `PyBytesObject` starts with a `PyVarObject` which itself starts with a `PyObject`:

```c
/* PyObject_VAR_HEAD defines the initial segment of all variable-size
 * container objects.  These end with a declaration of an array with 1
 * element, but enough space is malloc'ed so that the array actually
 * has room for ob_size elements.  Note that ob_size is an element count,
 * not necessarily a byte count.
 */
#define PyObject_VAR_HEAD      PyVarObject ob_base;
#define Py_INVALID_SIZE (Py_ssize_t)-1

/* Nothing is actually declared to be a PyObject, but every pointer to
 * a Python object can be cast to a PyObject*.  This is inheritance built
 * by hand.  Similarly every pointer to a variable-size Python object can,
 * in addition, be cast to PyVarObject*.
 */
typedef struct _object {
    _PyObject_HEAD_EXTRA
    Py_ssize_t ob_refcnt;
    struct _typeobject *ob_type;
} PyObject;

typedef struct {
    PyObject ob_base;
    Py_ssize_t ob_size; /* Number of items in variable part */
} PyVarObject;
```

-> Here is the `ob_size` that `bytesobject.h` was mentioning.

##### The C function

Based on everything we just learned, the C code is pretty straightforward (`bytes.c`):

```c
#include "Python.h"

/**
 * print_python_bytes - prints info about a Python 3 bytes object
 * @p: a pointer to a Python 3 bytes object
 * 
 * Return: Nothing
 */
void print_python_bytes(PyObject *p)
{
     /* The pointer with the correct type.*/
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
          /* ob_sval is the array of bytes, ending with the value 0:
	     ob_sval[ob_size] == 0 */
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
     /* if this is not a PyBytesObject print an error message */
     else
     {
          fprintf(stderr, "  [ERROR] Invalid Bytes Object\n");
     }
}
```

### Calling the C function from the python script

#### Creating the dynamic library

As we said earlier, we will use the "dynamic library method" to call our C function from Python 3. So we just need to compile our C file with this command:

```
gcc -Wall -Wextra -pedantic -Werror -std=c99 -shared -Wl,-soname,libPython.so -o libPython.so -fPIC -I/usr/include/python3.4 bytes.c
```

_Don't forget to include the Python 3 header files directory: `-I/usr/include/python3.4`_

Hopefully, this should have created a dynamic library called `libPython.so`.

#### Using the dynamic library from Python 3

In order to use our function we simply need to add these lines in the Python script:

```python
import ctypes

lib = ctypes.CDLL('./libPython.so')
lib.print_python_bytes.argtypes = [ctypes.py_object]
```

and call our function this way:

```python
lib.print_python_bytes(s)
```

### The new Python script

Here is the complete source code of the new Python 3 script (`main_bytes.py`):

```python
#!/usr/bin/env python3
'''
Prints:
- the address of the bytes object
- a b"string" (bytes object)
- information about the bytes object
And then:
- reads a char from stdin
- prints the same (or not :)) information again
'''

import sys
import ctypes

lib = ctypes.CDLL('./libPython.so')
lib.print_python_bytes.argtypes = [ctypes.py_object]

s = b"Holberton"
print(hex(id(s)))
print(s)
lib.print_python_bytes(s)

sys.stdin.read(1)

print(hex(id(s)))
print(s)
lib.print_python_bytes(s)
```

Let's run it!

```shell
julien@holberton:~/holberton/w/hackthevm1$ ./main_bytes.py 
0x7f04d721b210
b'Holberton'
[.] bytes object info
  address of the object: 0x7f04d721b210
  size: 9
  trying string: Holberton
  address of the data: 0x7f04d721b230
  bytes: 48 6f 6c 62 65 72 74 6f 6e 00

```

As expected:

- `id()` returns the address of the object itself (`0x7f04d721b210`)
- the size of the data of our object (`ob_size`) is `9`
- the data of our object is "Holberton", `48` `6f` `6c` `62` `65` `72` `74` `6f` `6e` `00` (and it ends with `00` as specified on the header file `bytesobject.h`)

Annnnnnd, we have found the exact address of our string: `0x7f04d721b230` \o/

![monty python]()
_Sorry I had to add at least one Monty Python reference :) ([why](https://docs.python.org/3.4/faq/general.html#why-is-it-called-python))_

## rw_all.py

Now that we undertand a little bit more about what's happening, it's ok to "brute-force" the mapped memory regions. Let's update the script that replaces the string. Instead of looking only in the stack or the heap, let's look in all readable and writeable memory regions of the process. Here's the source code:

```python
#!/usr/bin/env python3
'''
Locates and replaces (if we have permission) all occurrences of
an ASCII string in the entire virtual memory of a process.

Usage: ./rw_all.py PID search_string replace_by_string
Where:
- PID is the pid of the target process
- search_string is the ASCII string you are looking to overwrite
- replace_by_string is the ASCII string you want to replace
search_string with
'''

import sys

def print_usage_and_exit():
    print('Usage: {} pid search write'.format(sys.argv[0]))
    exit(1)

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

# try opening the file
try:
    maps_file = open('/proc/{}/maps'.format(pid), 'r')
except IOError as e:
    print("[ERROR] Can not open file {}:".format(maps_filename))
    print("        I/O error({}): {}".format(e.errno, e.strerror))
    exit(1)

for line in maps_file:
    # print the name of the memory region
    sline = line.split(' ')
    name = sline[-1][:-1];
    print("[*] Searching in {}:".format(name))

    # parse line
    addr = sline[0]
    perm = sline[1]
    offset = sline[2]
    device = sline[3]
    inode = sline[4]
    pathname = sline[-1][:-1]

    # check if there are read and write permissions
    if perm[0] != 'r' or perm[1] != 'w':
        print("\t[\x1B[31m!\x1B[m] {} does not have read/write permissions ({})".format(pathname, perm))
        continue

    print("\tpathname = {}".format(pathname))
    print("\taddresses = {}".format(addr))
    print("\tpermisions = {}".format(perm))
    print("\toffset = {}".format(offset))
    print("\tinode = {}".format(inode))

    # get start and end of the memoy region
    addr = addr.split("-")
    if len(addr) != 2: # never trust anyone
    	print("[*] Wrong addr format")
        maps_file.close()
        exit(1)
    addr_start = int(addr[0], 16)
    addr_end = int(addr[1], 16)
    print("\tAddr start [{:x}] | end [{:x}]".format(addr_start, addr_end))

    # open and read the memory region
    try:
        mem_file = open(mem_filename, 'rb+')
    except IOError as e:
        print("[ERROR] Can not open file {}:".format(mem_filename))
        print("        I/O error({}): {}".format(e.errno, e.strerror))
        maps_file.close()

    # read the memory region
    mem_file.seek(addr_start)
    region = mem_file.read(addr_end - addr_start)

    # find string
    nb_found = 0;
    try:
        i = region.index(bytes(search_string, "ASCII"))
        while (i):
            print("\t[\x1B[32m:)\x1B[m] Found '{}' at {:x}".format(search_string, i))
            nb_found = nb_found + 1
            # write the new string
	    print("\t[:)] Writing '{}' at {:x}".format(write_string, addr_start + i))
            mem_file.seek(addr_start + i)
            mem_file.write(bytes(write_string, "ASCII"))
            mem_file.flush()

            # update our buffer
	    region.write(bytes(write_string, "ASCII"), i)

            i = region.index(bytes(search_string, "ASCII"))
    except Exception:
        if nb_found == 0:
            print("\t[\x1B[31m:(\x1B[m] Can't find '{}'".format(search_string))
    mem_file.close()

# close files
maps_file.close()
```

Let's run it!

```shell
julien@holberton:~/holberton/w/hackthevm1$ ./main_bytes.py 
0x7f37f1e01210
b'Holberton'
[.] bytes object info
  address of the object: 0x7f37f1e01210
  size: 9
  trying string: Holberton
  address of the data: 0x7f37f1e01230
  bytes: 48 6f 6c 62 65 72 74 6f 6e 00

```

```shell
julien@holberton:~/holberton/w/hackthevm1$ ps aux | grep main_bytes.py | grep -v grep
julien     4713  0.0  0.8  37720  8208 pts/0    S+   18:48   0:00 python3 ./main_bytes.py
julien@holberton:~/holberton/w/hackthevm1$ sudo ./rw_all.py 4713 Holberton "~ Betty ~"
[*] maps: /proc/4713/maps
[*] mem: /proc/4713/mem
[*] Searching in /usr/bin/python3.4:
	[!] /usr/bin/python3.4 does not have read/write permissions (r-xp)
...
[*] Searching in [heap]:
	pathname = [heap]
	addresses = 00e26000-00f11000
	permisions = rw-p
	offset = 00000000
	inode = 0
	Addr start [e26000] | end [f11000]
	[:)] Found 'Holberton' at 8e422
	[:)] Writing '~ Betty ~' at eb4422
...
[*] Searching in :
	pathname = 
	addresses = 7f37f1df1000-7f37f1fa7000
	permisions = rw-p
	offset = 00000000
	inode = 0
	Addr start [7f37f1df1000] | end [7f37f1fa7000]
	[:)] Found 'Holberton' at 10230
	[:)] Writing '~ Betty ~' at 7f37f1e01230
...
[*] Searching in [stack]:
	pathname = [stack]
	addresses = 7ffdc3d0c000-7ffdc3d2d000
	permisions = rw-p
	offset = 00000000
	inode = 0
	Addr start [7ffdc3d0c000] | end [7ffdc3d2d000]
	[:(] Can't find 'Holberton'
...
julien@holberton:~/holberton/w/hackthevm1$ 
```

And if we hit enter in the running `main_bytes.py`...

```shell
julien@holberton:~/holberton/w/hackthevm1$ ./main_bytes.py 
0x7f37f1e01210
b'Holberton'
[.] bytes object info
  address of the object: 0x7f37f1e01210
  size: 9
  trying string: Holberton
  address of the data: 0x7f37f1e01230
  bytes: 48 6f 6c 62 65 72 74 6f 6e 00

0x7f37f1e01210
b'~ Betty ~'
[.] bytes object info
  address of the object: 0x7f37f1e01210
  size: 9
  trying string: ~ Betty ~
  address of the data: 0x7f37f1e01230
  bytes: 7e 20 42 65 74 74 79 20 7e 00
julien@holberton:~/holberton/w/hackthevm1$ 
```

BOOM!

![happy]()

## Outro

We managed to modify the string used by our Python 3 script. Awesome! But we still have questions to answer:

- What is the "Holberton" string that is in the `[heap]` memory region?
- How does Python 3 allocate memory outside of the heap?
- If Python 3 is not using the heap, what does it refer to when it says "Objects are structures allocated on the heap" in `object.h`?

That will be for another time :)

In the meantime, if you are too curious to wait for the next article, you can try to find out yourself.

### Questions? Feedback?

If you have questions or feedback don't hesitate to ping us on Twitter at [@holbertonschool](https://twitter.com/holbertonschool) or [@julienbarbier42](https://twitter.com/julienbarbier42).
_Haters, please send your comments to `/dev/null`._

Happy Hacking!

### Thank you for reading!

As always, no-one is perfect (except [Chuck](http://codesqueeze.com/the-ultimate-top-25-chuck-norris-the-programmer-jokes/) of course), so don't hesitate to contribute or send me your comments.

### Files

[This repo]() contains the source code for all scripts and dynamic libraries created in this tutorial:

- `main.py`: the first target
- `main_id.py`: the second target, printing the id of the bytes object
- `main_bytes.py`: the final target, printing also the information about the bytes object, using our dynamic library
- `read_write_heap.py`: the "original" script to find and replace strings in the heap of a process
- `read_write_stack.py`: same but, searches and replaces in the stack instead of the heap
- `rw_all.py`: same but in every memory regions that are readable and writable
- `bytes.c`: the C function to print info about a Python 3 bytes object
