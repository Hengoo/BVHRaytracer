import matplotlib.pyplot as plt
import numpy as np

fig = plt.figure()  # an empty figure with no axes
fig.suptitle('No axes on this figure')  # Add a title so we know which it is


#load the workload file and visualize it.

#load:
y, z, a, b, c = np.loadtxt('Data/amazonLumberyardInterior_b4_l4_CombinedWorkGroupWiskerPlot.txt', delimiter=',', unpack=True, skiprows =1)

x = np.arange(y.size)
plt.plot(x,z, label='min', zorder=1)
plt.plot(x,a, label='max', zorder=2)
plt.fill_between(x, b,c, label = "mean + - standard deviation", color='m',zorder=3)


plt.plot(x,y, label='median', linewidth=2, color = 'k', zorder=10)

#plt.plot(x,c, label='mean + standart deviation')

plt.xlabel('x label')
plt.ylabel('y label')

plt.title("Simple Plot")

plt.legend()

plt.show()

# save to file
#plt.savefig('example.pdf')
#plt.savefig('example.pgf')