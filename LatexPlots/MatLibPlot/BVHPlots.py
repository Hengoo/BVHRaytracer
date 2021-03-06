import matplotlib.pyplot as plt
import matplotlib as mpl
import numpy as np

inputFolder = "../Data/"
outputFolder = "../Plots/BVHPlots/"

showImage = False

def endPlot():
	if showImage:
		plt.show()
	else:
		plt.close()

def nodeLeafCount(splitting):
	if splitting:
		filePath = inputFolder + "averageTable_AllInter.txt"
		outputName = "average"
	else:
		filePath = inputFolder + "averageTable_AllInterNoSplit.txt"
		outputName = "averageNoSlit"

	#load:
	(branchFactor, leafSize, subdivision, primaryNodeIntersections, primaryLeafIntersections, primaryAabb,
		primaryAabbSuccessRatio, primaryPrimitive, primaryPrimitiveSuccessRatio, secondaryNodeIntersections,
		secondaryLeafIntersections, secondaryAabb, secondaryAabbSuccessRatio, secondaryPrimitive,
		secondaryPrimitiveSuccessRatio, nodeSah, leafSah, nodeEpo, leafEpo, leafVolume, leafSurfaceArea,
		traversalNodeFullness, traversalLeafFullness, BVHNodeFullness, BVHLeafFullness, nodeCount, leafCount,
		averageLeafDepth, treeDepth, primaryWasteFactor, secondaryWasteFactor, primaryNodeCachelines,
		secondaryNodeCachelines, totalTime, nodeTime, leafTime, perAabbCost, perTriCost, sahNodeFactor) = np.loadtxt(filePath, delimiter=',', unpack=True, skiprows=1)
		
	fig = plt.figure(figsize=(12,12))
	plt.subplots_adjust(hspace = 0.35, wspace = 0.22)

	#Bvh node counts:
	ax = plt.subplot(3, 2, 1)
	
	#plt.title("Node count depending on node size")
	leafSizes = [1,2,3,4,8,12,16]
	for i in leafSizes:
		filter2 = nodeCount[leafSize == i] #* branchFactor[leafSize == i]
		filter1 = branchFactor[leafSize == i]
		plt.plot(filter1, filter2, label='L' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	ax.set_ylim(ymin= -0.1, ymax = 1.1)
	plt.xlabel('Node size')
	plt.ylabel("Relative \#Nodes\n\$\\triangleleft$ less is better")
	plt.legend()

	
	#aabb intersections by leaf size.
	ax = plt.subplot(3, 2, 2)
	nodeSizes = [2,3,4,8,12,16]
	#plt.title("Node count depending on leaf size")
	plt.plot([2],[1]) # <- in order to scip first color ;/
	for i in nodeSizes:
		filter2 = nodeCount[branchFactor == i] #* i
		filter1 = leafSize[branchFactor == i]
		plt.plot(filter1, filter2, label='N' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	ax.set_ylim(ymin= -0.1, ymax = 1.1)
	plt.xlabel('Leaf size')
	plt.ylabel('Relative \#Nodes\n\$\\triangleleft$ less is better')
	plt.legend()

	# leaf counts:
	ax = plt.subplot(3, 2, 3)
	
	#plt.title("Leaf count depending on node size")
	leafSizes = [1,2,3,4,8,12,16]
	for i in leafSizes:
		filter2 = leafCount[leafSize == i] #* branchFactor[leafSize == i]
		filter1 = branchFactor[leafSize == i]
		plt.plot(filter1, filter2, label='L' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	ax.set_ylim(ymin= -0.1, ymax = 1.1)
	plt.xlabel('Node size')
	plt.ylabel('Relative \#Leaves\n\$\\triangleleft$ less is better')
	plt.legend(ncol=3)

	
	ax = plt.subplot(3, 2, 4)
	nodeSizes = [2,3,4,8,12,16]
	#plt.title("Leaf count depending on leaf size")
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
	plt.ylabel('Relative \#Leaves\n\$\\triangleleft$ less is better')
	plt.legend()

	#node fullness
	ax = plt.subplot(3, 2, 5)
	#plt.title("Node Fullness")
	leafSizes = [1,2,3,4,8,12,16]
	for i in leafSizes:
		filter2 = BVHNodeFullness[leafSize == i] * 100
		filter1 = branchFactor[leafSize == i]
		filter2 /= filter1
		plt.plot(filter1, filter2, label='L' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	ax.set_ylim(ymin= -5, ymax = 105)
	plt.xlabel('Node size')
	plt.ylabel('Node Fullness [%]\nmore is better $\\triangleright$')
	plt.legend(ncol=3)
	
	#leaf fullness
	ax = plt.subplot(3, 2, 6)
	plt.plot([2],[100]) # <- in order to scip first color ;/
	nodeSizes = [2,3,4,8,12,16]
	#plt.title("Leaf Fullness")
	for i in nodeSizes:
		filter2 = BVHLeafFullness[branchFactor == i] * 100
		filter1 = leafSize[branchFactor == i]
		filter2 /= filter1
		plt.plot(filter1, filter2, label='N' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	ax.set_ylim(ymin= -5, ymax = 105)
	plt.xlabel('Leaf size')
	plt.ylabel('Leaf Fullness [%]\nmore is better $\\triangleright$')
	plt.legend(ncol=3)

	#save to file
	plt.savefig(outputFolder + outputName + "BVHCountOverview.pdf", bbox_inches='tight')
	plt.savefig(outputFolder + outputName + "BVHCountOverview.pgf", bbox_inches='tight')
	endPlot()

def bvhVolume():

	filePath = inputFolder + "averageTable_AllInter.txt"
	#filePath = inputFolder + "averageTable_AllInterNoSplit.txt"
	#filePath = inputFolder + "amazonLumberyardExteriorTable_AllInter.txt"

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
	plt.subplots_adjust(hspace = 0.35, wspace = 0.22)

	ax = plt.subplot(2, 2, 1)
	#plt.title("Leaf surface area")
	nodeSizes = [4,8,12,16]
	for i in nodeSizes:
		filter2 = leafSurfaceArea[branchFactor == i]
		filter1 = leafSize[branchFactor == i]
		plt.plot(filter1, filter2, label='L' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	#ax.set_ylim(ymin= -0.1)
	plt.xlabel('Leaf size')
	plt.ylabel('Relative Leaf surface area\n\$\\triangleleft$ less is better')
	plt.legend()

	ax = plt.subplot(2, 2, 2)
	nodeSizes = [4,8,12,16]
	#plt.title("Leaf volume")
	for i in nodeSizes:
		filter2 = leafVolume[branchFactor == i]
		filter1 = leafSize[branchFactor == i]
		plt.plot(filter1, filter2, label='N' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	#ax.set_ylim(ymin= -0.1)
	plt.xlabel('Leaf size')
	plt.ylabel('Relative Leaf Volume\n\$\\triangleleft$ less is better')
	plt.legend()

	#leaf epo
	ax = plt.subplot(2, 2, 3)
	nodeSizes = [4,8,12,16]
	#plt.title("Leaf EPO")
	for i in nodeSizes:
		filter2 = leafEpo[branchFactor == i]
		filter1 = leafSize[branchFactor == i]
		plt.plot(filter1, filter2, label='N' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	#ax.set_ylim(ymin= -0.1)
	plt.xlabel('Leaf size')
	plt.ylabel('Relative Leaf EPO\n\$\\triangleleft$ less is better')
	plt.legend()

	#node epo
	ax = plt.subplot(2, 2, 4)
	#plt.title("Node EPO")
	leafSizes = [4,8,12,16]
	for i in leafSizes:
		filter2 = nodeEpo[leafSize == i]
		filter1 = branchFactor[leafSize == i]
		plt.plot(filter1, filter2, label='L' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	#ax.set_ylim(ymin= -0.1)
	plt.xlabel('Node size')
	plt.ylabel('Relative Node EPO\n\$\\triangleleft$ less is better')
	plt.legend()

	#save to file
	plt.savefig(outputFolder + outputName + "BVHVolume.pdf", bbox_inches='tight')
	plt.savefig(outputFolder + outputName + "BVHVolume.pgf", bbox_inches='tight')
	endPlot()

def leafSurfaceAreaComparison():
	#comparing sponza and gallery epos

	filePath1 = inputFolder + "sponzaTable_AllInter.txt"
	filePath2 = inputFolder + "galleryTable_AllInter.txt"

	#load:
	(branchFactor, leafSize, subdivision, primaryNodeIntersections, primaryLeafIntersections, primaryAabb,
		primaryAabbSuccessRatio, primaryPrimitive, primaryPrimitiveSuccessRatio, secondaryNodeIntersections,
		secondaryLeafIntersections, secondaryAabb, secondaryAabbSuccessRatio, secondaryPrimitive,
		secondaryPrimitiveSuccessRatio, nodeSah, leafSah, nodeEpo, leafEpo, leafVolume, leafSurfaceArea,
		traversalNodeFullness, traversalLeafFullness, BVHNodeFullness, BVHLeafFullness, nodeCount, leafCount,
		averageLeafDepth, treeDepth, primaryWasteFactor, secondaryWasteFactor, primaryNodeCachelines,
		secondaryNodeCachelines, totalTime, nodeTime, leafTime, perAabbCost, perTriCost, sahNodeFactor) = np.loadtxt(filePath1, delimiter=',', unpack=True, skiprows=1)
		
	fig = plt.figure(figsize=(12,3.8))
	plt.subplots_adjust(hspace = 0.4, wspace = 0.22)


	ax = plt.subplot(1, 2, 1)
	nodeSizes = [4,8,12,16]
	plt.title("Sponza")
	for i in nodeSizes:
		#filter2 = leafEpo[branchFactor == i]
		filter2 = leafSurfaceArea[branchFactor == i] / 35727148
		filter1 = leafSize[branchFactor == i]
		plt.plot(filter1, filter2, label='N' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	#ax.set_ylim(ymin= -0.1)
	plt.xlabel('Leaf size')
	plt.ylabel('Leaf surface area\n\$\\triangleleft$ less is better')
	plt.legend()

	#load:
	(branchFactor, leafSize, subdivision, primaryNodeIntersections, primaryLeafIntersections, primaryAabb,
		primaryAabbSuccessRatio, primaryPrimitive, primaryPrimitiveSuccessRatio, secondaryNodeIntersections,
		secondaryLeafIntersections, secondaryAabb, secondaryAabbSuccessRatio, secondaryPrimitive,
		secondaryPrimitiveSuccessRatio, nodeSah, leafSah, nodeEpo, leafEpo, leafVolume, leafSurfaceArea,
		traversalNodeFullness, traversalLeafFullness, BVHNodeFullness, BVHLeafFullness, nodeCount, leafCount,
		averageLeafDepth, treeDepth, primaryWasteFactor, secondaryWasteFactor, primaryNodeCachelines,
		secondaryNodeCachelines, totalTime, nodeTime, leafTime, perAabbCost, perTriCost, sahNodeFactor) = np.loadtxt(filePath2, delimiter=',', unpack=True, skiprows=1)

	ax = plt.subplot(1, 2, 2)
	leafSizes = [4,8,12,16]
	plt.title("Gallery")
	for i in nodeSizes:
		#filter2 = leafEpo[branchFactor == i]
		filter2 = leafSurfaceArea[branchFactor == i] / 1023.30
		filter1 = leafSize[branchFactor == i]
		plt.plot(filter1, filter2, label='N' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	#ax.set_ylim(ymin= -0.1)
	plt.xlabel('Leaf size')
	plt.ylabel('Relative leaf surface area\n\$\\triangleleft$ less is better')
	plt.legend()

	#save to file
	plt.savefig(outputFolder + "SurfaceAreaComparison.pdf", bbox_inches='tight')
	plt.savefig(outputFolder + "SurfaceAreaComparison.pgf", bbox_inches='tight')
	endPlot()

def treeDepth():
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
	plt.subplots_adjust(hspace = 0.4, wspace = 0.22)


	#average leaf depth
	ax = plt.subplot(1, 2, 1)
	nodeSizes = [2,3,4,8,12,16]
	for i in nodeSizes:
		filter2 = averageLeafDepth[branchFactor == i]
		filter1 = leafSize[branchFactor == i]
		plt.plot(filter1, filter2, label='N' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	ax.set_ylim(ymin= -1)
	plt.xlabel('Leaf Size')
	plt.ylabel('Average tree depth\n\$\\triangleleft$ less is better')
	plt.legend()

	#average leaf depth
	ax = plt.subplot(1, 2, 2)
	leafSizes = [1,2,3,4,8,12,16]
	for i in leafSizes:
		filter2 = averageLeafDepth[leafSize == i]
		filter1 = branchFactor[leafSize == i]
		plt.plot(filter1, filter2, label='N' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	ax.set_ylim(ymin= -1)
	plt.xlabel('Node Size')
	plt.ylabel('Average tree depth\n\$\\triangleleft$ less is better')
	plt.legend()

	#save to file

	plt.savefig(outputFolder + "treeDepth.pdf", bbox_inches='tight')
	plt.savefig(outputFolder + "treeDepth.pgf", bbox_inches='tight')
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
	plt.subplots_adjust(hspace = 0.35, wspace = 0.15)
	
	#node fullness
	ax = plt.subplot(2, 2, 1)
	plt.title("Leaf splitting enabled")
	leafSizes = [1,2,3,4,8,12,16]
	for i in leafSizes:
		filter2 = BVHNodeFullness[leafSize == i] * 100
		filter1 = branchFactor[leafSize == i]
		filter2 /= filter1
		plt.plot(filter1, filter2, label='L' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	ax.set_ylim(ymin= -5, ymax = 105)
	plt.xlabel('Node size')
	plt.ylabel('Node Fullness [%]')
	plt.legend(ncol=3)
	
	#leaf fullness
	ax = plt.subplot(2, 2, 3)
	plt.plot([2],[1]) # <- in order to scip first color ;/
	nodeSizes = [2,3,4,8,12,16]
	plt.title("Leaf splitting enabled")
	for i in nodeSizes:
		filter2 = BVHLeafFullness[branchFactor == i] * 100
		filter1 = leafSize[branchFactor == i]
		filter2 /= filter1
		plt.plot(filter1, filter2, label='N' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	ax.set_ylim(ymin= -5, ymax = 105)
	plt.xlabel('Leaf size')
	plt.ylabel('Leaf Fullness [%]')
	plt.legend(ncol=3)

	#load:
	(branchFactor, leafSize, subdivision, primaryNodeIntersections, primaryLeafIntersections, primaryAabb,
		primaryAabbSuccessRatio, primaryPrimitive, primaryPrimitiveSuccessRatio, secondaryNodeIntersections,
		secondaryLeafIntersections, secondaryAabb, secondaryAabbSuccessRatio, secondaryPrimitive,
		secondaryPrimitiveSuccessRatio, nodeSah, leafSah, nodeEpo, leafEpo, leafVolume, leafSurfaceArea,
		traversalNodeFullness, traversalLeafFullness, BVHNodeFullness, BVHLeafFullness, nodeCount, leafCount,
		averageLeafDepth, treeDepth, primaryWasteFactor, secondaryWasteFactor, primaryNodeCachelines,
		secondaryNodeCachelines, totalTime, nodeTime, leafTime, perAabbCost, perTriCost, sahNodeFactor) = np.loadtxt(filePath2, delimiter=',', unpack=True, skiprows=1)

	#node fullness
	ax = plt.subplot(2, 2, 2)
	
	plt.title("Leaf splitting disabled")
	leafSizes = [1,2,3,4,8,12,16]
	for i in leafSizes:
		filter2 = BVHNodeFullness[leafSize == i] * 100
		filter1 = branchFactor[leafSize == i]
		filter2 /= filter1
		plt.plot(filter1, filter2, label='L' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	ax.set_ylim(ymin= -5, ymax = 105)
	plt.xlabel('Node size')
	plt.ylabel('Node Fullness [%]')
	plt.legend(ncol=3)
	
	#just average computation
	fullnessAverage = 0
	for i in range(2, 17):
		filter2 = BVHLeafFullness[branchFactor == i] * 100
		filter1 = leafSize[branchFactor == i]
		filter2 /= filter1
		fullnessAverage += np.average(filter2)
	fullnessAverage /= 15

	#leaf fullness
	ax = plt.subplot(2, 2, 4)
	plt.title("Leaf splitting disabled")

	filter2 = BVHLeafFullness[branchFactor == 2] * 100
	filter1 = leafSize[branchFactor == 2]
	filter2 /= filter1
	plt.plot(filter1, filter2, label= "All node sizes")

	plt.xticks(np.arange(2, 18, step=2))
	ax.set_ylim(ymin= -5, ymax = 105)
	plt.xlabel('Leaf size')
	plt.ylabel('Leaf Fullness [%]')
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

def BVHNodeComparison():
	#compares all scene results

	fig = plt.figure(figsize=(13, 15))
	plt.subplots_adjust(hspace = 0.5, wspace = 0.30)
	
	filePaths = ["sponzaTable_AllInter.txt", "sanMiguelTable_AllInter.txt", "galleryTable_AllInter.txt", "amazonLumberyardInteriorTable_AllInter.txt", "amazonLumberyardExteriorTable_AllInter.txt"]
	sceneNames = ["Sponza", "San Miguel", "Gallery", "Bistro Interior", "Bistro Exterior"]
	for iteration, n in enumerate(filePaths):
		filePath = inputFolder + n
		sceneName = sceneNames[iteration]

		#load:
		(branchFactor, leafSize, subdivision, primaryNodeIntersections, primaryLeafIntersections, primaryAabb,
			primaryAabbSuccessRatio, primaryPrimitive, primaryPrimitiveSuccessRatio, secondaryNodeIntersections,
			secondaryLeafIntersections, secondaryAabb, secondaryAabbSuccessRatio, secondaryPrimitive,
			secondaryPrimitiveSuccessRatio, nodeSah, leafSah, nodeEpo, leafEpo, leafVolume, leafSurfaceArea,
			traversalNodeFullness, traversalLeafFullness, BVHNodeFullness, BVHLeafFullness, nodeCount, leafCount,
			averageLeafDepth, treeDepth, primaryWasteFactor, secondaryWasteFactor, primaryNodeCachelines,
			secondaryNodeCachelines, totalTime, nodeTime, leafTime, perAabbCost, perTriCost, sahNodeFactor) = np.loadtxt(filePath, delimiter=',', unpack=True, skiprows=1)

		leafSizes = [1,2, 4, 8, 12, 16]
		nodeSizes = [2, 4, 8, 12, 16]

		ax = plt.subplot(5, 2, 1 + iteration * 2)
		plt.title(sceneName)
		#Node intersections by branching factor.
		for i in leafSizes:
			filter2 = nodeCount[leafSize == i]
			filter1 = branchFactor[leafSize == i]
			plt.plot(filter1, filter2, label='L' + str(i))

		ax.set_ylim(ymin= 0)
		plt.xlabel('Node size')
		plt.ylabel('\# Nodes\n\$\\triangleleft$ less is better')
		ax.yaxis.set_major_formatter(mpl.ticker.StrMethodFormatter('{x:,.0f}'))
		plt.legend(ncol=2, fontsize = "small")

		ax = plt.subplot(5, 2, 2 + iteration * 2)
		plt.title(sceneName)
		plt.plot([2],[1]) # <- in order to scip first color ;/
		#Leaf intersections by Leaf size.
		for i in nodeSizes:
			filter2 = nodeCount[branchFactor == i]
			filter1 = leafSize[branchFactor == i]
			plt.plot(filter1, filter2, label='N' + str(i))

		ax.set_ylim(ymin= 0)
		plt.xlabel('Leaf size')
		plt.ylabel('\# Nodes\n\$\\triangleleft$ less is better')
		ax.yaxis.set_major_formatter(mpl.ticker.StrMethodFormatter('{x:,.0f}'))
		plt.legend(ncol=2, fontsize = "small")
		
	plt.savefig(outputFolder + "BVHCopmarisonNode.pdf", bbox_inches='tight')
	plt.savefig(outputFolder + "BVHCopmarisonNode.pgf", bbox_inches='tight')
	endPlot()


def BVHLeafComparison():
	#compares all scene results

	fig = plt.figure(figsize=(13, 15))
	plt.subplots_adjust(hspace = 0.5, wspace = 0.30)
	
	filePaths = ["sponzaTable_AllInter.txt", "sanMiguelTable_AllInter.txt", "galleryTable_AllInter.txt", "amazonLumberyardInteriorTable_AllInter.txt", "amazonLumberyardExteriorTable_AllInter.txt"]
	sceneNames = ["Sponza", "San Miguel", "Gallery", "Bistro Interior", "Bistro Exterior"]
	for iteration, n in enumerate(filePaths):
		filePath = inputFolder + n
		sceneName = sceneNames[iteration]

		#load:
		(branchFactor, leafSize, subdivision, primaryNodeIntersections, primaryLeafIntersections, primaryAabb,
			primaryAabbSuccessRatio, primaryPrimitive, primaryPrimitiveSuccessRatio, secondaryNodeIntersections,
			secondaryLeafIntersections, secondaryAabb, secondaryAabbSuccessRatio, secondaryPrimitive,
			secondaryPrimitiveSuccessRatio, nodeSah, leafSah, nodeEpo, leafEpo, leafVolume, leafSurfaceArea,
			traversalNodeFullness, traversalLeafFullness, BVHNodeFullness, BVHLeafFullness, nodeCount, leafCount,
			averageLeafDepth, treeDepth, primaryWasteFactor, secondaryWasteFactor, primaryNodeCachelines,
			secondaryNodeCachelines, totalTime, nodeTime, leafTime, perAabbCost, perTriCost, sahNodeFactor) = np.loadtxt(filePath, delimiter=',', unpack=True, skiprows=1)

		leafSizes = [1,2, 4, 8, 12, 16]
		nodeSizes = [2, 4, 8, 12, 16]

		ax = plt.subplot(5, 2, 1 + iteration * 2)
		plt.title(sceneName)
		#Node intersections by branching factor.
		for i in leafSizes:
			filter2 = leafCount[leafSize == i]
			filter1 = branchFactor[leafSize == i]
			plt.plot(filter1, filter2, label='L' + str(i))

		ax.set_ylim(ymin= 0)
		plt.xlabel('Node size')
		plt.ylabel('\# Leaves\n\$\\triangleleft$ less is better')
		ax.yaxis.set_major_formatter(mpl.ticker.StrMethodFormatter('{x:,.0f}'))
		plt.legend(ncol=2, fontsize = "x-small")

		ax = plt.subplot(5, 2, 2 + iteration * 2)
		plt.title(sceneName)
		plt.plot([2],[1]) # <- in order to scip first color ;/
		#Leaf intersections by Leaf size.
		for i in nodeSizes:
			filter2 = leafCount[branchFactor == i]
			filter1 = leafSize[branchFactor == i]
			plt.plot(filter1, filter2, label='N' + str(i))

		ax.set_ylim(ymin= 0)
		plt.xlabel('Leaf size')
		plt.ylabel('\# Leaves\n\$\\triangleleft$ less is better')
		ax.yaxis.set_major_formatter(mpl.ticker.StrMethodFormatter('{x:,.0f}'))
		plt.legend(ncol=2, fontsize = "x-small")
		
	plt.savefig(outputFolder + "BVHCopmarisonLeaf.pdf", bbox_inches='tight')
	plt.savefig(outputFolder + "BVHCopmarisonLeaf.pgf", bbox_inches='tight')
	endPlot()

#nodeLeafCount(True)
#nodeLeafCount(False)

#treeDepth()

#fullnessGraph()

#bvhOverview()

#bvhVolume()
#leafSurfaceAreaComparison()

BVHNodeComparison()
BVHLeafComparison()