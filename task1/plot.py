import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

data = pd.read_csv('time.csv')
time = data['time mk sec']
size = data[' size byte']

# def func(x, a, b):
#     return a*x + b
#
# n = 5000
# t = np.polyfit(size[-n:], time[-n:], 1)

# f = np.poly1d(t)
# xp = np.linspace(np.min(size[-n:]), np.max(size[-n:]), 100)
#
# plt.plot(xp, f(xp), '-', color='blue', label=f'Аппроксимация\n y = {round(t[0], 2)}x + {round(t[1], 2)}')

plt.scatter(size, time, label=f'Latency = {time[0]} mk sec')
plt.legend()
plt.grid()
plt.xlabel('bytes')
plt.ylabel('time, mk sec')
plt.yscale('log')
plt.xscale('log')
plt.title('Dependence of the transmission time on the size\nlog scale')
plt.savefig('plot')
plt.show()
