import matplotlib.pyplot as plt
import numpy as np

inputFolder = "Summary/"
outputFolder = "Summary/"

scenes = [
	"sponza",
	"sanMiguel",
	"amazonLumberyardInterior",
	"amazonLumberyardExterior",
	"gallery",]

volumes = [
	13247007744,
	27700.1796875,
	1128518400,
	399729852416,
	1764.8564453125

]

surfaceAreas = [
	35727148,
	6581.56689453125,
	8314583.5,
	393323840,
	1023.3035888671875
]

def computeAverage(inputNames, outputName, suffix):
	tableSum = np.zeros([39,240])

	for i in range(len(inputNames)):
		filePath = inputFolder + scenes[i] + suffix
		tmp = np.loadtxt(filePath, delimiter=',', unpack=True, skiprows=1)
		
		for i in range(3, 21):
			tmp[i] /= tmp[i][0]
			print(i)

		tmp[25] /= tmp[25][0]
		tmp[26] /= tmp[26][0]
		tableSum += tmp
	tableSum /= len(inputNames)

	tableSum = np.swapaxes(tableSum, 0, 1)
	#read first line
	f = open(filePath, "r")
	line = f.readline()
	line = line[:-1]
	f.close()
	np.savetxt(outputFolder + outputName + suffix, tableSum, delimiter=", ", header = line, comments="")


test = ("branchFactor","leafSize ","subdivision","primaryNodeIntersections","primaryLeafIntersections","primaryAabb","primaryAabbSuccessRatio","primaryPrimitive","primaryPrimitiveSuccessRatio","secondaryNodeIntersections","secondaryLeafIntersections","secondaryAabb","secondaryAabbSuccessRatio","secondaryPrimitive","secondaryPrimitiveSuccessRatio","nodeSah","leafSah","nodeEpo","leafEpo","leafVolume","leafSurfaceArea","traversalNodeFullness","traversalLeafFullness","BVHNodeFullness","BVHLeafFullness","nodeCount","leafCount","averageLeafDepth","treeDepth","primaryWasteFactor","secondaryWasteFactor","primaryNodeCachelines","secondaryNodeCachelines","totalTime","nodeTime","leafTime","perAabbCost","perTriCost","sahNodeFactor")

#computeAverage(scenes, "average", "Table_Normal.txt")
#computeAverage(scenes, "average", "Table_Wide.txt")
computeAverage(scenes, "average", "Table_AllInter.txt")
computeAverage(scenes, "average", "Table_AllInterNoSplit.txt")


#19 = volume
#20 = surface area
#sponza:
#volume 13247007744
#surfaceArea 35727148

#san Miguel
#volume 27700.1796875
#surfaceArea 6581.56689453125

#lumberyard interior
#volume 1128518400
#surfaceArea 8314583.5

#lumberyard exterior
#volume 399729852416
#surfaceArea 393323840

#gallery
#volume 1764.8564453125
#surfaceArea 1023.3035888671875