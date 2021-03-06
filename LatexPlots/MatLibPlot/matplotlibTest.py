import matplotlib.pyplot as plt
import numpy as np

inputFolder = "../Data/"
outputFolder = "../Plots/"

showImage = True

	#TODO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! need to think about what plots should start at 0

def endPlot():
	if showImage:
		plt.show()
	else:
		plt.close()

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

def makeWorkGroupWiskerPlots(filePath, title, workGroupSize, outputName):
	filePath = filePath[0] + str(workGroupSize) + filePath[1]
	title = title + str(workGroupSize)

	
	#This needs a fix

	#load:
	y, z, a, b, c = np.loadtxt(filePath, delimiter=',', unpack=True, skiprows =1)

	(stepId, avgPrimaryNodeWork, avgPrimaryNodeUnique, avgPrimaryLeafWork, avgPrimaryLeafUnique,
		avgPrimaryRayTermination, primaryNodeWorkMax, primaryNodeWorkMin, primaryLeafWorkMax,
		primaryLeafWorkMin, avgSecondaryNodeWork, avgSecondaryNodeUnique, avgSecondaryLeafWork,
		avgSecondaryLeafUnique, avgSecondaryRayTermination, secondaryNodeWorkMax, secondaryNodeWorkMin,
		secondaryLeafWorkMax, secondaryLeafWorkMin) = np.loadtxt(filePath, delimiter=',', unpack=True, skiprows =1)

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

def makeWorkGroupAnalysis(filePath, workGroupSize, outputName):

	filePath = filePath[0] + str(workGroupSize) + filePath[1]
	outputName = outputName[0] + str(workGroupSize) + outputName[1]

	(stepId, avgPrimaryNodeWork, avgPrimaryNodeUnique, avgPrimaryLeafWork, avgPrimaryLeafUnique,
		avgPrimaryRayTermination, primaryNodeWorkMax, primaryNodeWorkMin, primaryLeafWorkMax,
		primaryLeafWorkMin, avgSecondaryNodeWork, avgSecondaryNodeUnique, avgSecondaryLeafWork,
		avgSecondaryLeafUnique, avgSecondaryRayTermination, secondaryNodeWorkMax, secondaryNodeWorkMin,
		secondaryLeafWorkMax, secondaryLeafWorkMin) = np.loadtxt(filePath, delimiter=',', unpack=True, skiprows =1)

	#find highest step id where secondary data is != 0
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

	#First plot is about how much is done in each step
	plt.figure(figsize=(7,7))
	plt.suptitle("Average Node and Leaf intersections per Step")

	plt.subplot(2,1,1)
	plt.title("Primary Ray")
	plt.axhline(linewidth=1, color='0.5')
	plt.axhline(y = workGroupSize * workGroupSize, linewidth=1, color='0.5')
	plt.plot(stepId, avgPrimaryNodeWork, label = "Node Intersections")
	plt.plot(stepId, avgPrimaryLeafWork, label = "Leaf Intersections")
	plt.plot(stepId, avgPrimaryRayTermination, label = "Finished Rays")
	xlimSave = plt.xlim()
	plt.legend()

	plt.subplot(2,1,2)
	plt.title("Secondary Ray")
	plt.axhline(linewidth=1, color='0.5')
	plt.axhline(y = workGroupSize * workGroupSize, linewidth=1, color='0.5')
	plt.plot(stepId, avgSecondaryNodeWork, label = "Nodes Intersections")
	plt.plot(stepId, avgSecondaryLeafWork, label = "Leafs Intersections")
	plt.plot(stepId, avgSecondaryRayTermination, label = "Finished Rays")
	plt.xlim(xlimSave)
	plt.xlabel("Step Id")
	plt.legend()

	plt.savefig(outputFolder + outputName + "_NodeLeaf.pdf")
	plt.savefig(outputFolder + outputName + "_NodeLeaf.pgf")
	endPlot()

	# second plot is about how many unique nodes and leafs where loaded
	plt.figure(figsize=(15,7))
	plt.suptitle("Unique Nodes and Leafs loaded per Step")
	# first section contains left side with nodes in primary(top) and secondary(below)
	yMax = max(secondaryNodeWorkMax.max(), primaryNodeWorkMax.max())
	newyLim = ( 0 - yMax * 0.05, yMax *1.05)

	plt.subplot(2,2,3)
	plt.title("Unique Nodes loaded per Step (Secondary ray)")
	plt.axhline(linewidth=1, color='0.5')
	plt.plot(stepId, avgSecondaryNodeUnique, color=(0.9,0.5,0.13, 1), label = "Unique Nodes")
	
	plt.fill_between(stepId, secondaryNodeWorkMin, secondaryNodeWorkMax, label = "min max unique Nodes", color=(0.9,0.5,0.13, 0.5), zorder=-1)
	plt.xlim(xlimSave)
	plt.ylim(newyLim)
	plt.legend()

	plt.subplot(2,2,1)
	plt.title("Unique Nodes loaded per Step (Primary Ray)")
	plt.axhline(linewidth=1, color='0.5')
	plt.plot(stepId, avgPrimaryNodeUnique, color=(0.9,0.5,0.13, 1), label = "Unique Nodes")

	plt.fill_between(stepId, primaryNodeWorkMin, primaryNodeWorkMax, label = "min max unique Nodes", color=(0.9,0.5,0.13, 0.5), zorder=-1)
	plt.xlim(xlimSave)
	plt.ylim(newyLim)
	plt.legend()
	
	# second section contains right side with leafs in primary(top) and secondary(below)
	yMax = max(secondaryLeafWorkMax.max(), primaryLeafWorkMax.max())
	newyLim = ( 0 - yMax * 0.05, yMax *1.05)

	plt.subplot(2,2,4)
	plt.title("Unique Leafs loaded per Step (Secondary ray)")
	plt.axhline(linewidth=1, color='0.5')
	plt.plot(stepId, avgSecondaryLeafUnique, color=(0.13,0.5,0.9, 1), label = "Unique Leafs")
	
	plt.fill_between(stepId, secondaryLeafWorkMin, secondaryLeafWorkMax, label = "min max unique Leafs", color=(0.13,0.5,0.9, 0.5), zorder= -1)	
	plt.xlim(xlimSave)
	plt.ylim(newyLim)
	plt.legend()


	plt.subplot(2,2,2)
	plt.title("Unique Leafs loaded per Step (Primary Ray)")
	plt.axhline(linewidth=1, color='0.5')
	plt.plot(stepId, avgPrimaryLeafUnique, color=(0.13,0.5,0.9, 1), label = "Unique Leafs")

	plt.fill_between(stepId, primaryLeafWorkMin, primaryLeafWorkMax, label = "min max unique Leafs", color=(0.13,0.5,0.9, 0.5), zorder=-1)
	plt.xlim(xlimSave)
	plt.ylim(newyLim)
	plt.legend()

	plt.savefig(outputFolder + outputName + "_Unique.pdf")
	plt.savefig(outputFolder + outputName + "_Unique.pgf")
	endPlot()

def makeWorkGroupUniqueAnalysis(filePath, outName, workGroupSize):
	print("workGroup unique analysis")
	workSquare = workGroupSize * workGroupSize
	#additional analysis about unique nodes per workgroup, not per step:
	(loadedPrimaryNodes, loadedPrimaryLeafs, loadedPrimaryNodesMax, loadedPrimaryLeafsMax, loadedPrimaryNodesMin,
		loadedPrimaryLeafsMin, loadedSecondaryNodes, loadedSecondaryLeafs, loadedSecondaryNodesMax, loadedSecondaryLeafsMax,
		loadedSecondaryNodesMin, loadedSecondaryLeafsMin, loadedWidePrimaryNodes, loadedWidePrimaryLeafs, loadedWideSecondaryNodes,
		loadedWideSecondaryLeafs) = np.loadtxt(filePath, delimiter=',', unpack=True, skiprows=1)
	x = np.arange(len(loadedPrimaryNodes))

	#sort everything by ??
	#best look would be if i sort the wide and the single seperately, but thats kidna "wrong"?
	p = (loadedPrimaryNodes).argsort()
	loadedPrimaryNodes = loadedPrimaryNodes[p]
	loadedPrimaryNodesMax = loadedPrimaryNodesMax[p]
	loadedPrimaryNodesMin = loadedPrimaryNodesMin[p]
	#loadedWidePrimaryNodes = loadedWidePrimaryNodes[p]
	npArrayAnalysis(loadedPrimaryNodes, "loadedPrimaryNodes	")
	npArrayAnalysis(loadedWidePrimaryNodes, "loadedWidePrimaryNodes	")

	p = (loadedPrimaryLeafs).argsort()
	loadedPrimaryLeafs = loadedPrimaryLeafs[p]
	loadedPrimaryLeafsMax = loadedPrimaryLeafsMax[p]
	loadedPrimaryLeafsMin = loadedPrimaryLeafsMin[p]
	#loadedWidePrimaryLeafs = loadedWidePrimaryLeafs[p]
	npArrayAnalysis(loadedPrimaryLeafs, "loadedPrimaryLeafs	")
	npArrayAnalysis(loadedWidePrimaryLeafs, "loadedWidePrimaryLeafs	")

	p = (loadedSecondaryNodes).argsort()
	loadedSecondaryNodes = loadedSecondaryNodes[p]
	loadedSecondaryNodesMin = loadedSecondaryNodesMin[p]
	loadedSecondaryNodesMax = loadedSecondaryNodesMax[p]
	#loadedWideSecondaryNodes = loadedWideSecondaryNodes[p]

	npArrayAnalysis(loadedSecondaryNodes, "loadedSecondaryNodes	")
	npArrayAnalysis(loadedWideSecondaryNodes, "loadedWideSecondaryNodes")

	p = (loadedSecondaryLeafs).argsort()
	loadedSecondaryLeafs = loadedSecondaryLeafs[p]
	loadedSecondaryLeafsMax = loadedSecondaryLeafsMax[p]
	loadedSecondaryLeafsMin = loadedSecondaryLeafsMin[p]
	#loadedWideSecondaryLeafs = loadedWideSecondaryLeafs[p]
	npArrayAnalysis(loadedSecondaryLeafs, "loadedSecondaryLeafs	")
	npArrayAnalysis(loadedWideSecondaryLeafs, "loadedWideSecondaryLeafs")

	#sort the wide arrays by themself so we can at least see anything
	loadedWidePrimaryNodes.sort()
	loadedWidePrimaryLeafs.sort()
	loadedWideSecondaryNodes.sort()
	loadedWideSecondaryLeafs.sort()

	plt.figure(figsize=(15, 9))
	plt.suptitle("Loaded nodes and leafs.")
	#title -> workgroup size
	ax0 = plt.subplot(2, 2, 1)
	plt.axhline(linewidth=1, color='0.5')
	plt.fill_between(x, loadedPrimaryNodesMin, loadedPrimaryNodesMax, label="min - max unique Nodes", color=(0.9,0.5,0.13, 0.5), zorder=-1)
	plt.plot(x, loadedPrimaryNodes / workSquare, color=(0.9, 0.5, 0.13, 1), label="unique Nodes")
	plt.plot(x, loadedWidePrimaryNodes / workSquare, label = "unique Nodes in wideRenderer")
	plt.legend(loc='upper left')
	plt.title("Primary ray Nodes")
	plt.ylabel("avg per ray loaded Nodes")
	plt.tick_params(
		axis='x',		# changes apply to the x-axis
		which='both',	# both major and minor ticks are affected
		bottom=False,	# ticks along the bottom edge are off
		top=False,		# ticks along the top edge are off
		labelbottom=False) # labels along the bottom edge are off

	ax1 = plt.subplot(2, 2, 2)
	plt.axhline(linewidth=1, color='0.5')
	plt.fill_between(x, loadedPrimaryLeafsMin, loadedPrimaryLeafsMax, label = "min - max unique Leafs", color=(0.9,0.5,0.13, 0.5), zorder=-1)
	plt.plot(x, loadedPrimaryLeafs / workSquare, color=(0.9, 0.5, 0.13, 1), label="Unique Leafs")
	plt.plot(x, loadedWidePrimaryLeafs / workSquare, label="unique Leafs in wideRenderer")
	plt.legend(loc='upper left')
	plt.title("Primary ray Leafs")
	plt.ylabel("avg per ray loaded Leafs")
	plt.tick_params(
		axis='x',		# changes apply to the x-axis
		which='both',	# both major and minor ticks are affected
		bottom=False,	# ticks along the bottom edge are off
		top=False,		# ticks along the top edge are off
		labelbottom=False) # labels along the bottom edge are off

	ax1 = plt.subplot(2, 2, 3)
	plt.axhline(linewidth=1, color='0.5')
	plt.fill_between(x, loadedSecondaryNodesMin, loadedSecondaryNodesMax, label="min - max unique Nodes", color=(0.9,0.5,0.13, 0.5), zorder=-1)
	plt.plot(x, loadedSecondaryNodes / workSquare, color=(0.9, 0.5, 0.13, 1), label="unique Nodes")
	plt.plot(x, loadedWideSecondaryNodes / workSquare, label = "unique Nodes in wideRenderer")
	plt.legend(loc='upper left')
	plt.title("Secondary ray Nodes")
	plt.ylabel("avg per ray loaded Nodes")
	plt.tick_params(
	axis='x',		# changes apply to the x-axis
		which='both',	# both major and minor ticks are affected
		bottom=False,	# ticks along the bottom edge are off
		top=False,		# ticks along the top edge are off
		labelbottom=False) # labels along the bottom edge are off


	ax1 = plt.subplot(2, 2, 4)
	plt.axhline(linewidth=1, color='0.5')
	plt.fill_between(x, loadedSecondaryLeafsMin, loadedSecondaryLeafsMax, label = "min - max unique Leafs", color=(0.9,0.5,0.13, 0.5), zorder=-1)
	plt.plot(x, loadedSecondaryLeafs / workSquare, color=(0.9, 0.5, 0.13, 1), label="Unique Leafs")
	plt.plot(x, loadedWideSecondaryLeafs / workSquare, label="unique Leafs in wideRenderer")
	plt.legend(loc='upper left')
	plt.title("Secondary ray Leafs")
	plt.ylabel("avg per ray loaded Leafs")
	plt.tick_params(
		axis='x',		# changes apply to the x-axis
		which='both',	# both major and minor ticks are affected
		bottom=False,	# ticks along the bottom edge are off
		top=False,		# ticks along the top edge are off
		labelbottom=False) # labels along the bottom edge are off
	plt.savefig(outputFolder + "UniqueLoadedAnalysis" + outName + "s"+ str(workGroupSize) + ".pdf")
	plt.savefig(outputFolder + "UniqueLoadedAnalysis" + outName + "s"+ str(workGroupSize) + ".pgf")
	endPlot()

def workGroupUniqueLoadedCachelines():
	filePath0 = inputFolder + "WorkGroups/WorkGroupSize_16_Version_1/amazonLumberyardInterior_b4_l4_c0_WorkGroupUniqueWork.txt"
	filePath1 = inputFolder + "WorkGroups/WorkGroupSize_16_Version_1/amazonLumberyardInterior_b4_l4_c0_WorkGroupData.txt"
	workGroupSize = 16
	workSquare = workGroupSize * workGroupSize
	#additional analysis about unique nodes per workgroup, not per step:
	(loadedPrimaryNodes, loadedPrimaryLeafs, loadedPrimaryNodesMax, loadedPrimaryLeafsMax, loadedPrimaryNodesMin,
		loadedPrimaryLeafsMin, loadedSecondaryNodes, loadedSecondaryLeafs, loadedSecondaryNodesMax, loadedSecondaryLeafsMax,
		loadedSecondaryNodesMin, loadedSecondaryLeafsMin, loadedWidePrimaryNodes, loadedWidePrimaryLeafs, loadedWideSecondaryNodes,
		loadedWideSecondaryLeafs) = np.loadtxt(filePath0, delimiter=',', unpack=True, skiprows=1)
	x = np.arange(len(loadedPrimaryNodes))

	#Those values are hardcoded:
	nodeCachelines = 2
	leafCachelines = nodeCachelines + 3

	primaryLoadedCachelines = (loadedPrimaryNodes * nodeCachelines + loadedPrimaryLeafs * leafCachelines).mean() / workSquare
	secondaryLoadedCachelines = (loadedSecondaryNodes * nodeCachelines + loadedSecondaryLeafs * leafCachelines).mean() / workSquare

	print("Single ray traveral, cachelines loaded per ray")
	print("average primary loaded Cachelines " + str(primaryLoadedCachelines))
	print("average secondary loaded Cachelines " + str(secondaryLoadedCachelines))

	maxPrimaryLoadedCachelines = (loadedPrimaryNodesMax * nodeCachelines + loadedPrimaryLeafsMax * leafCachelines).max()
	maxSecondaryLoadedCachelines = (loadedSecondaryNodesMax * nodeCachelines + loadedSecondaryLeafsMax * leafCachelines).max()
	
	print("Max primary loaded Cachelines " + str(maxPrimaryLoadedCachelines))
	print("Max secondary loaded Cachelines " + str(maxSecondaryLoadedCachelines))

	print("Wide ray traveral, cachelines loaded workgroup Per step")

	widePrimaryLoadedCachelines = loadedWidePrimaryNodes * nodeCachelines + loadedWidePrimaryLeafs * leafCachelines
	wideSecondaryLoadedCachelines = loadedWideSecondaryNodes * nodeCachelines + loadedWideSecondaryLeafs * leafCachelines

	#npArrayAnalysis(widePrimaryLoadedCachelines, "widePrimaryLoadedCachelines")
	#npArrayAnalysis(wideSecondaryLoadedCachelines, "secondaryWideLoadedCachelines")
	(stepId, avgPrimaryNodeWork, avgPrimaryNodeUnique, avgPrimaryLeafWork, avgPrimaryLeafUnique,
		avgPrimaryRayTermination, primaryNodeWorkMax, primaryNodeWorkMin, primaryLeafWorkMax,
		primaryLeafWorkMin, avgSecondaryNodeWork, avgSecondaryNodeUnique, avgSecondaryLeafWork,
		avgSecondaryLeafUnique, avgSecondaryRayTermination, secondaryNodeWorkMax,
		secondaryNodeWorkMin, secondaryLeafWorkMax, secondaryLeafWorkMin) = np.loadtxt(filePath1, delimiter=',', unpack=True, skiprows=1)
	
	#plot should show how many cachelines are loaded per step.

	avgPrimaryNodeLoadedCachelines = avgPrimaryNodeUnique * nodeCachelines
	avgPrimaryLeafLoadedCachelines = avgPrimaryLeafUnique * leafCachelines
	avgSecondaryNodeLoadedCachelines = avgSecondaryNodeUnique * nodeCachelines
	avgSecondaryLeafLoadedCachelines = avgSecondaryLeafUnique * leafCachelines

	maxPrimaryNodeLoadedCachelines = primaryNodeWorkMax * nodeCachelines 
	maxPrimaryLeafLoadedCachelines =  primaryLeafWorkMax * leafCachelines
	maxSecondaryNodeLoadedCachelines = secondaryNodeWorkMax * nodeCachelines
	maxSecondaryLeafLoadedCachelines =  secondaryLeafWorkMax * leafCachelines

	maxAvgPrim = max(avgPrimaryNodeLoadedCachelines.max() , avgPrimaryLeafLoadedCachelines.max())
	maxAvgSec = max(avgSecondaryNodeLoadedCachelines.max() , avgSecondaryLeafLoadedCachelines.max())

	maxPrim = max(maxPrimaryNodeLoadedCachelines.max() , maxPrimaryLeafLoadedCachelines.max())
	maxSec = max(maxSecondaryNodeLoadedCachelines.max() , maxSecondaryLeafLoadedCachelines.max())

	print("Wide: Average Max primary loaded Cachelines " + str(maxAvgPrim))
	print("Wide: Average Max secondary loaded Cachelines " + str(maxAvgSec))

	print("Wide: Max primary loaded Cachelines " + str(maxPrim))
	print("Wide: Max secondary loaded Cachelines " + str(maxSec))

	# plot to show how many cachelines are loaded for what.
	# add single traversal per ray average as reference line?

	plt.plot(stepId, avgPrimaryNodeLoadedCachelines, label = "Avg Primary Node loaded cachelines")
	plt.plot(stepId, avgPrimaryLeafLoadedCachelines, label = "Avg Primary Leaf loaded cachelines")
	plt.plot(stepId, avgSecondaryNodeLoadedCachelines, label = "Avg Secondary Node loaded cachelines")
	plt.plot(stepId, avgSecondaryLeafLoadedCachelines, label = "Avg Secondary Leaf loaded cachelines")
	plt.axhline(linewidth=1, color='0.5')

	plt.ylabel("loaded cachelines")
	plt.xlabel("steps")

	plt.axhline(y=primaryLoadedCachelines, linewidth=1, color='0.1')
	plt.axhline(y=secondaryLoadedCachelines, linewidth=1, color='0')

	plt.legend()
	endPlot()

	plt.plot(stepId, maxPrimaryNodeLoadedCachelines, label = "Max Primary Node loaded cachelines")
	plt.plot(stepId, maxPrimaryLeafLoadedCachelines, label = "Max Primary Leaf loaded cachelines")
	plt.plot(stepId, maxSecondaryNodeLoadedCachelines, label = "Max Secondary Node loaded cachelines")
	plt.plot(stepId, maxSecondaryLeafLoadedCachelines, label = "Max Secondary Leaf loaded cachelines")
	plt.axhline(linewidth=1, color='0.5')
	
	plt.ylabel("loaded cachelines")
	plt.xlabel("steps")

	plt.axhline(y=maxPrimaryLoadedCachelines, linewidth=1, color='0.1')
	plt.axhline(y=maxSecondaryLoadedCachelines, linewidth=1, color='0')

	plt.legend()
	endPlot()

def perRayPlot(filePath):
	plt.figure(figsize=(15,7))
	plt.suptitle("N4L4 workGroup 16")

	tmpFilePath = filePath + "WideV0_c0.txt"
	(totalTimeV0, nodeTimeV0, leafTimeV0) = np.loadtxt(tmpFilePath , delimiter=',', unpack=True, skiprows = 1)
	tmpFilePath = filePath + "WideV1_c0.txt"
	(totalTimeV1, nodeTimeV1, leafTimeV1) = np.loadtxt(tmpFilePath , delimiter=',', unpack=True, skiprows = 1)
	x = np.arange(totalTimeV0.size)

	#sort array by total time:
	p = totalTimeV0.argsort()
	totalTimeV0 = totalTimeV0[p]
	nodeTimeV0 = nodeTimeV0[p]
	leafTimeV0 = leafTimeV0[p]

	p = totalTimeV1.argsort()
	totalTimeV1 = totalTimeV1[p]
	nodeTimeV1 = nodeTimeV1[p]
	leafTimeV1 = leafTimeV1[p]

	ax0 = plt.subplot(2,2,1)
	plt.title("V0 camera0")
	plt.axhline(linewidth=1, color='0.5')
	plt.plot(x , totalTimeV0, label = "total Ray Time")
	plt.plot(x , nodeTimeV0, label = "node Time")
	plt.plot(x, leafTimeV0, label="leaf Time")
	plt.ylabel(("Ray time in ms (for each workgroup)"))
	plt.legend()

	ax0 = plt.subplot(2,2,3, sharex = ax0, sharey = ax0)
	plt.title("V1 camera0")
	plt.axhline(linewidth=1, color='0.5')
	plt.plot(x , totalTimeV1, label = "total Ray Time")
	plt.plot(x , nodeTimeV1, label = "node Time")
	plt.plot(x , leafTimeV1, label = "leaf Time")
	plt.ylabel(("Ray time in ms (for each workgroup)"))
	plt.legend()

	npArrayAnalysis(totalTimeV0, "V0C0")
	npArrayAnalysis(totalTimeV1, "V1C0")

	tmpFilePath = filePath + "WideV0_c1.txt"
	(totalTimeV0, nodeTimeV0, leafTimeV0) = np.loadtxt(tmpFilePath , delimiter=',', unpack=True, skiprows = 1)
	tmpFilePath = filePath + "WideV1_c1.txt"
	(totalTimeV1, nodeTimeV1, leafTimeV1) = np.loadtxt(tmpFilePath , delimiter=',', unpack=True, skiprows = 1)

	#sort array by total time:
	p = totalTimeV0.argsort()
	totalTimeV0 = totalTimeV0[p]
	nodeTimeV0 = nodeTimeV0[p]
	leafTimeV0 = leafTimeV0[p]

	p = totalTimeV1.argsort()
	totalTimeV1 = totalTimeV1[p]
	nodeTimeV1 = nodeTimeV1[p]
	leafTimeV1 = leafTimeV1[p]

	ax2 = plt.subplot(2, 2, 2, sharex = ax0, sharey = ax0)
	plt.title("V0 camera1")
	plt.axhline(linewidth=1, color='0.5')
	plt.plot(x , totalTimeV0, label = "total Ray Time")
	plt.plot(x , nodeTimeV0, label = "node Time")
	plt.plot(x , leafTimeV0, label = "leaf Time")
	plt.legend()

	ax3 = plt.subplot(2, 2, 4, sharex = ax0, sharey = ax0)
	plt.title("V1 camera1")
	plt.axhline(linewidth=1, color='0.5')
	plt.plot(x , totalTimeV1, label = "total Ray Time")
	plt.plot(x , nodeTimeV1, label = "node Time")
	plt.plot(x , leafTimeV1, label = "leaf Time")
	plt.legend()

	plt.savefig(outputFolder + 'rayTimingAnalysis.pdf')
	plt.savefig(outputFolder + 'rayTimingAnalysis.pgf')

	endPlot()

def npArrayAnalysis(a, title):
	#some analysis for me: min, max, sd and variance, median, average
	text = ':	mean: {:0.3f}	median: {:1.3f}	min: {:2.3f}	max: {:3.3f}	sd: {:4.3f}	var: {:5.3f}'.format(a.mean(), np.median(a), a.min(),a.max(), a.std(), a.var())
	print(title + text)

def rayTotalAnalysis():
	def rayTotalAnalysisHelperComparison(ax, totalTimeV1, leaf, branch, leafSize, width):
		maskV1 = np.ma.masked_where(leaf != leafSize, totalTimeV1)
		maskBranch = np.ma.masked_where(leaf != leafSize, branch)
		maskV1 = maskV1.compressed()
		maskBranch = maskBranch.compressed()

		plt.xticks(np.arange(4, 20, step=4))
		plt.title("Leafsize " + str(leafSize))
		ax.bar(maskBranch, maskV1, width=width, bottom=1, label="Wide renderer")
		ax.axhline(linewidth=1, y = 1, color='0.5')
		plt.legend()

	def rayTotalAnalysisHelperOverview(ax, totalTime, totalTimeV1, leaf, branch, leafSize, width):
		mask = np.ma.masked_where(leaf != leafSize, totalTime)
		maskV1 = np.ma.masked_where(leaf != leafSize, totalTimeV1)
		maskBranch = np.ma.masked_where(leaf != leafSize, branch)
		mask = mask.compressed()
		maskV1 = maskV1.compressed()
		maskBranch = maskBranch.compressed()

		plt.xticks(np.arange(4, 20, step=4))
		plt.title("Leafsize " + str(leafSize))

		ax.bar(maskBranch - width / 2, maskV1, width=width, label="Wide ray")
		ax.bar(maskBranch + width / 2, mask, width=width, label="Single ray Traversal")
		
		#ax.axhline(linewidth=1, y = 0, color='0.5')
		plt.legend(loc = 'lower left')
	
	#overview of the 3 raytracer version, normal , wideV0 and wideV1 for N,L 4 to 16
	(branch, leaf, totalTime) = np.loadtxt(inputFolder + "rayTotalTime.txt" , delimiter=',', unpack=True, skiprows = 1)
	(branch, leaf, totalTimeV1) = np.loadtxt(inputFolder + "rayTotalTimeV1.txt" , delimiter=',', unpack=True, skiprows = 1)

	width = 1

	#first do not normalized plot
	fig = plt.figure(figsize=(12, 8))
	fig.suptitle("General performance overview")

	ax0 = plt.subplot(2,2,1)
	rayTotalAnalysisHelperOverview(ax0, totalTime, totalTimeV1, leaf, branch, 4, width)
	plt.ylabel("Render time in seconds")

	ax1 = plt.subplot(2,2,2, sharex = ax0, sharey = ax0)
	rayTotalAnalysisHelperOverview(ax1, totalTime, totalTimeV1, leaf, branch, 8, width)

	ax2 = plt.subplot(2,2,3, sharex = ax0, sharey = ax0)
	rayTotalAnalysisHelperOverview(ax2, totalTime, totalTimeV1, leaf, branch, 12, width)
	plt.ylabel("Render time in seconds")
	plt.xlabel("Nodesize")

	ax3 = plt.subplot(2,2,4, sharex = ax0, sharey = ax0)
	rayTotalAnalysisHelperOverview(ax3, totalTime, totalTimeV1, leaf, branch, 16, width)
	plt.xlabel("Nodesize")


	plt.savefig(outputFolder + "PerformanceOverview.pdf")
	plt.savefig(outputFolder + "PerformanceOverview.pgf")

	#Now plot that is normalized by "smallest" (single ray traversal)
	fig = plt.figure(figsize=(12, 8))
	fig.suptitle("Performance comparison of single ray traversal to wide traversal")
	totalTimeV1 = (totalTimeV1 / totalTime) - 1

	ax0 = plt.subplot(2,2,1)
	plt.ylabel("time relative to single ray traversal")
	rayTotalAnalysisHelperComparison(ax0, totalTimeV1, leaf, branch, 4, width)

	ax1 = plt.subplot(2,2,2, sharex = ax0, sharey = ax0)
	rayTotalAnalysisHelperComparison(ax1, totalTimeV1, leaf, branch, 8, width)

	ax2 = plt.subplot(2,2,3, sharex = ax0, sharey = ax0)
	plt.xlabel("Nodesize")
	plt.ylabel("time relative to single ray traversal")
	rayTotalAnalysisHelperComparison(ax2, totalTimeV1, leaf, branch, 12, width)

	ax3 = plt.subplot(2,2,4, sharex = ax0, sharey = ax0)
	plt.xlabel("Nodesize")
	rayTotalAnalysisHelperComparison(ax3, totalTimeV1, leaf, branch, 16, width)

	plt.savefig(outputFolder + "PerformanceComparison.pdf")
	plt.savefig(outputFolder + "PerformanceComparison.pgf")
	endPlot()

def rayTotalAnalysisPadding():
	def helperOverview(padding, branch, totalTime, totalTimeV0, totalTimeV1, nodeSize):
		mask = np.ma.masked_where(branch != nodeSize, totalTime)
		maskV0 = np.ma.masked_where(branch != nodeSize, totalTimeV0)
		maskV1 = np.ma.masked_where(branch != nodeSize, totalTimeV1)
		maskBranch = np.ma.masked_where(branch != nodeSize, branch)
		maskPadding = np.ma.masked_where(branch != nodeSize, padding)
		mask = mask.compressed()
		maskV0 = maskV0.compressed()
		maskV1 = maskV1.compressed()
		maskBranch = maskBranch.compressed()
		maskPadding = maskPadding.compressed()

		plt.xticks((0,1,2,5,10,20))
		plt.title("NodeSize " + str(nodeSize))
		plt.bar(maskPadding - width, maskV0, width = width, label = "Wide ray V0")
		plt.bar(maskPadding, maskV1, width = width, label = "Wide ray V1")
		plt.bar(maskPadding + width, mask, width = width, label = "Singe ray traversal")

		#plt.axhline(linewidth=1, y = 1, color='0.5')
		plt.legend()

	def helperRelativeOverview(padding, branch, totalTimeV0, totalTimeV1, nodeSize):
		maskV0 = np.ma.masked_where(branch != nodeSize, totalTimeV0)
		maskV1 = np.ma.masked_where(branch != nodeSize, totalTimeV1)
		maskBranch = np.ma.masked_where(branch != nodeSize, branch)
		maskPadding = np.ma.masked_where(branch != nodeSize, padding)
		maskV0 = maskV0.compressed()
		maskV1 = maskV1.compressed()
		maskBranch = maskBranch.compressed()
		maskPadding = maskPadding.compressed()

		plt.xticks((0,1,2,5,10,20))
		plt.title("NodeSize " + str(nodeSize))
		plt.bar(maskPadding - width / 2, maskV0, width = width, bottom = 1, label = "V0")
		plt.bar(maskPadding + width / 2, maskV1, width = width, bottom = 1, label = "V1")

		plt.axhline(linewidth=1, y = 1, color='0.5')
		plt.legend()

	(padding, branch, leaf, totalTime) = np.loadtxt(inputFolder + "PaddingResults/PadSummary_rayTotalTime.txt" , delimiter=',', unpack=True, skiprows = 1)
	(padding, branch, leaf, totalTimeV0) = np.loadtxt(inputFolder + "PaddingResults/PadSummary_rayTotalTimeV0.txt" , delimiter=',', unpack=True, skiprows = 1)
	(padding, branch, leaf, totalTimeV1) = np.loadtxt(inputFolder + "PaddingResults/PadSummary_rayTotalTimeV1.txt" , delimiter=',', unpack=True, skiprows = 1)

	width = 0.3
	plt.figure(figsize=(12,8))

	ax0 = plt.subplot(2,2,1)
	plt.suptitle("Performance effect of padding")
	plt.ylabel("Render time in seconds")
	helperOverview(padding, branch, totalTime, totalTimeV0, totalTimeV1, 4)

	ax1 = plt.subplot(2,2,2, sharex = ax0, sharey = ax0)
	helperOverview(padding, branch, totalTime, totalTimeV0, totalTimeV1, 8)

	ax2 = plt.subplot(2,2,3, sharex = ax0, sharey = ax0)
	plt.ylabel("Render time in seconds")
	plt.xlabel("Padding")
	helperOverview(padding, branch, totalTime, totalTimeV0, totalTimeV1, 12)

	ax3 = plt.subplot(2,2,4, sharex = ax0, sharey = ax0)
	plt.xlabel("Padding")
	helperOverview(padding, branch, totalTime, totalTimeV0, totalTimeV1, 16)
	
	plt.savefig(outputFolder + "PerformanceOverviewPadding.pdf")
	plt.savefig(outputFolder + "PerformanceOverviewPadding.pgf")

	#normalize by "smallest"
	totalTimeV0 = (totalTimeV0 / totalTime) - 1
	totalTimeV1 = (totalTimeV1 / totalTime) - 1

	plt.figure(figsize=(12,8))

	ax0 = plt.subplot(2,2,1)
	plt.suptitle("Performance comparison of single ray traversal to wide traversal with different paddings.")
	plt.ylabel("time relative to single ray traversal")
	helperRelativeOverview(padding, branch, totalTimeV0, totalTimeV1, 4)

	ax1 = plt.subplot(2,2,2, sharex = ax0, sharey = ax0)
	helperRelativeOverview(padding, branch, totalTimeV0, totalTimeV1, 8)

	ax2 = plt.subplot(2,2,3, sharex = ax0, sharey = ax0)
	plt.ylabel("time relative to single ray traversal")
	plt.xlabel("Padding")
	helperRelativeOverview(padding, branch, totalTimeV0, totalTimeV1, 12)

	ax3 = plt.subplot(2,2,4, sharex = ax0, sharey = ax0)
	plt.xlabel("Padding")
	helperRelativeOverview(padding, branch, totalTimeV0, totalTimeV1, 16)

	plt.savefig(outputFolder + "PerformanceRelativeOverviewPadding.pdf")
	plt.savefig(outputFolder + "PerformanceRelativeOverviewPadding.pgf")

	endPlot()



#makePerfAnalysis(inputFolder + "amazonLumberyardInterior_4To16Table.txt", "Amazon Lumberyard Interior Sse", "AmazonLumberyardInterior_4To16Perf")
#makeIntersectionAnalysis(inputFolder + "amazonLumberyardInterior_1To16Table.txt" , "Amazon Lumberyard Interior", "AmazonLumberyardInterior_1To16")

#makeIntersectionAnalysis(inputFolder + "amazonLumberyardInterior_1To16Table.txt" , "Amazon Lumberyard Interior", "AmazonLumberyardInterior_1To16")

#makeWorkGroupWiskerPlots((inputFolder + "WorkGroups/WorkGroupSize_" , "_Version_0/amazonLumberyardInterior_b4_l4_c0_WorkGroupData.txt"), "Primary N4L4S", 16, ("N4L4S" ,"WorkGroupAnalysisC0_Old"))

#Analysis about workgroup per step stuff and  -> per step unique loaded nodes is not that usefull since i average over alive rays. not sure what to do else
#makeWorkGroupAnalysis((inputFolder + "WorkGroups/WorkGroupSize_" , "_Version_0/amazonLumberyardInterior_b4_l4_c0_WorkGroupData.txt"), 16, ("N4L4S" ,"WorkGroupAnalysisC0_Old"))
#makeWorkGroupAnalysis((inputFolder + "WorkGroups/WorkGroupSize_" , "_Version_1/amazonLumberyardInterior_b4_l4_c0_WorkGroupData.txt"), 16, ("N4L4S" ,"WorkGroupAnalysisC0_New"))

#general workgroup unique node analysis (comparison to single ray traversal)
#makeWorkGroupUniqueAnalysis(inputFolder + "WorkGroups/WorkGroupSize_16_Version_0/amazonLumberyardInterior_b4_l4_c0_WorkGroupUniqueWork.txt", "c0", 16)
#makeWorkGroupUniqueAnalysis(inputFolder + "WorkGroups/WorkGroupSize_16_Version_0/amazonLumberyardInterior_b4_l4_c1_WorkGroupUniqueWork.txt", "c1", 16)

#workGroupUniqueLoadedCachelines()

#perRayPlot(inputFolder + "amazonLumberyardInteriorRayPerformance")

rayTotalAnalysis()
#rayTotalAnalysisPadding()