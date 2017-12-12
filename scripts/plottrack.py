#!/usr/bin/python

import argparse
import sys
import numpy as np
import matplotlib.pyplot as plt

def main(argv):  
  parser = argparse.ArgumentParser(description='Plot a racetrack problem.')
  parser.add_argument('-p', '--problem', required=True)  
  args = parser.parse_args()
  problem = args.problem
  
  plt.close("all")
  
  ftrack = open(problem)

  X = int(ftrack.readline())
  Y = int(ftrack.readline())
  track_strings = []
  numWalls = 0
  numSlips = 0
  numStarts = 0
  numGoals = 0
  numSquares = 0
  for j in range(Y):
    line = ftrack.readline()
    numWalls += line.count('X')
    numSlips += line.count('o')
    numStarts += line.count('E')
    numGoals += line.count('G')
    numSquares += line.count(' ')
    track_strings.append(line)

  Walls = np.zeros((numWalls,2), np.int8)
  Slips = np.zeros((numSlips,2), np.int8)
  Starts = np.zeros((numStarts,2), np.int8)
  Goals = np.zeros((numGoals,2), np.int8)
  Squares = np.zeros((numSquares,2), np.int8)
  cnt = [0, 0, 0, 0, 0]
  for j in range(Y):
      line = track_strings[j]
      for i in range(X):
          if line[i] == 'X':
              Walls[cnt[0],:] = [i,Y-j-1]
              cnt[0] = cnt[0] + 1
          if line[i] == 'o':
              Slips[cnt[1],:] = [i,Y-j-1]
              cnt[1] = cnt[1] + 1
          if line[i] == 'E':
              Starts[cnt[2],:] = [i,Y-j-1]
              cnt[2] = cnt[2] + 1
          if line[i] == 'G':
              Goals[cnt[3],:] = [i,Y-j-1]
              cnt[3] = cnt[3] + 1
          if line[i] == ' ':
              Squares[cnt[4],:] = [i,Y-j-1]
              cnt[4] = cnt[4] + 1
        
  print X, Y
  square_size=8
  fig = plt.figure(figsize=(X / 6, Y / 6), dpi=100)
  plt.plot(Walls[:,0],Walls[:,1],'s', color = '0.55', ms=square_size) #, mec='0.25')
  plt.plot(Slips[:,0],Slips[:,1],'ws', color='1.00', ms=square_size, mec='0.25')
  plt.plot(Starts[:,0],Starts[:,1],'gs', ms=square_size)
  plt.plot(Goals[:,0],Goals[:,1],'rs', ms=square_size)
  # plt.plot(Squares[:,0],Squares[:,1],'ws', ms=square_size, mec='0.00')
  ax = plt.axis([-1, X, -1, Y])
  plt.axis("off")
  plt.show()

  ftrack.close()

if __name__ == "__main__":
  main(sys.argv[1:])
