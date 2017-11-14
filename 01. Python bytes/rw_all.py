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
if search_string == "":
    print_usage_and_exit()
write_string = str(sys.argv[3])
if write_string == "":
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
    name = sline[-1][:-1]
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
        print("\t[\x1B[31m!\x1B[m] {} does not have read/write\
         permissions ({})".format(pathname, perm))
        continue

    print("\tpathname = {}".format(pathname))
    print("\taddresses = {}".format(addr))
    print("\tpermisions = {}".format(perm))
    print("\toffset = {}".format(offset))
    print("\tinode = {}".format(inode))

    # get start and end of the memoy region
    addr = addr.split("-")
    if len(addr) != 2:  # never trust anyone
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
    nb_found = 0
    try:
        i = region.index(bytes(search_string, "ASCII"))
        while (i):
            print("\t[\x1B[32m:)\x1B[m] Found '{}' at\
             {:x}".format(search_string, i))
            nb_found = nb_found + 1
            # write the new string
            print("\t[:)] Writing '{}' at\
             {:x}".format(write_string, addr_start + i))
            mem_file.seek(addr_start + i)
            mem_file.write(bytes(write_string, "ASCII"))
            mem_file.flush()

            # update our buffer
            region.write(bytes(write_string, "ASCII"), i)

            i = region.index(bytes(search_string, "ASCII"))
    except Exception:
        if nb_found == 0:
            print("\t[\x1B[31m:(\x1B[m] Can't find\
             '{}'".format(search_string))
    mem_file.close()

# close files
maps_file.close()
