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
	filePath = inputFolder + "averageTable_AllInter.txt"
	#filePath = inputFolder + "sanMiguelTable_AllInter.txt"
	#filePath = inputFolder + "sponzaTable_AllInter.txt"
	#filePath = inputFolder + "galleryTable_AllInter.txt"

	

	#load:
	(branchFactor, leafSize, subdivision, primaryNodeIntersections, primaryLeafIntersections, primaryAabb,
		primaryAabbSuccessRatio, primaryPrimitive, primaryPrimitiveSuccessRatio, secondaryNodeIntersections,
		secondaryLeafIntersections, secondaryAabb, secondaryAabbSuccessRatio, secondaryPrimitive,
		secondaryPrimitiveSuccessRatio, nodeSah, leafSah, nodeEpo, leafEpo, leafVolume, leafSurfaceArea,
		traversalNodeFullness, traversalLeafFullness, BVHNodeFullness, BVHLeafFullness, nodeCount, leafCount,
		averageLeafDepth, treeDepth, primaryWasteFactor, secondaryWasteFactor, primaryNodeCachelines,
		secondaryNodeCachelines, totalTime, nodeTime, leafTime, perAabbCost, perTriCost, sahNodeFactor) = np.loadtxt(filePath, delimiter=',', unpack=True, skiprows=1)
		
	#Bvh node counts:
	ax = plt.subplot(2, 2, 1)
	
	plt.title("Node Sizes1")
	leafSizes = [2,3,4,8,12,16]
	for i in leafSizes:
		filter2 = nodeCount[leafSize == i] #* branchFactor[leafSize == i]
		filter1 = branchFactor[leafSize == i]
		plt.plot(filter1, filter2, label='L' + str(i))

	plt.xlabel('Node size')
	plt.ylabel('Number of nodes')
	plt.legend()

	
	#aabb intersections by leaf size.
	ax = plt.subplot(2, 2, 2)
	nodeSizes = [2,3,4,8,12,16]
	plt.title("Node Sizes2")
	for i in nodeSizes:
		filter2 = nodeCount[branchFactor == i] #* i
		filter1 = leafSize[branchFactor == i]
		plt.plot(filter1, filter2, label='N' + str(i))

	plt.xlabel('Leaf size')
	plt.ylabel('Number of nodes')
	plt.legend()

	# leaf counts:
	ax = plt.subplot(2, 2, 3)
	
	plt.title("Node Sizes1")
	leafSizes = [2,3,4,8,12,16]
	for i in leafSizes:
		filter2 = leafCount[leafSize == i] #* branchFactor[leafSize == i]
		filter1 = branchFactor[leafSize == i]
		plt.plot(filter1, filter2, label='L' + str(i))

	plt.xlabel('Node size')
	plt.ylabel('Number of leafs')
	plt.legend()

	
	#aabb intersections by leaf size.
	ax = plt.subplot(2, 2, 4)
	nodeSizes = [2,3,4,8,12,16]
	plt.title("Node Sizes2")
	for i in nodeSizes:
		filter2 = leafCount[branchFactor == i] #* i
		filter1 = leafSize[branchFactor == i]
		plt.plot(filter1, filter2, label='N' + str(i))

	plt.xlabel('Leaf size')
	plt.ylabel('Number of leafs')
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

	plt.xlabel('Leaf size')
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

	plt.xlabel('Leaf size')
	plt.ylabel('Leaf Surface Area')
	plt.legend()
	#save to file
	#plt.savefig(outputFolder + outputName + "PrimaryAabbIntersection.pdf")
	#plt.savefig(outputFolder + outputName + "PrimaryAabbIntersection.pgf")
	endPlot()



bvhOverview()