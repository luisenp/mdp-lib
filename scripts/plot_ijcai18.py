import numpy as np
import matplotlib.pyplot as plt

f = open('/tmp/borra.txt', 'r')

solvers = ('flares(1)',
           'soft-flares(0)',
           'soft-flares-depth(1)',
           'soft-flares-traj(1)',
           'soft-flares-traj-logistic(2)', 
           'soft-flares-traj-exp(2)', 
           'soft-flares-traj-linear(2)',
           'soft-flares-depth-logistic(4)', 
           'soft-flares-depth-exp(4)', 
           'soft-flares-depth-linear(4)')

n = 5
i = 0
problem_results = {}
costs = {}
times = {}
current_problem_str = ''
current_solver_str = ''
for line in f:
  if i % (n + 1) == 0:
    tmp = line.rstrip('\n').split(' | ')
    current_problem_str = tmp[0]
    current_solver_str = tmp[1]
    if current_problem_str not in problem_results:
      problem_results[current_problem_str] = {}
    problem_results[current_problem_str][current_solver_str] = {}
    problem_results[current_problem_str][current_solver_str]['costs'] = []
    problem_results[current_problem_str][current_solver_str]['times'] = []
  else:
    tmp = line.rstrip('\n').split(' ')
    problem_results[current_problem_str][current_solver_str]['costs'].append(
      float(tmp[1]))
    problem_results[current_problem_str][current_solver_str]['times'].append(
      1000 * float(tmp[3]))
  i += 1

for problem in problem_results.keys():
  for solver in solvers:
    order = np.argsort(problem_results[problem][solver]['times'])
    times = np.array(problem_results[problem][solver]['times'])[order]
    costs = np.array(problem_results[problem][solver]['costs'])[order]
    plt.plot(times, costs, label=solver)
  plt.legend()  
  plt.show()

f.close() 