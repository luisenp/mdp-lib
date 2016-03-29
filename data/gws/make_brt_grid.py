#!/usr/bin/python

import sys

n = int(sys.argv[1])

grid = 'G@'
for i in range(2 * n - 2):
  grid += '.'
grid += '\n'
for i in range(n / 2):
  grid += '.'
  for i in range(2 * n - 1):
    grid += '@'
  grid += '\n'
for i in range(n / 2 + 1, n - 1):
  for i in range(2 * n):
    grid += '.'
  grid += '\n'
grid += '.'
for i in range(2 * n - 1):
  grid += 'x'
grid += '\n'
grid += 'S'
for i in range(2 * n - 2):
  grid += '.'
grid += 'G'
  
print grid