import matplotlib.pyplot as plt
import numpy as np

inputFolder = "../Data/"
outputFolder = "../Plots/IntersectionPlots"

showImage = True

def endPlot():
	if showImage:
		plt.show()
	else:
		plt.close()

def makeIntersectionAnalysis(filePath, title, outputName):
	#load the workload file and visualize it.

	#load:
	(branchFactor, leafSize, subdivision, primaryNodeIntersections, primaryLeafIntersections, secondaryNodeIntersections,
		secondaryLeafIntersections, averageLeafDepth, primaryAabb, primaryPrimitive, nodeSah, leafSah, nodeEpo, leafEpo,
		leafVolume, leafSurfaceArea, nodeFullness, primaryAabbSuccessRatio, primaryTriangleSuccessRatio, secondaryAabbSuccessRatio,
		secondaryTriangleSuccessRatio, secondaryAabb, secondaryPrimitive, primaryWasteFactor, secondaryWasteFactor, primaryNodeCachelines,
		secondaryNodeCachelines, totalTime, nodeTime, leafTime, perNodeCost, perLeafCost, sahNodeFactor) = np.loadtxt(filePath, delimiter=',', unpack=True, skiprows=1)

	x = np.arange(branchFactor.size)

	#Node intersections by branching factor.
	plt.title(title)
	filter2 = primaryNodeIntersections[leafSize == 1]
	filter1 = branchFactor[leafSize == 1]
	plt.plot(filter1, filter2, label='L1')
	for i in range(4,17,4):
		filter2 = primaryNodeIntersections[leafSize == i]
		filter1 = branchFactor[leafSize == i]
		plt.plot(filter1, filter2, label='L' + str(i))

	plt.xlabel('Node size')
	plt.ylabel('Primary Node intersections')
	plt.legend()
	#save to file
	plt.savefig(outputFolder + outputName + "PrimaryNodeIntersection.pdf")
	plt.savefig(outputFolder + outputName + "PrimaryNodeIntersection.pgf")
	endPlot()

	#aabb intersections by branching factor.
	plt.title(title)
	for i in range(4,17,4):
		filter2 = primaryAabb[leafSize == i]
		filter1 = branchFactor[leafSize == i]
		plt.plot(filter1, filter2, label='L' + str(i))

	plt.xlabel('Node size')
	plt.ylabel('Primary Aabb intersections')
	plt.legend()
	#save to file
	plt.savefig(outputFolder + outputName + "PrimaryAabbIntersection.pdf")
	plt.savefig(outputFolder + outputName + "PrimaryAabbIntersection.pgf")
	endPlot()


	#Leaf intersections by Leaf size.
	plt.title(title)
	for i in range(4,17,4):
		filter2 = primaryLeafIntersections[branchFactor == i]
		filter1 = leafSize[branchFactor == i]
		plt.plot(filter1, filter2, label='N' + str(i))

	plt.xlabel('Leaf size')
	plt.ylabel('Primary Leaf intersections')
	plt.legend()
	#save to file
	plt.savefig(outputFolder + outputName + "PrimaryLeafIntersection.pdf")
	plt.savefig(outputFolder + outputName + "PrimaryLeafIntersection.pgf")
	endPlot()
	

	#Leaf intersections by Leaf size.
	plt.title(title)
	for i in range(4,17,4):
		filter2 = primaryPrimitive[branchFactor == i]
		filter1 = leafSize[branchFactor == i]
		plt.plot(filter1, filter2, label='N' + str(i))

	plt.xlabel('Leaf size')
	plt.ylabel('Primary Triangle intersections')
	plt.legend()

	#save to file
	plt.savefig(outputFolder + outputName + "PrimaryTriIntersection.pdf")
	plt.savefig(outputFolder + outputName + "PrimaryTriIntersection.pgf")
	endPlot()

makeIntersectionAnalysis(inputFolder + "amazonLumberyardInterior_1To16Table.txt" , "Amazon Lumberyard Interior", "AmazonLumberyardInterior_1To16")