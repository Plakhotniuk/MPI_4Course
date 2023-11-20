import matplotlib.pyplot as plt
import numpy as np

data = np.loadtxt('time2.txt')
processes = data[0::10, 0]
data = [min(data[i:i+10, 1]) for i in range(0, data.shape[0], 10)]
# print(data)
time1 = data[0]
data = [time1 / i for i in data]


# def func(x, a, b):
#     return a*x + b
#
# n = 5000
# t = np.polyfit(size[-n:], time[-n:], 1)

# f = np.poly1d(t)
xp = np.linspace(np.min(processes), np.max(processes), 100)
plt.plot(xp, xp, '-', color='blue', label='y = x')

plt.scatter(processes, data, label='measurements')
plt.legend()
plt.grid()
plt.xlabel('n')
plt.ylabel('acceleration')
plt.title('Acceleration dependence on the number of processes')
plt.savefig('plot2')
plt.show()