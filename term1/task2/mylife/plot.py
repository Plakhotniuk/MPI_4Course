import matplotlib.pyplot as plt
import numpy as np

data = np.loadtxt('data/time.txt')
processes = data[:, 0]
times = data[:, 1]

time1 = times[0]
times = time1 / times


xp = np.linspace(np.min(processes), np.max(processes), 100)
plt.plot(xp, xp, '-', color='blue', label='y = x')

plt.scatter(processes, times)
plt.legend()
plt.grid()
plt.xlabel('processes')
plt.ylabel('acceleration')
plt.title('Acceleration dependence on the number of processes')
plt.savefig('plots/AccelerationRowDecomposition.png')
plt.show()
