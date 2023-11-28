import matplotlib.pyplot as plt
import pandas as pd

data = pd.read_csv('time.csv')
time = data['time mk sec']
size = data['size byte']


plt.plot(size, time, label=f'Latency = {time[0]} mk sec')
plt.scatter(size, time)
plt.legend()
plt.grid()
plt.xlabel('bytes')
plt.ylabel('time, mk sec')
plt.yscale('log')
plt.xscale('log')
plt.title('Dependence of the transmission time on the size\nlog scale')
plt.savefig('plot')
plt.show()
