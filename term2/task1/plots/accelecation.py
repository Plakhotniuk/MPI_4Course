import matplotlib.pyplot as plt
import numpy as np

data = np.loadtxt('../data/time.txt')
processes = data[:, 1]
times = data[:, 0]

time1 = times[0]
times = time1 / times

plt.plot(processes, times)
plt.grid()
plt.xlabel('threads')
plt.ylabel('acceleration')
plt.title('Acceleration dependence on the number of threads')
plt.savefig('acceleration.png')
plt.show()
