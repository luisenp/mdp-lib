#!/usr/bin/python

import numpy as np
import matplotlib.pyplot as plt

plt.close("all")

fgrid = open('map12.gw')
grid = [line.rstrip('\n') for line in fgrid]
fgrid.close()

height = len(grid)
width = len(grid[0])

def append_indices(the_list, height):
  the_list[0].append(x)
  the_list[1].append(height - y)
  

obstacles = [[],[]]
dangerous = [[],[]]
starts = [[],[]]
goals = [[],[]]
cells = [[],[]]
for x in range(width):
  for y in range(height):
    if grid[y][x] == 'x':
      append_indices(obstacles, height)
    elif grid[y][x] == '@':
      append_indices(dangerous, height)
    elif grid[y][x] == 'S':
      append_indices(starts, height)
    elif grid[y][x] == 'G':
      append_indices(goals, height)
    elif grid[y][x] == '.':
      append_indices(cells, height)

plt.plot(obstacles[0],obstacles[1], 'sk', ms=7, mec='k')
plt.plot(dangerous[0],dangerous[1], 'sr', ms=7, mec='k')
plt.plot(cells[0],cells[1], 'sw',  ms=7, mec='k')
plt.plot(starts[0],starts[1], 'sb',  ms=7, mec='k')
plt.plot(goals[0],goals[1], 'sg',  ms=7, mec='k')
plt.axis([-1, width + 1, -1, height + 1])
plt.xticks(size=16)
plt.yticks(size=16)
plt.show()