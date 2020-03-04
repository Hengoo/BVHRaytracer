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

def nodeLeafCount(splitting):
	if splitting:
		filePath = inputFolder + "averageTable_AllInter.txt"
		title = "BVH node and leaf count"
		outputName = "average"
	else:
		filePath = inputFolder + "averageTable_AllInterNoSplit.txt"
		title = "BVH without splitting node and leaf count"
		outputName = "averageNoSlit"

	#load:
	(branchFactor, leafSize, subdivision, primaryNodeIntersections, primaryLeafIntersections, primaryAabb,
		primaryAabbSuccessRatio, primaryPrimitive, primaryPrimitiveSuccessRatio, secondaryNodeIntersections,
		secondaryLeafIntersections, secondaryAabb, secondaryAabbSuccessRatio, secondaryPrimitive,
		secondaryPrimitiveSuccessRatio, nodeSah, leafSah, nodeEpo, leafEpo, leafVolume, leafSurfaceArea,
		traversalNodeFullness, traversalLeafFullness, BVHNodeFullness, BVHLeafFullness, nodeCount, leafCount,
		averageLeafDepth, treeDepth, primaryWasteFactor, secondaryWasteFactor, primaryNodeCachelines,
		secondaryNodeCachelines, totalTime, nodeTime, leafTime, perAabbCost, perTriCost, sahNodeFactor) = np.loadtxt(filePath, delimiter=',', unpack=True, skiprows=1)
		
	fig = plt.figure(figsize=(12,7))
	plt.subplots_adjust(hspace = 0.35, wspace = 0.15)
	plt.suptitle(title)

	#Bvh node counts:
	ax = plt.subplot(2, 2, 1)
	
	plt.title("Node count depending on node size")
	leafSizes = [1,2,3,4,8,12,16]
	for i in leafSizes:
		filter2 = nodeCount[leafSize == i] #* branchFactor[leafSize == i]
		filter1 = branchFactor[leafSize == i]
		plt.plot(filter1, filter2, label='L' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	ax.set_ylim(ymin= -0.1, ymax = 1.1)
	plt.xlabel('Node size')
	plt.ylabel('Number of nodes')
	plt.legend()

	
	#aabb intersections by leaf size.
	ax = plt.subplot(2, 2, 2)
	nodeSizes = [2,3,4,8,12,16]
	plt.title("Node count depending on leaf size")
	plt.plot([2],[1]) # <- in order to scip first color ;/
	for i in nodeSizes:
		filter2 = nodeCount[branchFactor == i] #* i
		filter1 = leafSize[branchFactor == i]
		plt.plot(filter1, filter2, label='N' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	ax.set_ylim(ymin= -0.1, ymax = 1.1)
	plt.xlabel('Leaf size')
	plt.ylabel('Number of nodes')
	plt.legend()

	# leaf counts:
	ax = plt.subplot(2, 2, 3)
	
	plt.title("Leaf count depending on node size")
	leafSizes = [1,2,3,4,8,12,16]
	for i in leafSizes:
		filter2 = leafCount[leafSize == i] #* branchFactor[leafSize == i]
		filter1 = branchFactor[leafSize == i]
		plt.plot(filter1, filter2, label='L' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	ax.set_ylim(ymin= -0.1, ymax = 1.1)
	plt.xlabel('Node size')
	plt.ylabel('Number of leafs')
	plt.legend(ncol=3)

	
	ax = plt.subplot(2, 2, 4)
	nodeSizes = [2,3,4,8,12,16]
	plt.title("Leaf count depending on leaf size")
	if splitting:
		plt.plot([2],[1]) # <- in order to scip first color ;/
		for i in nodeSizes:
			filter2 = leafCount[branchFactor == i] #* i
			filter1 = leafSize[branchFactor == i]
			plt.plot(filter1, filter2, label='N' + str(i))
	else:
		filter2 = leafCount[branchFactor == 2]
		filter1 = leafSize[branchFactor == 2]
		plt.plot(filter1, filter2, label="All node sizes")
	plt.xticks(np.arange(2, 18, step=2))
	ax.set_ylim(ymin= -0.1, ymax = 1.1)
	plt.xlabel('Leaf size')
	plt.ylabel('Number of leafs')
	plt.legend()


	#save to file
	plt.savefig(outputFolder + outputName + "BVHCountOverview.pdf")
	plt.savefig(outputFolder + outputName + "BVHCountOverview.pgf")
	endPlot()

def bvhVolume():

	filePath = inputFolder + "averageTable_AllInter.txt"
	#filePath = inputFolder + "averageTable_AllInterNoSplit.txt"
	#filePath = inputFolder + "amazonLumberyardExteriorTable_AllInter.txt"
	
	title = "Surface area, volume and EPO of BVH"
	outputName = "average"

	#load:
	(branchFactor, leafSize, subdivision, primaryNodeIntersections, primaryLeafIntersections, primaryAabb,
		primaryAabbSuccessRatio, primaryPrimitive, primaryPrimitiveSuccessRatio, secondaryNodeIntersections,
		secondaryLeafIntersections, secondaryAabb, secondaryAabbSuccessRatio, secondaryPrimitive,
		secondaryPrimitiveSuccessRatio, nodeSah, leafSah, nodeEpo, leafEpo, leafVolume, leafSurfaceArea,
		traversalNodeFullness, traversalLeafFullness, BVHNodeFullness, BVHLeafFullness, nodeCount, leafCount,
		averageLeafDepth, treeDepth, primaryWasteFactor, secondaryWasteFactor, primaryNodeCachelines,
		secondaryNodeCachelines, totalTime, nodeTime, leafTime, perAabbCost, perTriCost, sahNodeFactor) = np.loadtxt(filePath, delimiter=',', unpack=True, skiprows=1)
		
	fig = plt.figure(figsize=(12,7))
	plt.subplots_adjust(hspace = 0.35, wspace = 0.15)
	plt.suptitle(title)

	ax = plt.subplot(2, 2, 1)
	plt.title("Leaf surface area")
	nodeSizes = [4,8,12,16]
	for i in nodeSizes:
		filter2 = leafSurfaceArea[branchFactor == i]
		filter1 = leafSize[branchFactor == i]
		plt.plot(filter1, filter2, label='L' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	#ax.set_ylim(ymin= -0.1)
	plt.xlabel('Leaf size')
	plt.ylabel('Leaf surface area')
	plt.legend()

	ax = plt.subplot(2, 2, 2)
	nodeSizes = [4,8,12,16]
	plt.title("Leaf volume")
	for i in nodeSizes:
		filter2 = leafVolume[branchFactor == i]
		filter1 = leafSize[branchFactor == i]
		plt.plot(filter1, filter2, label='N' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	#ax.set_ylim(ymin= -0.1)
	plt.xlabel('Leaf size')
	plt.ylabel('Leaf Volume')
	plt.legend()

	#leaf epo
	ax = plt.subplot(2, 2, 3)
	nodeSizes = [4,8,12,16]
	plt.title("Leaf EPO")
	for i in nodeSizes:
		filter2 = leafEpo[branchFactor == i]
		filter1 = leafSize[branchFactor == i]
		plt.plot(filter1, filter2, label='N' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	#ax.set_ylim(ymin= -0.1)
	plt.xlabel('Leaf size')
	plt.ylabel('Leaf EPO')
	plt.legend()

	#node epo
	ax = plt.subplot(2, 2, 4)
	plt.title("Node EPO")
	leafSizes = [4,8,12,16]
	for i in leafSizes:
		filter2 = nodeEpo[leafSize == i]
		filter1 = branchFactor[leafSize == i]
		plt.plot(filter1, filter2, label='L' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	#ax.set_ylim(ymin= -0.1)
	plt.xlabel('Node size')
	plt.ylabel('Node EPO')
	plt.legend()

	#save to file
	plt.savefig(outputFolder + outputName + "BVHVolume.pdf")
	plt.savefig(outputFolder + outputName + "BVHVolume.pgf")
	endPlot()

def epoComparison():
	#comparing sponza and gallery epos

	filePath1 = inputFolder + "sponzaTable_AllInter.txt"
	filePath2 = inputFolder + "galleryTable_AllInter.txt"
	
	outputName = "epoComparison"

	#load:
	(branchFactor, leafSize, subdivision, primaryNodeIntersections, primaryLeafIntersections, primaryAabb,
		primaryAabbSuccessRatio, primaryPrimitive, primaryPrimitiveSuccessRatio, secondaryNodeIntersections,
		secondaryLeafIntersections, secondaryAabb, secondaryAabbSuccessRatio, secondaryPrimitive,
		secondaryPrimitiveSuccessRatio, nodeSah, leafSah, nodeEpo, leafEpo, leafVolume, leafSurfaceArea,
		traversalNodeFullness, traversalLeafFullness, BVHNodeFullness, BVHLeafFullness, nodeCount, leafCount,
		averageLeafDepth, treeDepth, primaryWasteFactor, secondaryWasteFactor, primaryNodeCachelines,
		secondaryNodeCachelines, totalTime, nodeTime, leafTime, perAabbCost, perTriCost, sahNodeFactor) = np.loadtxt(filePath1, delimiter=',', unpack=True, skiprows=1)
		
	fig = plt.figure(figsize=(12,3.8))
	plt.subplots_adjust(hspace = 0.4, wspace = 0.15)


	#leaf epo sponza
	ax = plt.subplot(1, 2, 1)
	nodeSizes = [4,8,12,16]
	plt.title("Leaf EPO Sponza")
	for i in nodeSizes:
		filter2 = leafEpo[branchFactor == i]
		filter1 = leafSize[branchFactor == i]
		plt.plot(filter1, filter2, label='N' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	#ax.set_ylim(ymin= -0.1)
	plt.xlabel('Leaf size')
	plt.ylabel('Leaf EPO')
	plt.legend()

	#load:
	(branchFactor, leafSize, subdivision, primaryNodeIntersections, primaryLeafIntersections, primaryAabb,
		primaryAabbSuccessRatio, primaryPrimitive, primaryPrimitiveSuccessRatio, secondaryNodeIntersections,
		secondaryLeafIntersections, secondaryAabb, secondaryAabbSuccessRatio, secondaryPrimitive,
		secondaryPrimitiveSuccessRatio, nodeSah, leafSah, nodeEpo, leafEpo, leafVolume, leafSurfaceArea,
		traversalNodeFullness, traversalLeafFullness, BVHNodeFullness, BVHLeafFullness, nodeCount, leafCount,
		averageLeafDepth, treeDepth, primaryWasteFactor, secondaryWasteFactor, primaryNodeCachelines,
		secondaryNodeCachelines, totalTime, nodeTime, leafTime, perAabbCost, perTriCost, sahNodeFactor) = np.loadtxt(filePath2, delimiter=',', unpack=True, skiprows=1)

	#leaf epo gallery
	ax = plt.subplot(1, 2, 2)
	plt.title("Leaf EPO Gallery")
	for i in nodeSizes:
		filter2 = leafEpo[branchFactor == i]
		filter1 = leafSize[branchFactor == i]
		plt.plot(filter1, filter2, label='N' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	#ax.set_ylim(ymin= -0.1)
	plt.xlabel('Leaf size')
	plt.ylabel('Leaf EPO')
	plt.legend()

	#save to file
	plt.savefig(outputFolder + "epoComparison.pdf")
	plt.savefig(outputFolder + "epoComparison.pgf")
	endPlot()

def fullnessGraph():
	filePath = inputFolder + "averageTable_AllInter.txt"
	filePath2 = inputFolder + "averageTable_AllInterNoSplit.txt"
	#fullnessGraph(inputFolder + "averageTable_AllInter.txt" , "Fullness", "average")
	#fullnessGraph(inputFolder + "averageTable_AllInterNoSplit.txt", "Fullness without split", "averageNoSlit")

	#load:
	(branchFactor, leafSize, subdivision, primaryNodeIntersections, primaryLeafIntersections, primaryAabb,
		primaryAabbSuccessRatio, primaryPrimitive, primaryPrimitiveSuccessRatio, secondaryNodeIntersections,
		secondaryLeafIntersections, secondaryAabb, secondaryAabbSuccessRatio, secondaryPrimitive,
		secondaryPrimitiveSuccessRatio, nodeSah, leafSah, nodeEpo, leafEpo, leafVolume, leafSurfaceArea,
		traversalNodeFullness, traversalLeafFullness, BVHNodeFullness, BVHLeafFullness, nodeCount, leafCount,
		averageLeafDepth, treeDepth, primaryWasteFactor, secondaryWasteFactor, primaryNodeCachelines,
		secondaryNodeCachelines, totalTime, nodeTime, leafTime, perAabbCost, perTriCost, sahNodeFactor) = np.loadtxt(filePath, delimiter=',', unpack=True, skiprows=1)
		
	fig = plt.figure(figsize=(12,7))
	plt.suptitle("Node and leaf fullness for BVH with and without leaf splitting")
	plt.subplots_adjust(hspace = 0.35, wspace = 0.15)
	#Bvh node counts:
	ax = plt.subplot(2, 2, 1)
	
	plt.title("Leaf splitting enabled")
	leafSizes = [1,2,3,4,8,12,16]
	for i in leafSizes:
		filter2 = BVHNodeFullness[leafSize == i]
		filter1 = branchFactor[leafSize == i]
		filter2 /= filter1
		plt.plot(filter1, filter2, label='L' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	ax.set_ylim(ymin= -0.1, ymax = 1.1)
	plt.xlabel('Node size')
	plt.ylabel('Node Fullness')
	plt.legend(ncol=3)
	
	#aabb intersections by leaf size.
	ax = plt.subplot(2, 2, 3)
	plt.plot([2],[1]) # <- in order to scip first color ;/
	nodeSizes = [2,3,4,8,12,16]
	plt.title("Leaf splitting enabled")
	for i in nodeSizes:
		filter2 = BVHLeafFullness[branchFactor == i]
		filter1 = leafSize[branchFactor == i]
		filter2 /= filter1
		plt.plot(filter1, filter2, label='N' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	ax.set_ylim(ymin= -0.1, ymax = 1.1)
	plt.xlabel('Leaf size')
	plt.ylabel('Leaf Fullness')
	plt.legend(ncol=3)

	#load:
	(branchFactor, leafSize, subdivision, primaryNodeIntersections, primaryLeafIntersections, primaryAabb,
		primaryAabbSuccessRatio, primaryPrimitive, primaryPrimitiveSuccessRatio, secondaryNodeIntersections,
		secondaryLeafIntersections, secondaryAabb, secondaryAabbSuccessRatio, secondaryPrimitive,
		secondaryPrimitiveSuccessRatio, nodeSah, leafSah, nodeEpo, leafEpo, leafVolume, leafSurfaceArea,
		traversalNodeFullness, traversalLeafFullness, BVHNodeFullness, BVHLeafFullness, nodeCount, leafCount,
		averageLeafDepth, treeDepth, primaryWasteFactor, secondaryWasteFactor, primaryNodeCachelines,
		secondaryNodeCachelines, totalTime, nodeTime, leafTime, perAabbCost, perTriCost, sahNodeFactor) = np.loadtxt(filePath2, delimiter=',', unpack=True, skiprows=1)

	#Bvh node counts:
	ax = plt.subplot(2, 2, 2)
	
	plt.title("Leaf splitting disabled")
	leafSizes = [1,2,3,4,8,12,16]
	for i in leafSizes:
		filter2 = BVHNodeFullness[leafSize == i]
		filter1 = branchFactor[leafSize == i]
		filter2 /= filter1
		plt.plot(filter1, filter2, label='L' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	ax.set_ylim(ymin= -0.1, ymax = 1.1)
	plt.xlabel('Node size')
	plt.ylabel('Node Fullness')
	plt.legend(ncol=3)
	
	#aabb intersections by leaf size.
	ax = plt.subplot(2, 2, 4)
	plt.title("Leaf splitting disabled")

	filter2 = BVHLeafFullness[branchFactor == 2]
	filter1 = leafSize[branchFactor == 2]
	filter2 /= filter1
	plt.plot(filter1, filter2, label= "All node sizes")

	plt.xticks(np.arange(2, 18, step=2))
	ax.set_ylim(ymin= -0.1, ymax = 1.1)
	plt.xlabel('Leaf size')
	plt.ylabel('Leaf Fullness')
	plt.legend(ncol=3)

	#save to file
	plt.savefig(outputFolder + "BVHFullnessOverview.pdf")
	plt.savefig(outputFolder + "BVHFullnessOverview.pgf")
	endPlot()
	
def bvhOverview():
	filePath = inputFolder + "averageTable_AllInter.txt"
	#filePath = inputFolder + "averageTable_AllInterNoSplit.txt"

	#filePath = inputFolder + "sanMiguelTable_AllInter.txt"
	#filePath = inputFolder + "sponzaTable_AllInter.txt"
	#filePath = inputFolder + "galleryTable_AllInter.txt"

	outputName = "average"

	#load:
	(branchFactor, leafSize, subdivision, primaryNodeIntersections, primaryLeafIntersections, primaryAabb,
		primaryAabbSuccessRatio, primaryPrimitive, primaryPrimitiveSuccessRatio, secondaryNodeIntersections,
		secondaryLeafIntersections, secondaryAabb, secondaryAabbSuccessRatio, secondaryPrimitive,
		secondaryPrimitiveSuccessRatio, nodeSah, leafSah, nodeEpo, leafEpo, leafVolume, leafSurfaceArea,
		traversalNodeFullness, traversalLeafFullness, BVHNodeFullness, BVHLeafFullness, nodeCount, leafCount,
		averageLeafDepth, treeDepth, primaryWasteFactor, secondaryWasteFactor, primaryNodeCachelines,
		secondaryNodeCachelines, totalTime, nodeTime, leafTime, perAabbCost, perTriCost, sahNodeFactor) = np.loadtxt(filePath, delimiter=',', unpack=True, skiprows=1)

	#Surface area
	ax = plt.subplot(1, 2, 1)
	plt.title("Leaf Surface Area")
	for i in range(4,17,4):
		filter2 = leafSurfaceArea[branchFactor == i] 
		filter1 = leafSize[branchFactor == i]
		plt.plot(filter1, filter2, label='N' + str(i))
	plt.xlabel('Leaf size')
	plt.ylabel('Leaf Surface Area')
	plt.legend()

	#Volume
	ax = plt.subplot(1, 2, 2)
	plt.title("Leaf Volume")
	for i in range(4,17,4):
		filter2 = leafVolume[branchFactor == i]
		filter1 = leafSize[branchFactor == i]
		plt.plot(filter1, filter2, label='N' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	plt.xlabel('Leaf size')
	plt.ylabel('Leaf Surface Area')
	plt.legend()
	#save to file
	#plt.savefig(outputFolder + outputName + "PrimaryAabbIntersection.pdf")
	#plt.savefig(outputFolder + outputName + "PrimaryAabbIntersection.pgf")
	endPlot()


	#Epo (i dont think i want to show node epo depending on leafsize and leaf epo depending on node size..)
	ax = plt.subplot(1, 2, 1)
	plt.title("Leaf EPO")
	for i in range(4,17,4):
		filter2 = leafEpo[branchFactor == i] 
		filter1 = leafSize[branchFactor == i]
		plt.plot(filter1, filter2, label='N' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	plt.xlabel('Leaf Size')
	plt.ylabel('leafEpo')
	plt.legend()

	ax = plt.subplot(1, 2, 2)
	plt.title("Node EPO")
	for i in range(4,17,4):
		filter2 = nodeEpo[leafSize == i]
		filter1 = branchFactor[leafSize == i]
		plt.plot(filter1, filter2, label='N' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	plt.xlabel('Node Size')
	plt.ylabel('node epo')
	plt.legend()
	#save to file
	#plt.savefig(outputFolder + outputName + "PrimaryAabbIntersection.pdf")
	#plt.savefig(outputFolder + outputName + "PrimaryAabbIntersection.pgf")
	endPlot()

#nodeLeafCount(True)
#nodeLeafCount(False)

#fullnessGraph()

#bvhOverview()

bvhVolume()
epoComparison()
