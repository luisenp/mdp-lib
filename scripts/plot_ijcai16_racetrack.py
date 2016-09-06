import numpy as np
import matplotlib.pyplot as plt

from scipy import stats

# Reading data from the racetrack problem.
data = {}
with open('../results_aaai17.txt', 'r') as f:
  for line in f:
    if line.startswith('known'):
      idx_bar = line.find('|')
      track = line[6 : idx_bar - 6]
      algorithm = line[idx_bar + 1 : -1]
      if track not in data:
        data[track] = {}
      data[track][algorithm] = []
      cnt = 0
    elif cnt < 100:
      data[track][algorithm].append(float(line))
      cnt += 1

# Plotting data
algorithms = [#'lrtdp',
              'flares(0)', 
              'flares(1)',
              'hdp(0,0)',
              'hdp(0,1)',
              'hdp(1,0)',
              'hdp(1,1)',
              'ssipp(4)',
              'ssipp(8)']

labels = [#'OPT', 
          'F(0)', 'F(1)', 
          'H(0,0)', 'H(0,1)', 'H(1,0)', 'H(1,1)',
          'S(4)', 'S(8)']
i = 0
fig = plt.figure()
plt.rcParams.update({'font.size': 14})

tracks = ['ring-6', 'square-4', 'square-5', 'ring-5']

for track in tracks:
  print '***** %s *****' % track
  ax = fig.add_subplot(2,2,i)
  ax.set_title(track)
  i += 1  
  results = np.empty((100, len(algorithms)))
  j = 0
  for alg in algorithms:
    results[:, j] = data[track][alg]
    print (alg, np.mean(results[:, j]),
           stats.ttest_ind(results[:,0], results[:,j], equal_var=False))

    j += 1
  plt.boxplot(results, sym='', widths=0.35)
  ax.set_xticklabels(labels, rotation='45')
fig.tight_layout()
fig.show()