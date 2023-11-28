import matplotlib.pyplot as plt
import numpy as np


def data_post_processing(data):
    processes = data[0::10, 0]
    data = [min(data[i:i+10, 1]) for i in range(0, data.shape[0], 10)]
    time1 = data[0]
    data = [time1 / i for i in data]
    return data, processes


data_row = np.loadtxt('../task2/mylife/data/time.txt')
data_row, processes_row = data_post_processing(data_row)

data_column = np.loadtxt('../task3/mylife/data/time.txt')
data_column, processes_column = data_post_processing(data_column)

data_cartesian = np.loadtxt('../task5/mylife/data/time.txt')
data_cartesian, processes_cartesian = data_post_processing(data_cartesian)

xp = np.linspace(np.min(processes_cartesian), np.max(processes_cartesian), 100)
plt.plot(xp, xp, '-', color='black', label='y = x')

plt.plot(processes_row, data_row, color='red', label='row decomposition')
plt.plot(processes_column, data_column, color='green', label='column decomposition')
plt.plot(processes_cartesian, data_cartesian, color='blue', label='cartesian decomposition')

plt.legend()
plt.grid()
plt.xlabel('processes')
plt.ylabel('acceleration')
plt.title('Acceleration dependence on the number of processes')
plt.savefig('DecompositionsComparison.png')
plt.show()
