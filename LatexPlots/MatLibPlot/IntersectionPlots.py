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

	printImprovement = False

	fig = plt.figure(figsize=(12, 7))
	plt.subplots_adjust(hspace = 0.25, wspace = 0.22)
	
	ax = plt.subplot(2, 2, 1)
	#Node intersections by branching factor.
	for i in leafSizes:
		filter2 = primaryNodeIntersections[leafSize == i]
		filter1 = branchFactor[leafSize == i]
		plt.plot(filter1, filter2, label='L' + str(i))

		if printImprovement and i == 4:
			print("-------- node intersection improvement")
			lastValue = filter2[0]
			for n in range(0, 15):
				currentValue = filter2[n]
				tmp1 = "%.2f" % ((currentValue / lastValue - 1) * 100)
				tmp2 = "%.2f" % currentValue
				print(" & N" + str(n + 2) +" & " + tmp2 +  " & " + tmp1 + " \\% \\\\")
				lastValue = currentValue

	ax.set_ylim(ymin= 0)
	plt.xlabel('Node size')
	plt.ylabel('\# Primary Node intersections\n\$\\triangleleft$ less is better')
	plt.legend()

	ax = plt.subplot(2, 2, 2)
	#aabb intersections by branching factor.


	for i in leafSizes:
		#filter2 = primaryAabb[leafSize == i]
		filter3 = primaryAabb[leafSize == i]
		filter2 = primaryNodeIntersections[leafSize == i]
		filter2 *= filter1
		plt.plot(filter1, filter2, label='L' + str(i))
		if printImprovement and i == 4:
			print("-------- aabb improvement")
			lastValue = filter2[0]
			for n in range(0, 15):
				currentValue = filter2[n]
				tmp1 = "%.2f" % ((currentValue / lastValue - 1) * 100)
				tmp2 = "%.2f" % currentValue
				print(" & N" + str(n + 2) +" & " + tmp2 +  " & " + tmp1 + " \\% \\\\")
				lastValue = currentValue

	ax.set_ylim(ymin= 0)
	plt.xlabel('Node size')
	plt.ylabel('\# Primary Aabb intersections\n\$\\triangleleft$ less is better')
	plt.legend()

	ax = plt.subplot(2, 2, 3)
	plt.plot([2],[1]) # <- in order to scip first color ;/
	#Leaf intersections by Leaf size.
	for i in nodeSizes:
		filter2 = primaryLeafIntersections[branchFactor == i]
		filter1 = leafSize[branchFactor == i]

		if printImprovement and i == 4:
			print("-------- leaf intersection improvement")
			lastValue = filter2[0]
			for n in range(0, 16):
				currentValue = filter2[n]
				tmp1 = "%.2f" % ((currentValue / lastValue - 1) * 100)
				tmp2 = "%.2f" % currentValue
				print(" & L" + str(n + 1) +" & " + tmp2 +  " & " + tmp1 + " \\% \\\\")
				lastValue = currentValue
		plt.plot(filter1, filter2, label='N' + str(i))

	ax.set_ylim(ymin= 0)
	plt.xlabel('Leaf size')
	plt.ylabel('\# Primary Leaf intersections\n\$\\triangleleft$ less is better')
	plt.legend()
	
	ax = plt.subplot(2, 2, 4)
	plt.plot([2],[1]) # <- in order to scip first color ;/
	#Leaf intersections by Leaf size.
	for i in nodeSizes:
		#filter2 = primaryPrimitive[branchFactor == i]
		filter2 = primaryLeafIntersections[branchFactor == i]
		filter1 = leafSize[branchFactor == i]
		filter2 *= filter1

		if printImprovement and i == 4:
			print("-------- triangle improvement")
			lastValue = filter2[0]
			for n in range(0, 16):
				currentValue = filter2[n]
				tmp1 = "%.2f" % ((currentValue / lastValue - 1) * 100)
				tmp2 = "%.2f" % currentValue
				print(" & L" + str(n + 1) +" & " + tmp2 +  " & " + tmp1 + " \\% \\\\")
				lastValue = currentValue
		plt.plot(filter1, filter2, label='N' + str(i))

	ax.set_ylim(ymin= 0)
	plt.xlabel('Leaf size')
	plt.ylabel('\# Primary Triangle intersections\n\$\\triangleleft$ less is better')
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
	plt.subplots_adjust(hspace = 0.25, wspace = 0.22)
	
	ax = plt.subplot(2, 2, 1)
	#Node intersections by branching factor.
	for i in leafSizes:
		filter2 = secondaryNodeIntersections[leafSize == i]
		filter1 = branchFactor[leafSize == i]
		plt.plot(filter1, filter2, label='L' + str(i))

	ax.set_ylim(ymin= 0)
	plt.xlabel('Node size')
	plt.ylabel('\# Secondary Node intersections\n\$\\triangleleft$ less is better')
	plt.legend()

	ax = plt.subplot(2, 2, 2)
	#aabb intersections by branching factor.
	for i in leafSizes:
		#filter2 = secondaryAabb[leafSize == i]
		filter2 = secondaryNodeIntersections[leafSize == i]
		filter2 *= filter1
		plt.plot(filter1, filter2, label='L' + str(i))

	ax.set_ylim(ymin= 0)
	plt.xlabel('Node size')
	plt.ylabel('\# Secondary Aabb intersections\n\$\\triangleleft$ less is better')
	plt.legend()

	ax = plt.subplot(2, 2, 3)
	plt.plot([2],[1]) # <- in order to scip first color ;/
	#Leaf intersections by Leaf size.
	for i in nodeSizes:
		filter2 = secondaryLeafIntersections[branchFactor == i]
		filter1 = leafSize[branchFactor == i]
		plt.plot(filter1, filter2, label='N' + str(i))

	ax.set_ylim(ymin= 0)
	plt.xlabel('Leaf size')
	plt.ylabel('\# Secondary Leaf intersections\n\$\\triangleleft$ less is better')
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

	ax.set_ylim(ymin= 0)
	plt.xlabel('Leaf size')
	plt.ylabel('\# Secondary Triangle intersections\n\$\\triangleleft$ less is better')
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
	plt.subplots_adjust(hspace = 0.4, wspace = 0.22)
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
	#ax.set_ylim(ymin= -5, ymax = 105)
	plt.xlabel('Node size')
	plt.ylabel('Traversal Node Fullness [%]\nmore is better $\\triangleright$')
	plt.legend(ncol=3)
	
	#leaf fullness
	ax = plt.subplot(1, 2, 2)
	plt.plot([2],[100]) # <- in order to scip first color ;/
	#plt.title("Leaf Fullness")
	for i in nodeSizes:
		filter2 = traversalLeafFullness[branchFactor == i] * 100
		filter1 = leafSize[branchFactor == i]
		filter2 /= filter1
		plt.plot(filter1, filter2, label='N' + str(i))
	plt.xticks(np.arange(2, 18, step=2))
	#ax.set_ylim(ymin= -5, ymax = 105)
	plt.xlabel('Leaf size')
	plt.ylabel('Traversal Leaf Fullness [%]\nmore is better $\\triangleright$')
	plt.legend(ncol=3)

	#save to file

	plt.savefig(outputFolder + "measuredFullness.pdf", bbox_inches='tight')
	plt.savefig(outputFolder + "measuredFullness.pgf", bbox_inches='tight')
	endPlot()

def intersectionCompPrim():
	#compares all scene results

	fig = plt.figure(figsize=(13, 15))
	plt.subplots_adjust(hspace = 0.5, wspace = 0.20)
	
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
			filter2 = primaryNodeIntersections[leafSize == i]
			filter1 = branchFactor[leafSize == i]
			plt.plot(filter1, filter2, label='L' + str(i))

		ax.set_ylim(ymin= 0)
		plt.xlabel('Node size')
		plt.ylabel('\# Primary Node intersections\n\$\\triangleleft$ less is better')
		plt.legend(ncol=2)

		ax = plt.subplot(5, 2, 2 + iteration * 2)
		plt.title(sceneName)
		plt.plot([2],[1]) # <- in order to scip first color ;/
		#Leaf intersections by Leaf size.
		for i in nodeSizes:
			filter2 = primaryLeafIntersections[branchFactor == i]
			filter1 = leafSize[branchFactor == i]
			plt.plot(filter1, filter2, label='N' + str(i))

		ax.set_ylim(ymin= 0)
		plt.xlabel('Leaf size')
		plt.ylabel('\# Primary Leaf intersections\n\$\\triangleleft$ less is better')
		plt.legend(ncol=2)
		
	plt.savefig(outputFolder + "intersectionComparisonPrim.pdf", bbox_inches='tight')
	plt.savefig(outputFolder + "intersectionComparisonPrim.pgf", bbox_inches='tight')
	endPlot()

def intersectionCompSec():
	#compares all scene results

	fig = plt.figure(figsize=(13, 15))
	plt.subplots_adjust(hspace = 0.5, wspace = 0.20)
	
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
			filter2 = secondaryNodeIntersections[leafSize == i]
			filter1 = branchFactor[leafSize == i]
			plt.plot(filter1, filter2, label='L' + str(i))

		ax.set_ylim(ymin= 0)
		plt.xlabel('Node size')
		plt.ylabel('\# Secondary Node intersections\n\$\\triangleleft$ less is better')
		plt.legend(ncol=2)

		ax = plt.subplot(5, 2, 2 + iteration * 2)
		plt.title(sceneName)
		plt.plot([2],[1]) # <- in order to scip first color ;/
		#Leaf intersections by Leaf size.
		for i in nodeSizes:
			filter2 = secondaryLeafIntersections[branchFactor == i]
			filter1 = leafSize[branchFactor == i]
			plt.plot(filter1, filter2, label='N' + str(i))

		ax.set_ylim(ymin= 0)
		plt.xlabel('Leaf size')
		plt.ylabel('\# Secondary Leaf intersections\n\$\\triangleleft$ less is better')
		plt.legend(ncol=2)
		
	plt.savefig(outputFolder + "intersectionComparisonSec.pdf", bbox_inches='tight')
	plt.savefig(outputFolder + "intersectionComparisonSec.pgf", bbox_inches='tight')
	endPlot()

#primaryAnalysis()
#secondaryAnalysis()
#measuredFullness()

intersectionCompPrim()
intersectionCompSec()