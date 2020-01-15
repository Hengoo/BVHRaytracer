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

def perRayPlot(filePath):
	
	plt.suptitle("N4L4 workGroup 16")
	plt.figure(figsize=(15,7))

	tmpFilePath = filePath + "WideV0_c0.txt"
	(totalTimeV0, nodeTimeV0, leafTimeV0) = np.loadtxt(tmpFilePath , delimiter=',', unpack=True, skiprows = 1)
	tmpFilePath = filePath + "WideV1_c0.txt"
	(totalTimeV1, nodeTimeV1, leafTimeV1) = np.loadtxt(tmpFilePath , delimiter=',', unpack=True, skiprows = 1)
	x = np.arange(totalTimeV0.size)
	#calculate max value of totalTimeV0 and totalTimveV1 so all plots use same scope
	yMax = max(totalTimeV0.max() , totalTimeV1.max())
	newyLim = ( 0 - yMax * 0.05, yMax *1.05)

	#sort array by total time:
	p = totalTimeV0.argsort()
	totalTimeV0 = totalTimeV0[p]
	nodeTimeV0 = nodeTimeV0[p]
	leafTimeV0 = leafTimeV0[p]

	p = totalTimeV1.argsort()
	totalTimeV1 = totalTimeV1[p]
	nodeTimeV1 = nodeTimeV1[p]
	leafTimeV1 = leafTimeV1[p]

	plt.subplot(2,2,1)
	plt.title("V0 camrea0")
	plt.axhline(linewidth=1, color='0.5')
	#plt.plot(x , totalTimeV0, label = "total Ray Time")
	plt.plot(x , nodeTimeV0, label = "node Time")
	plt.plot(x , leafTimeV0, label = "leaf Time")
	plt.ylim(newyLim)
	plt.legend()

	plt.subplot(2,2,3)
	plt.title("V1 camrea0")
	plt.axhline(linewidth=1, color='0.5')
	#plt.plot(x , totalTimeV1, label = "total Ray Time")
	plt.plot(x , nodeTimeV1, label = "node Time")
	plt.plot(x , leafTimeV1, label = "leaf Time")
	plt.ylim(newyLim)
	plt.legend()

	npArrayAnalysis(totalTimeV0, "V0C0")
	npArrayAnalysis(totalTimeV1, "V1C0")

	tmpFilePath = filePath + "WideV0_c1.txt"
	(totalTimeV0, nodeTimeV0, leafTimeV0) = np.loadtxt(tmpFilePath , delimiter=',', unpack=True, skiprows = 1)
	tmpFilePath = filePath + "WideV1_c1.txt"
	(totalTimeV1, nodeTimeV1, leafTimeV1) = np.loadtxt(tmpFilePath , delimiter=',', unpack=True, skiprows = 1)
	#calculate max value of totalTimeV0 and totalTimveV1 so all plots use same scope
	yMax = max(totalTimeV0.max() , totalTimeV1.max())
	newyLim = ( 0 - yMax * 0.05, yMax *1.05)

	#sort array by total time:
	p = totalTimeV0.argsort()
	totalTimeV0 = totalTimeV0[p]
	nodeTimeV0 = nodeTimeV0[p]
	leafTimeV0 = leafTimeV0[p]

	p = totalTimeV1.argsort()
	totalTimeV1 = totalTimeV1[p]
	nodeTimeV1 = nodeTimeV1[p]
	leafTimeV1 = leafTimeV1[p]

	plt.subplot(2,2,2)
	plt.title("V0 camrea1")
	plt.axhline(linewidth=1, color='0.5')
	#plt.plot(x , totalTimeV0, label = "total Ray Time")
	plt.plot(x , nodeTimeV0, label = "node Time")
	plt.plot(x , leafTimeV0, label = "leaf Time")
	plt.ylim(newyLim)
	plt.legend()

	plt.subplot(2,2,4)
	plt.title("V1 camrea1")
	plt.axhline(linewidth=1, color='0.5')
	#plt.plot(x , totalTimeV1, label = "total Ray Time")
	plt.plot(x , nodeTimeV1, label = "node Time")
	plt.plot(x , leafTimeV1, label = "leaf Time")
	plt.ylim(newyLim)
	plt.legend()

	plt.savefig(outputFolder + 'rayTimingAnalysis.pdf')
	plt.savefig(outputFolder + 'rayTimingAnalysis.pgf')

	endPlot()

def npArrayAnalysis(a, title):
	#some analysis for me: min, max, std and variance, median, average
	print(title +":  mean: " + str(a.mean()) + " median: " + str(np.median(a)) + " min: " + str(a.min()) + " max: " + str(a.max()) + " std: " + str(a.std()) + " var: " + str(a.var()))

def rayTotalAnalysis():
	plt.suptitle("General performance overview")
	plt.figure(figsize=(12,8))
	#overview of the 3 raytracer version, normal , wideV0 and wideV1 for N,L 4 to 16
	(branch, leaf, totalTime) = np.loadtxt(inputFolder + "rayTotalTime.txt" , delimiter=',', unpack=True, skiprows = 1)
	(branch, leaf, totalTimeV0) = np.loadtxt(inputFolder + "rayTotalTimeV0.txt" , delimiter=',', unpack=True, skiprows = 1)
	(branch, leaf, totalTimeV1) = np.loadtxt(inputFolder + "rayTotalTimeV1.txt" , delimiter=',', unpack=True, skiprows = 1)

	width = 1

	#normalize by "smallest"
	totalTimeV0 = (totalTimeV0 / totalTime) - 1
	totalTimeV1 = (totalTimeV1 / totalTime) - 1

	timeMin = min(totalTimeV0.min(), totalTimeV1.min())
	timeMax = max(totalTimeV0.max(), totalTimeV1.max())
	newyLim = ( timeMin + 1 + - (-timeMin + timeMax) * 0.05, timeMax + 1 + (-timeMin + timeMax) * 0.05)

	plt.subplot(2,2,1)
	leafSize = 4
	maskV0 = np.ma.masked_where(leaf != leafSize, totalTimeV0)
	maskV1 = np.ma.masked_where(leaf != leafSize, totalTimeV1)
	maskBranch = np.ma.masked_where(leaf != leafSize, branch)
	maskV0 = maskV0.compressed()
	maskV1 = maskV1.compressed()
	maskBranch = maskBranch.compressed()

	plt.xticks(np.arange(4, 20, step=4))
	plt.title("Leafsize " + str(leafSize))
	plt.bar(maskBranch - width / 2, maskV0, width = width, bottom = 1, label = "V0")
	plt.bar(maskBranch + width / 2, maskV1, width = width, bottom = 1, label = "V1")
	plt.ylabel("time relative to non wide renderer")
	plt.axhline(linewidth=1, y = 1, color='0.5')
	plt.ylim(newyLim)
	plt.legend()
	
	plt.subplot(2,2,2)
	leafSize = 8
	maskV0 = np.ma.masked_where(leaf != leafSize, totalTimeV0)
	maskV1 = np.ma.masked_where(leaf != leafSize, totalTimeV1)
	maskBranch = np.ma.masked_where(leaf != leafSize, branch)
	maskV0 = maskV0.compressed()
	maskV1 = maskV1.compressed()
	maskBranch = maskBranch.compressed()

	plt.xticks(np.arange(4, 20, step=4))
	plt.title("Leafsize " + str(leafSize))
	plt.bar(maskBranch - width / 2, maskV0, width = width, bottom = 1, label = "V0")
	plt.bar(maskBranch + width / 2, maskV1, width = width, bottom = 1, label = "V1")
	plt.axhline(linewidth=1, y = 1, color='0.5')
	plt.ylim(newyLim)
	plt.legend()

	plt.subplot(2,2,3)
	leafSize = 12
	maskV0 = np.ma.masked_where(leaf != leafSize, totalTimeV0)
	maskV1 = np.ma.masked_where(leaf != leafSize, totalTimeV1)
	maskBranch = np.ma.masked_where(leaf != leafSize, branch)
	maskV0 = maskV0.compressed()
	maskV1 = maskV1.compressed()
	maskBranch = maskBranch.compressed()

	plt.xticks(np.arange(4, 20, step=4))
	plt.title("Leafsize " + str(leafSize))
	plt.bar(maskBranch - width / 2, maskV0, width = width, bottom = 1, label = "V0")
	plt.bar(maskBranch + width / 2, maskV1, width = width, bottom = 1, label = "V1")
	plt.ylabel("time relative to non wide renderer")
	plt.xlabel("Nodesize")
	plt.axhline(linewidth=1, y = 1, color='0.5')
	plt.ylim(newyLim)
	plt.legend()

	plt.subplot(2,2,4)
	leafSize = 16
	maskV0 = np.ma.masked_where(leaf != leafSize, totalTimeV0)
	maskV1 = np.ma.masked_where(leaf != leafSize, totalTimeV1)
	maskBranch = np.ma.masked_where(leaf != leafSize, branch)
	maskV0 = maskV0.compressed()
	maskV1 = maskV1.compressed()
	maskBranch = maskBranch.compressed()

	plt.xticks(np.arange(4, 20, step=4))
	plt.title("Leafsize " + str(leafSize))
	plt.bar(maskBranch - width / 2, maskV0, width = width, bottom = 1, label = "V0")
	plt.bar(maskBranch + width / 2, maskV1, width = width, bottom = 1, label = "V1")
	plt.xlabel("Nodesize")
	plt.axhline(linewidth=1, y = 1, color='0.5')
	plt.ylim(newyLim)
	plt.legend()

	plt.savefig(outputFolder + "PerformanceOverview.pdf")
	plt.savefig(outputFolder + "PerformanceOverview.pgf")
	endPlot()

def rayTotalAnalysisPadding():
	(padding, branch, leaf, totalTime) = np.loadtxt(inputFolder + "PaddingResults/PadSummary_rayTotalTime.txt" , delimiter=',', unpack=True, skiprows = 1)
	(padding, branch, leaf, totalTimeV0) = np.loadtxt(inputFolder + "PaddingResults/PadSummary_rayTotalTimeV0.txt" , delimiter=',', unpack=True, skiprows = 1)
	(padding, branch, leaf, totalTimeV1) = np.loadtxt(inputFolder + "PaddingResults/PadSummary_rayTotalTimeV1.txt" , delimiter=',', unpack=True, skiprows = 1)

	width = 0.3

	#normalize by "smallest"
	totalTimeV0 = (totalTimeV0 / totalTime) - 1
	totalTimeV1 = (totalTimeV1 / totalTime) - 1

	timeMin = min(totalTimeV0.min(), totalTimeV1.min())
	timeMax = max(totalTimeV0.max(), totalTimeV1.max())
	newyLim = ( timeMin + 1 + - (-timeMin + timeMax) * 0.05, timeMax + 1 + (-timeMin + timeMax) * 0.05)


	plt.figure(figsize=(12,8))

	plt.subplot(2,2,1)
	plt.suptitle("Performance effect of padding")
	branchSize = 4
	maskV0 = np.ma.masked_where(branch != branchSize, totalTimeV0)
	maskV1 = np.ma.masked_where(branch != branchSize, totalTimeV1)
	maskBranch = np.ma.masked_where(branch != branchSize, branch)
	maskPadding = np.ma.masked_where(branch != branchSize, padding)
	maskV0 = maskV0.compressed()
	maskV1 = maskV1.compressed()
	maskBranch = maskBranch.compressed()
	maskPadding = maskPadding.compressed()

	plt.xticks((0,1,2,5,10,20))
	plt.title("NodeSize " + str(branchSize))
	plt.bar(maskPadding - width / 2, maskV0, width = width, bottom = 1, label = "V0")
	plt.bar(maskPadding + width / 2, maskV1, width = width, bottom = 1, label = "V1")
	plt.ylabel("time relative to single ray traversal")
	plt.axhline(linewidth=1, y = 1, color='0.5')
	plt.ylim(newyLim)
	plt.legend()
	
	plt.subplot(2,2,2)
	branchSize = 8
	maskV0 = np.ma.masked_where(branch != branchSize, totalTimeV0)
	maskV1 = np.ma.masked_where(branch != branchSize, totalTimeV1)
	maskBranch = np.ma.masked_where(branch != branchSize, branch)
	maskPadding = np.ma.masked_where(branch != branchSize, padding)
	maskV0 = maskV0.compressed()
	maskV1 = maskV1.compressed()
	maskBranch = maskBranch.compressed()
	maskPadding = maskPadding.compressed()

	plt.xticks((0,1,2,5,10,20))
	plt.title("NodeSize " + str(branchSize))
	plt.bar(maskPadding - width / 2, maskV0, width = width, bottom = 1, label = "V0")
	plt.bar(maskPadding + width / 2, maskV1, width = width, bottom = 1, label = "V1")
	plt.axhline(linewidth=1, y = 1, color='0.5')
	plt.ylim(newyLim)
	plt.legend()

	plt.subplot(2,2,3)
	branchSize = 12
	maskV0 = np.ma.masked_where(branch != branchSize, totalTimeV0)
	maskV1 = np.ma.masked_where(branch != branchSize, totalTimeV1)
	maskBranch = np.ma.masked_where(branch != branchSize, branch)
	maskPadding = np.ma.masked_where(branch != branchSize, padding)
	maskV0 = maskV0.compressed()
	maskV1 = maskV1.compressed()
	maskBranch = maskBranch.compressed()
	maskPadding = maskPadding.compressed()

	plt.xticks((0,1,2,5,10,20))
	plt.title("NodeSize " + str(branchSize))
	plt.bar(maskPadding - width / 2, maskV0, width = width, bottom = 1, label = "V0")
	plt.bar(maskPadding + width / 2, maskV1, width = width, bottom = 1, label = "V1")
	plt.ylabel("time relative to single ray traversal")
	plt.xlabel("Padding")
	plt.axhline(linewidth=1, y = 1, color='0.5')
	plt.ylim(newyLim)
	plt.legend()

	plt.subplot(2,2,4)
	branchSize = 16
	maskV0 = np.ma.masked_where(branch != branchSize, totalTimeV0)
	maskV1 = np.ma.masked_where(branch != branchSize, totalTimeV1)
	maskBranch = np.ma.masked_where(branch != branchSize, branch)
	maskPadding = np.ma.masked_where(branch != branchSize, padding)
	maskV0 = maskV0.compressed()
	maskV1 = maskV1.compressed()
	maskBranch = maskBranch.compressed()
	maskPadding = maskPadding.compressed()

	plt.xticks((0,1,2,5,10,20))
	plt.title("NodeSize " + str(branchSize))
	plt.bar(maskPadding - width / 2, maskV0, width = width, bottom = 1, label = "V0")
	plt.bar(maskPadding + width / 2, maskV1, width = width, bottom = 1, label = "V1")
	plt.xlabel("Padding")
	plt.axhline(linewidth=1, y = 1, color='0.5')
	plt.ylim(newyLim)
	plt.legend()

	plt.savefig(outputFolder + "PerformanceOverviewPadding.pdf")
	plt.savefig(outputFolder + "PerformanceOverviewPadding.pgf")

	endPlot()


#makePerfAnalysis(inputFolder + "amazonLumberyardInterior_4To16Table.txt", "Amazon Lumberyard Interior Sse", "AmazonLumberyardInterior_4To16Perf")
#makeIntersectionAnalysis(inputFolder + "amazonLumberyardInterior_1To16Table.txt" , "Amazon Lumberyard Interior", "AmazonLumberyardInterior_1To16")

#makeIntersectionAnalysis(inputFolder + "amazonLumberyardInterior_1To16Table.txt" , "Amazon Lumberyard Interior", "AmazonLumberyardInterior_1To16")

#makeWorkGroupWiskerPlots((inputFolder + "WorkGroups/WorkGroupSize_" , "_Version_0/amazonLumberyardInterior_b4_l4_c0_WorkGroupData.txt"), "Primary N4L4S", 16, ("N4L4S" ,"WorkGroupAnalysisC0_Old"))

#makeWorkGroupAnalysis((inputFolder + "WorkGroups/WorkGroupSize_" , "_Version_0/amazonLumberyardInterior_b4_l4_c0_WorkGroupData.txt"), 16, ("N4L4S" ,"WorkGroupAnalysisC0_Old"))
#makeWorkGroupAnalysis((inputFolder + "WorkGroups/WorkGroupSize_" , "_Version_1/amazonLumberyardInterior_b4_l4_c0_WorkGroupData.txt"), 16, ("N4L4S" ,"WorkGroupAnalysisC0_New"))

#perRayPlot(inputFolder + "amazonLumberyardInteriorRayPerformance")

#rayTotalAnalysis()
rayTotalAnalysisPadding()