import matplotlib.pyplot as plt
import numpy as np

inputFolder = "../Data/CachePlot/"
outputFolder = "../Plots/"

showImage = True

	#TODO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! need to think about what plots should start at 0

def endPlot():
	if showImage:
		plt.show()
	else:
		plt.cla()

def perRayCacheMiss(cacheSize):
	filePath = inputFolder + "amazonLumberyardInteriorPerRayCacheMiss_c0_" + str(cacheSize) + ".txt"
	(id, missC0) = np.loadtxt(filePath, delimiter=',', unpack=True, skiprows=1)
	filePath = inputFolder + "amazonLumberyardInteriorPerRayCacheMiss_c1_" + str(cacheSize) + ".txt"
	(id, missC1) = np.loadtxt(filePath, delimiter=',', unpack=True, skiprows=1)


	plt.plot(id, missC0, label="camera0")
	plt.plot(id , missC1, label = "camera1")
	plt.xticks(np.arange(0, cacheSize + 16, step=16))
	plt.ylabel("cache Misses")
	plt.xlabel("Ray Id")
	plt.legend()

	print(np.sum(missC0))
	print(np.sum(missC1))

	plt.savefig(outputFolder + "PerRayCacheMiss.pgf")
	plt.savefig(outputFolder + "PerRayCacheMiss.pdf")
	endPlot()

perRayCacheMiss(256)