import matplotlib.pyplot as plt
import numpy as np

inputFolder = "../Data/"
outputFolder = "../Plots/"

showImage = False

def endPlot(plt):
	if showImage:
		plt.show()
	else:
		plt.cla()

def makePerfAnalysis(filePath, title, outputName):
	fig = plt.figure() # an empty figure with no axes

	#load:
	(branchFactor, leafSize, subdivision, primaryNodeIntersections, primaryLeafIntersections, secondaryNodeIntersections,
		secondaryLeafIntersections, averageLeafDepth, primaryAabb, primaryPrimitive, nodeSah, leafSah, nodeEpo, leafEpo,
		leafVolume, leafSurfaceArea, nodeFullness, primaryAabbSuccessRatio, primaryTriangleSuccessRatio, secondaryAabbSuccessRatio,
		secondaryTriangleSuccessRatio, secondaryAabb, secondaryPrimitive, primaryWasteFactor, secondaryWasteFactor, primaryNodeCachelines,
		secondaryNodeCachelines, totalTime, nodeTime, leafTime, perNodeCost, perLeafCost, sahNodeFactor) = np.loadtxt(filePath, delimiter=',', unpack=True, skiprows=1)
	
	#Total time by branch factor.
	plt.title(title)
	for i in range(4,17,4):
		filter2 = totalTime[leafSize == i]
		filter1 = branchFactor[leafSize == i]
		plt.plot(filter1, filter2, label='L' + str(i))

	plt.xlabel('Node size')
	plt.ylabel('Render time')
	plt.legend()
	#save to file
	plt.savefig(outputFolder + outputName + "RenderTimePerBranch.pdf")
	plt.savefig(outputFolder + outputName + "RenderTimePerBranch.pgf")
	endPlot(plt)

	#total time by leaf size
	plt.title(title)
	for i in range(4,17,4):
		filter2 = totalTime[branchFactor == i]
		filter1 = leafSize[branchFactor == i]
		plt.plot(filter1, filter2, label='N' + str(i))

	plt.xlabel('Leaf size')
	plt.ylabel('Render time')
	plt.legend()
	#save to file
	plt.savefig(outputFolder + outputName + "RenderTimePerLeaf.pdf")
	plt.savefig(outputFolder + outputName + "RenderTimePerLeaf.pgf")
	endPlot(plt)

	#Total time by branch factor.
	plt.title(title)
	for i in range(4,17,4):
		filter2 = nodeTime[leafSize == i]
		filter1 = branchFactor[leafSize == i]
		plt.plot(filter1, filter2, label='L' + str(i))

	plt.xlabel('Node size')
	plt.ylabel('Node render time')
	plt.legend()
	#save to file
	plt.savefig(outputFolder + outputName + "NodeRenderTime.pdf")
	plt.savefig(outputFolder + outputName + "NodeRenderTime.pgf")
	endPlot(plt)

	#total time by leaf size
	plt.title(title)
	for i in range(4,17,4):
		filter2 = leafTime[branchFactor == i]
		filter1 = leafSize[branchFactor == i]
		plt.plot(filter1, filter2, label='N' + str(i))

	plt.xlabel('Leaf size')
	plt.ylabel('Leaf render time')
	plt.legend()
	#save to file
	plt.savefig(outputFolder + outputName + "LeafRenderTime.pdf")
	plt.savefig(outputFolder + outputName + "LeafRenderTime.pgf")
	endPlot(plt)

	#Node factor
	plt.title(title)
	for i in range(4,17,4):
		filter2 = sahNodeFactor[leafSize == i]
		filter1 = branchFactor[leafSize == i]
		plt.plot(filter1, filter2, label='L' + str(i))

	plt.xlabel('Node size')
	plt.ylabel('Sah Node Factor')
	plt.legend()
	#save to file
	plt.savefig(outputFolder + outputName + "NodeFactorBranch.pdf")
	plt.savefig(outputFolder + outputName + "NodeFactorBranch.pgf")
	endPlot(plt)

	#Node factor
	plt.title(title)
	for i in range(4,17,4):
		filter2 = sahNodeFactor[branchFactor == i]
		filter1 = leafSize[branchFactor == i]
		plt.plot(filter1, filter2, label='N' + str(i))

	plt.xlabel('Leaf size')
	plt.ylabel('Sah Node Factor')
	plt.legend()
	#save to file
	plt.savefig(outputFolder + outputName + "NodeFactorLeaf.pdf")
	plt.savefig(outputFolder + outputName + "NodeFactorLeaf.pgf")
	endPlot(plt)

def makeIntersectionAnalysis(filePath, title, outputName):
	fig = plt.figure()  # an empty figure with no axes

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
	endPlot(plt)

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
	endPlot(plt)


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
	endPlot(plt)
	

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
	endPlot(plt)

def makeWorkGroupAnalysis(filePath, title, outputName):
	fig = plt.figure() # an empty figure with no axes
	
	plt.title(title)


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


	plt.legend()

	#save to file
	plt.savefig(outputFolder + outputName + '.pdf')
	plt.savefig(outputFolder + outputName + '.pgf')
	endPlot(plt)


makePerfAnalysis(inputFolder + "amazonLumberyardInterior_4To16Table.txt", "Amazon Lumberyard Interior Sse", "AmazonLumberyardInterior_4To16Perf")
makeIntersectionAnalysis(inputFolder + "amazonLumberyardInterior_1To16Table.txt" , "Amazon Lumberyard Interior", "AmazonLumberyardInterior_1To16")

#makeWorkGroupAnalysis(inputFolder + 'amazonLumberyardInterior_b4_l4_PrimaryWorkGroupWiskerPlot.txt', 'Primary N4L4', "PrimaryN4L4WorkGroupAnalysis")
#makeWorkGroupAnalysis(inputFolder + 'amazonLumberyardInterior_b4_l4_SecondaryWorkGroupWiskerPlot.txt', 'Secondary N4L4', "SecondaryN4L4WorkGroupAnalysis")