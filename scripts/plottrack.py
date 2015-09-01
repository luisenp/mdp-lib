import numpy as np
import matplotlib.pyplot as plt

plt.close("all")

ftrack = open('testcss2.track')

[X,Y] = [int(x) for x in ftrack.readline().split()]
numWalls = int(ftrack.readline())
numSlips = int(ftrack.readline())
numStarts = int(ftrack.readline())
numGoals = int(ftrack.readline())

Walls = np.zeros((numWalls,2), np.int8)
Slips = np.zeros((numSlips,2), np.int8)
Starts = np.zeros((numStarts,2), np.int8)
Goals = np.zeros((numGoals,2), np.int8)
k1 = 0
k2 = 0
k3 = 0
k4 = 0
for j in range(Y):
    line = ftrack.readline()
    for i in range(X):
        if line[i] == 'X':
            Walls[k1,:] = [i,Y-j-1]
            k1 = k1 + 1
        if line[i] == '.':
            Slips[k2,:] = [i,Y-j-1]
            k2 = k2 + 1
        if line[i] == 'S':
            Starts[k3,:] = [i,Y-j-1]
            k3 = k3 + 1
        if line[i] == 'G':
            Goals[k4,:] = [i,Y-j-1]
            k4 = k4 + 1
      
plt.plot(Walls[:,0],Walls[:,1],'s', color = '0.25', ms=9, mec='0.25')
plt.plot(Slips[:,0],Slips[:,1],'.', color='0.65', ms=9, mec='0.25')
plt.plot(Starts[:,0],Starts[:,1],'gs')
plt.plot(Goals[:,0],Goals[:,1],'rs')
#plt.text(3,20, 'Start')
#plt.text(60,20, 'Goal')
plt.axis([-1,X,-1,Y])
plt.show()

ftrack.close()