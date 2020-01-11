import matplotlib.pyplot as plt
import numpy as np

inputFolder = "../Data/"
outputFolder = "../Plots/"

showImage = False

	#TODO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! need to think about what plots should start at 0

def endPlot():
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
	endPlot()

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
	endPlot()

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
	endPlot()

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
	endPlot()

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
	endPlot()

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
	endPlot()

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

def makeWorkGroupWiskerPlots(filePath, title, outputName):
	plt.title(title)

	#load:
	y, z, a, b, c = np.loadtxt(filePath, delimiter=',', unpack=True, skiprows =1)

	x = np.arange(y.size)
	plt.plot(x,z, label='min', zorder=1)
	plt.plot(x,a, label='max', zorder=2)#
	plt.fill_between(x, b,c, label = "mean + - standard deviation", color='m',zorder=3)


	plt.plot(x,y, label='median', linewidth=2, color = 'k', zorder=10)

	#plt.plot(x,c, label='mean + standart deviation')

	plt.xlabel('x label')
	plt.ylabel('y label')


	plt.legend()

	#save to file
	plt.savefig(outputFolder + outputName + '.pdf')
	plt.savefig(outputFolder + outputName + '.pgf')
	endPlot()

def mask(array):
	#prepare for masking arrays - 'conventional' arrays won't do it
	maskArray = np.ma.array(array)
	
	#mask values below a certain threshold, but i want the axis to touch 0
	#array = np.ma.masked_where(array <= 0 , array)
	for i in range(len(array)-2):
		index = i+1
		if(array[index-1] == 0 and array[index] == 0 and array[index+1] == 0):
			maskArray[index] = np.ma.masked
	if array[0] == 0 and array[1] == 0:
		maskArray [0] = np.ma.masked
	if array[-1] == 0 and array[-2] == 0:
		maskArray [-1] = np.ma.masked
	return maskArray

def makeWorkGroupAnalysis(filePath, title, workGroupSize, outputName):

	filePath = filePath[0] + str(workGroupSize) + filePath[1]
	title = title + str(workGroupSize)
	outputName = outputName[0] + str(workGroupSize) + outputName[1]
	(stepId, avgPrimaryNodeWork, avgPrimaryNodeUnique, avgPrimaryLeafWork, avgPrimaryLeafUnique, 
		avgPrimaryRayTermination, avgSecondaryNodeWork, avgSecondaryNodeUnique,
		avgSecondaryLeafWork, avgSecondaryLeafUnique, avgSecondaryRayTermination) = np.loadtxt(filePath, delimiter=',', unpack=True, skiprows =1)

	#find highers step id where secondary data is != 0
	secondaryEndId = 0
	for(currentId, a, b, c, d, e) in zip(stepId , avgSecondaryNodeWork, avgSecondaryNodeUnique, avgSecondaryLeafWork, avgSecondaryLeafUnique, avgSecondaryRayTermination) :
		if(a + b + c + d + e != 0):
			secondaryEndId = currentId
	secondaryEndId = secondaryEndId + 5

	#mask those arrays i want to mask:
	avgPrimaryNodeWork = mask(avgPrimaryNodeWork)
	avgPrimaryNodeUnique = mask(avgPrimaryNodeUnique)
	avgPrimaryLeafWork = mask(avgPrimaryLeafWork)
	avgPrimaryLeafUnique = mask(avgPrimaryLeafUnique)
	avgPrimaryRayTermination = mask(avgPrimaryRayTermination)

	avgSecondaryNodeWork = mask(avgSecondaryNodeWork)
	avgSecondaryNodeUnique = mask(avgSecondaryNodeUnique)
	avgSecondaryLeafWork = mask(avgSecondaryLeafWork)
	avgSecondaryLeafUnique = mask(avgSecondaryLeafUnique)
	avgSecondaryRayTermination = mask(avgSecondaryRayTermination)

	plt.figure(figsize=(15,7))

	#first the overall node and leaf work + how many rays terminated
	plt.subplot(2,2,1)
	plt.title("Primary ")
	plt.axhline(linewidth=1, color='0.5')
	plt.axhline(y = workGroupSize * workGroupSize, linewidth=1, color='0.5')
	plt.plot(stepId, avgPrimaryNodeWork, label = "Nodes")
	plt.plot(stepId, avgPrimaryLeafWork, label = "Leafs")
	plt.plot(stepId, avgPrimaryRayTermination, label = "Finished Rays")
	xlimSave = plt.xlim()
	plt.legend()

	plt.subplot(2,2,3)
	plt.title("Secondary")
	plt.axhline(linewidth=1, color='0.5')
	plt.axhline(y = workGroupSize * workGroupSize, linewidth=1, color='0.5')
	plt.plot(stepId, avgSecondaryNodeWork, label = "Nodes")
	plt.plot(stepId, avgSecondaryLeafWork, label = "Leafs")
	plt.plot(stepId, avgSecondaryRayTermination, label = "Finished Rays")
	plt.xlim(xlimSave)
	plt.xlabel("Step Id")
	plt.legend()

	# second part is unique nodes and leafs
	plt.subplot(2,2,2)
	plt.title("Unique Nodes and Leafs loaded per Step (Primary Ray)")
	plt.axhline(linewidth=1, color='0.5')
	plt.plot(stepId, avgPrimaryNodeUnique, label = "Unique Nodes")
	plt.plot(stepId, avgPrimaryLeafUnique, label = "Unique Leafs")
	plt.xlim(xlimSave)
	plt.legend()

	plt.subplot(2,2,4)
	plt.title("Unique Nodes and Leafs loaded per Step (Secondary ray)")
	plt.axhline(linewidth=1, color='0.5')
	plt.plot(stepId, avgSecondaryNodeUnique, label = "Unique Nodes")
	plt.plot(stepId, avgSecondaryLeafUnique, label = "Unique Leafs")
	plt.xlim(xlimSave)
	plt.xlabel("Step Id")
	plt.legend()

	plt.savefig(outputFolder + outputName + ".pdf")
	plt.savefig(outputFolder + outputName + ".pgf")
	endPlot()

#makePerfAnalysis(inputFolder + "amazonLumberyardInterior_4To16Table.txt", "Amazon Lumberyard Interior Sse", "AmazonLumberyardInterior_4To16Perf")
#makeIntersectionAnalysis(inputFolder + "amazonLumberyardInterior_1To16Table.txt" , "Amazon Lumberyard Interior", "AmazonLumberyardInterior_1To16")

#makeWorkGroupAnalysis(inputFolder + 'amazonLumberyardInterior_b4_l4_s16_c1_WorkGroupData.txt', 'Primary N4L4S16', "PrimaryN4L4S16WorkGroupAnalysis")

makeWorkGroupAnalysis((inputFolder + "WorkGroupTest/MyVersion/"+ "amazonLumberyardInterior_b4_l4_s", "_c0_WorkGroupData.txt"), "Primary N4L4S", 16, ("N4L4S" ,"WorkGroupAnalysisC0_Old"))
makeWorkGroupAnalysis((inputFolder + "WorkGroupTest/NewVersion/"+ "amazonLumberyardInterior_b4_l4_s", "_c0_WorkGroupData.txt"), "Primary N4L4S", 16, ("N4L4S" ,"WorkGroupAnalysisC0_New"))

#makeWorkGroupAnalysis(inputFolder + 'amazonLumberyardInterior_b4_l4_PrimaryWorkGroupWiskerPlot.txt', 'Primary N4L4', "PrimaryN4L4WorkGroupAnalysis")
#makeWorkGroupAnalysis(inputFolder + 'amazonLumberyardInterior_b4_l4_SecondaryWorkGroupWiskerPlot.txt', 'Secondary N4L4', "SecondaryN4L4WorkGroupAnalysis")