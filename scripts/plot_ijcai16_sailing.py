import numpy as np
import matplotlib.pyplot as plt

from scipy import stats

# Reading data from the raceproblem problem.
data = {}
with open('../ijcai_grw_sailing.txt', 'r') as f:
  for line in f:
    if line.find('|') != -1:
      idx_bar = line.find('|')
      problem = line[: idx_bar]
      algorithm = line[idx_bar + 1 : -1]
      if problem not in data:
        data[problem] = {}
      data[problem][algorithm] = []
      cnt = 0
    elif cnt < 100:
      data[problem][algorithm].append(float(line))
      cnt += 1

# Plotting data
algorithms = ['lrtdp',
              'flares(0)', 
              'flares(1)',
              'hdp(0,0)',
              'hdp(0,1)',
              'hdp(1,0)',
              'hdp(1,1)',
              'ssipp(4)',
              'ssipp(8)']

labels = ['OPT', 
          'F(0)', 'F(1)', 
          'H(0,0)', 'H(0,1)', 'H(1,0)', 'H(1,1)',
          'S(4)', 'S(8)']
i = 0
fig = plt.figure()
plt.rcParams.update({'font.size': 14})

problems = {
  '40-39' : ('size=(40,40) goal=(39,39)', 0), 
  '20-10' : ('size=(20,20) goal=(10,10)', 1), 
  '20-19' : ('size=(20,20) goal=(19,19)', 2),
  '40-20' : ('size=(40,40) goal=(20,20)', 3), }




for problem in problems:
  print '***** %s *****' % problem
  ax = fig.add_subplot(2,2,problems[problem][1])
  ax.set_title(problems[problem][0])
  i += 1  
  results = np.empty((100, len(algorithms)))
  j = 0
  for alg in algorithms:
    results[:, j] = data[problem][alg]
    print (alg, np.mean(results[:, j]),
           scipy.stats.ttest_ind(results[:,0], results[:,j], equal_var=False))

    j += 1
  plt.boxplot(results, sym='', widths=0.35)
  ax.set_xticklabels(labels, rotation=45)
fig.tight_layout()
fig.show()