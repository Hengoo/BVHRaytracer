import matplotlib.pyplot as plt
import numpy as np

inputFolder = "../Data/"
outputFolder = "../Plots/IntersectionPlots/"

showImage = True

def endPlot():
	if showImage:
		plt.show()
	else:
		plt.close()

def primaryAnalysis():
	#load the workload file and visualize it.
	filePath = inputFolder + "averageTable_AllInter.txt" 
	#load:
	(branchFactor, leafSize, subdivision, primaryNodeIntersections, primaryLeafIntersections, primaryAabb,
		primaryAabbSuccessRatio, primaryPrimitive, primaryPrimitiveSuccessRatio, secondaryNodeIntersections,
		secondaryLeafIntersections, secondaryAabb, secondaryAabbSuccessRatio, secondaryPrimitive,
		secondaryPrimitiveSuccessRatio, nodeSah, leafSah, nodeEpo, leafEpo, leafVolume, leafSurfaceArea,
		traversalNodeFullness, traversalLeafFullness, BVHNodeFullness, BVHLeafFullness, nodeCount, leafCount,
		averageLeafDepth, treeDepth, primaryWasteFactor, secondaryWasteFactor, primaryNodeCachelines,
		secondaryNodeCachelines, totalTime, nodeTime, leafTime, perAabbCost, perTriCost, sahNodeFactor)= np.loadtxt(filePath, delimiter=',', unpack=True, skiprows=1)

	#x = np.arange(branchFactor.size)

	leafSizes = [1,2, 4, 8, 12, 16]
	nodeSizes = [2, 4, 8, 12, 16]

	fig = plt.figure(figsize=(12, 7))
	plt.subplots_adjust(hspace = 0.25, wspace = 0.2)
	
	ax = plt.subplot(2, 2, 1)
	#Node intersections by branching factor.
	for i in leafSizes:
		filter2 = primaryNodeIntersections[leafSize == i]
		filter1 = branchFactor[leafSize == i]
		plt.plot(filter1, filter2, label='L' + str(i))

	ax.set_ylim(ymin= -1)
	plt.xlabel('Node size')
	plt.ylabel('\# Primary Node intersections')
	plt.legend()

	ax = plt.subplot(2, 2, 2)
	#aabb intersections by branching factor.
	for i in leafSizes:
		#filter2 = primaryAabb[leafSize == i]
		filter3 = primaryAabb[leafSize == i]
		filter2 = primaryNodeIntersections[leafSize == i]
		filter2 *= filter1
		plt.plot(filter1, filter2, label='L' + str(i))

	ax.set_ylim(ymin= -5)
	plt.xlabel('Node size')
	plt.ylabel('\# Primary Aabb intersections')
	plt.legend()

	ax = plt.subplot(2, 2, 3)
	plt.plot([2],[1]) # <- in order to scip first color ;/
	#Leaf intersections by Leaf size.
	for i in nodeSizes:
		filter2 = primaryLeafIntersections[branchFactor == i]
		filter1 = leafSize[branchFactor == i]
		plt.plot(filter1, filter2, label='N' + str(i))

	ax.set_ylim(ymin= -1)
	plt.xlabel('Leaf size')
	plt.ylabel('\# Primary Leaf intersections')
	plt.legend()
	
	ax = plt.subplot(2, 2, 4)
	plt.plot([2],[1]) # <- in order to scip first color ;/
	#Leaf intersections by Leaf size.
	for i in nodeSizes:
		#filter2 = primaryPrimitive[branchFactor == i]
		filter2 = primaryLeafIntersections[branchFactor == i]
		filter1 = leafSize[branchFactor == i]
		filter2 *= filter1
		plt.plot(filter1, filter2, label='N' + str(i))

	ax.set_ylim(ymin= -5)
	plt.xlabel('Leaf size')
	plt.ylabel('\# Primary Triangle intersections')
	plt.legend()

	#save to file
	plt.savefig(outputFolder + "IntersectionResults.pdf", bbox_inches='tight')
	plt.savefig(outputFolder + "IntersectionResults.pgf", bbox_inches='tight')
	endPlot()

def secondaryAnalysis():
	#load the workload file and visualize it.
	filePath = inputFolder + "averageTable_AllInter.txt" 
	#load:
	(branchFactor, leafSize, subdivision, primaryNodeIntersections, primaryLeafIntersections, primaryAabb,
		primaryAabbSuccessRatio, primaryPrimitive, primaryPrimitiveSuccessRatio, secondaryNodeIntersections,
		secondaryLeafIntersections, secondaryAabb, secondaryAabbSuccessRatio, secondaryPrimitive,
		secondaryPrimitiveSuccessRatio, nodeSah, leafSah, nodeEpo, leafEpo, leafVolume, leafSurfaceArea,
		traversalNodeFullness, traversalLeafFullness, BVHNodeFullness, BVHLeafFullness, nodeCount, leafCount,
		averageLeafDepth, treeDepth, primaryWasteFactor, secondaryWasteFactor, primaryNodeCachelines,
		secondaryNodeCachelines, totalTime, nodeTime, leafTime, perAabbCost, perTriCost, sahNodeFactor)= np.loadtxt(filePath, delimiter=',', unpack=True, skiprows=1)

	#x = np.arange(branchFactor.size)

	leafSizes = [1,2, 4, 8, 12, 16]
	nodeSizes = [2, 4, 8, 12, 16]

	fig = plt.figure(figsize=(12, 7))
	plt.subplots_adjust(hspace = 0.25, wspace = 0.2)
	
	ax = plt.subplot(2, 2, 1)
	#Node intersections by branching factor.
	for i in leafSizes:
		filter2 = secondaryNodeIntersections[leafSize == i]
		filter1 = branchFactor[leafSize == i]
		plt.plot(filter1, filter2, label='L' + str(i))

	ax.set_ylim(ymin= -1)
	plt.xlabel('Node size')
	plt.ylabel('\# Secondary Node intersections')
	plt.legend()

	ax = plt.subplot(2, 2, 2)
	#aabb intersections by branching factor.
	for i in leafSizes:
		#filter2 = secondaryAabb[leafSize == i]
		filter2 = secondaryNodeIntersections[leafSize == i]
		filter2 *= filter1
		plt.plot(filter1, filter2, label='L' + str(i))

	ax.set_ylim(ymin= -5)
	plt.xlabel('Node size')
	plt.ylabel('\# Secondary Aabb intersections')
	plt.legend()

	ax = plt.subplot(2, 2, 3)
	plt.plot([2],[1]) # <- in order to scip first color ;/
	#Leaf intersections by Leaf size.
	for i in nodeSizes:
		filter2 = secondaryLeafIntersections[branchFactor == i]
		filter1 = leafSize[branchFactor == i]
		plt.plot(filter1, filter2, label='N' + str(i))

	ax.set_ylim(ymin= -1)
	plt.xlabel('Leaf size')
	plt.ylabel('\# Secondary Leaf intersections')
	plt.legend()
	
	ax = plt.subplot(2, 2, 4)
	plt.plot([2],[1]) # <- in order to scip first color ;/
	#Leaf intersections by Leaf size.
	for i in nodeSizes:
		#filter2 = secondaryPrimitive[branchFactor == i]
		filter2 = secondaryLeafIntersections[branchFactor == i]
		filter1 = leafSize[branchFactor == i]
		filter2 *= filter1
		plt.plot(filter1, filter2, label='N' + str(i))

	ax.set_ylim(ymin= -5)
	plt.xlabel('Leaf size')
	plt.ylabel('\# Secondary Triangle intersections')
	plt.legend()

	#save to file
	plt.savefig(outputFolder + "SecondaryIntersectionResults.pdf", bbox_inches='tight')
	plt.savefig(outputFolder + "SecondaryIntersectionResults.pgf", bbox_inches='tight')
	endPlot()

def measuredFullness():
	#comparing sponza and gallery epos

	filePath = inputFolder + "averageTable_AllInter.txt"
	#filePath = inputFolder + "galleryTable_AllInter.txt"


	#load:
	(branchFactor, leafSize, subdivision, primaryNodeIntersections, primaryLeafIntersections, primaryAabb,
		primaryAabbSuccessRatio, primaryPrimitive, primaryPrimitiveSuccessRatio, secondaryNodeIntersections,
		secondaryLeafIntersections, secondaryAabb, secondaryAabbSuccessRatio, secondaryPrimitive,
		secondaryPrimitiveSuccessRatio, nodeSah, leafSah, nodeEpo, leafEpo, leafVolume, leafSurfaceArea,
		traversalNodeFullness, traversalLeafFullness, BVHNodeFullness, BVHLeafFullness, nodeCount, leafCount,
		averageLeafDepth, treeDepth, primaryWasteFactor, secondaryWasteFactor, primaryNodeCachelines,
		secondaryNodeCachelines, totalTime, nodeTime, leafTime, perAabbCost, perTriCost, sahNodeFactor) = np.loadtxt(filePath, delimiter=',', unpack=True, skiprows=1)
		
	fig = plt.figure(figsize=(12,3.8))
	plt.subplots_adjust(hspace = 0.4, wspace = 0.15)
	nodeSizes = [2, 3, 4, 8, 12, 16]
	
	leafSizes = [1,2,3,4,8,12,16]
	#node fullness
	ax = plt.subplot(1, 2, 1)
	#plt.title("Node Fullness")
	
	for i in leafSizes:
		filter2 = traversalNodeFullness[leafSize == i] * 100
		filter1 = branchFactor[leafSize == i]
		filter2 /= filter1
		plt.plot(filter1, filter2, label='L' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	ax.set_ylim(ymin= -5, ymax = 105)
	plt.xlabel('Node size')
	plt.ylabel('Traversal Node Fullness [%]')
	plt.legend(ncol=3)
	
	#leaf fullness
	ax = plt.subplot(1, 2, 2)
	plt.plot([2],[1]) # <- in order to scip first color ;/
	#plt.title("Leaf Fullness")
	for i in nodeSizes:
		filter2 = traversalLeafFullness[branchFactor == i] * 100
		filter1 = leafSize[branchFactor == i]
		filter2 /= filter1
		plt.plot(filter1, filter2, label='N' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	ax.set_ylim(ymin= -5, ymax = 105)
	plt.xlabel('Leaf size')
	plt.ylabel('Traversal Leaf Fullness [%]')
	plt.legend(ncol=3)

	#save to file

	plt.savefig(outputFolder + "measuredFullness.pdf", bbox_inches='tight')
	plt.savefig(outputFolder + "measuredFullness.pgf", bbox_inches='tight')
	endPlot()


#primaryAnalysis()
secondaryAnalysis()

#measuredFullness()
