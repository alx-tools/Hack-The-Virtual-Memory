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
