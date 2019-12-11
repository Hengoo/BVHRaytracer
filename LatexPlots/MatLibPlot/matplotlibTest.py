import matplotlib.pyplot as plt
import numpy as np

inputFolder = "../Data/"
outputFolder = "../Plots/"

def makeWorkGroupAnalysis(filePath, title, outputName):
	fig = plt.figure()  # an empty figure with no axes
	
	#fig.suptitle('No axes on this figure')  # Add a title so we know which it is


	#load the workload file and visualize it.

	#load:
	y, z, a, b, c = np.loadtxt(filePath, delimiter=',', unpack=True, skiprows =1)

	x = np.arange(y.size)
	plt.plot(x,z, label='min', zorder=1)
	plt.plot(x,a, label='max', zorder=2)
	plt.fill_between(x, b,c, label = "mean + - standard deviation", color='m',zorder=3)


	plt.plot(x,y, label='median', linewidth=2, color = 'k', zorder=10)

	#plt.plot(x,c, label='mean + standart deviation')

	plt.xlabel('x label')
	plt.ylabel('y label')

	plt.title(title)

	plt.legend()

	#save to file
	plt.savefig(outputFolder + outputName + '.pdf')
	plt.savefig(outputFolder + outputName + '.pgf')
	plt.show()




makeWorkGroupAnalysis(inputFolder + 'amazonLumberyardInterior_b4_l4_PrimaryWorkGroupWiskerPlot.txt', 'Primary N4L4', "PrimaryN4L4")
makeWorkGroupAnalysis(inputFolder + 'amazonLumberyardInterior_b4_l4_SecondaryWorkGroupWiskerPlot.txt', 'Secondary N4L4', "SecondaryN4L4")