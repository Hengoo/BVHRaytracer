import matplotlib.pyplot as plt
import numpy as np

inputFolder = "../Data/"
outputFolder = "../Plots/BVHPlots/"

showImage = True

def endPlot():
	if showImage:
		plt.show()
	else:
		plt.close()

def bvhOverview():
	filePath = inputFolder + "amazonLumberyardInteriorTable_AllIntersections.txt"
	filePath = inputFolder + "averageTable_Wide.txt"
	

	#load:
	(branchFactor, leafSize, subdivision, primaryNodeIntersections, primaryLeafIntersections, primaryAabb,
		primaryAabbSuccessRatio, primaryPrimitive, primaryPrimitiveSuccessRatio, secondaryNodeIntersections,
		secondaryLeafIntersections, secondaryAabb, secondaryAabbSuccessRatio, secondaryPrimitive,
		secondaryPrimitiveSuccessRatio, nodeSah, leafSah, nodeEpo, leafEpo, leafVolume, leafSurfaceArea,
		traversalNodeFullness, traversalLeafFullness, BVHNodeFullness, BVHLeafFullness, nodeCount, leafCount,
		averageLeafDepth, treeDepth, primaryWasteFactor, secondaryWasteFactor, primaryNodeCachelines,
		secondaryNodeCachelines, totalTime, nodeTime, leafTime, perAabbCost, perTriCost, sahNodeFactor) = np.loadtxt(filePath, delimiter=',', unpack=True, skiprows=1)
		
	#Bvh node and leaf count
	plt.title("Node Sizes1")
	filter2 = nodeCount[leafSize == 1] #* branchFactor[leafSize == 1]
	filter1 = branchFactor[leafSize == 1]
	plt.plot(filter1, filter2, label='L1')
	for i in range(4,17,4):
		filter2 = nodeCount[leafSize == i] #* branchFactor[leafSize == i]
		filter1 = branchFactor[leafSize == i]
		plt.plot(filter1, filter2, label='L' + str(i))

	plt.xlabel('Node size')
	plt.ylabel('Number of nodes')
	plt.legend()
	#save to file
	#plt.savefig(outputFolder + outputName + "PrimaryNodeIntersection.pdf")
	#plt.savefig(outputFolder + outputName + "PrimaryNodeIntersection.pgf")
	endPlot()

	
	#aabb intersections by leaf size.
	plt.title("Node Sizes2")
	for i in range(4,17,4):
		filter2 = nodeCount[branchFactor == i] #* i
		filter1 = leafSize[branchFactor == i]
		plt.plot(filter1, filter2, label='N' + str(i))

	plt.xlabel('Leaf size')
	plt.ylabel('Number of nodes')
	plt.legend()
	#save to file
	#plt.savefig(outputFolder + outputName + "PrimaryAabbIntersection.pdf")
	#plt.savefig(outputFolder + outputName + "PrimaryAabbIntersection.pgf")
	endPlot()

	#aabb intersections by leaf size.
	plt.title("Leaf Surface Area")
	for i in range(4,17,4):
		filter2 = leafSurfaceArea[branchFactor == i] #* leafCount[branchFactor == i]
		filter1 = leafSize[branchFactor == i]
		plt.plot(filter1, filter2, label='N' + str(i))

	plt.xlabel('SAH')
	plt.ylabel('Leaf Surface Area')
	plt.legend()
	#save to file
	#plt.savefig(outputFolder + outputName + "PrimaryAabbIntersection.pdf")
	#plt.savefig(outputFolder + outputName + "PrimaryAabbIntersection.pgf")
	endPlot()

		#aabb intersections by leaf size.
	plt.title("Leaf Volume")
	for i in range(4,17,4):
		filter2 = leafVolume[branchFactor == i] #* leafCount[branchFactor == i]
		filter1 = leafSize[branchFactor == i]
		plt.plot(filter1, filter2, label='N' + str(i))

	plt.xlabel('SAH')
	plt.ylabel('Leaf Volume')
	plt.legend()
	#save to file
	#plt.savefig(outputFolder + outputName + "PrimaryAabbIntersection.pdf")
	#plt.savefig(outputFolder + outputName + "PrimaryAabbIntersection.pgf")
	endPlot()



bvhOverview()