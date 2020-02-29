import os.path
import math
from os import path

allNames =[
	"lizard",
	"shiftHappens",
	"erato",
	"cubes",
	"sponza",
	"daviaRock",
	"rungholt",
	"breakfast",
	"sanMiguel",
	"amazonLumberyardInterior",
	"amazonLumberyardExterior",
	"amazonLumberyardCombinedExterior",
	"gallery",
]


def fixNone(value):
	#this converts None to 0 (only used for output)
	if (value == None):
		return 0
	return value

class sceneContainer:
	def __init__(self):
		self.sceneNameId = 0
		self.sceneName = ""
		self.subdivisions = []

class storageType:
	def __init__(self, subdivision, branch, leaf, listOfVariableCounts):
		self.branch = branch
		self.leaf = leaf
		self.subdivision = subdivision
				#names of variables like node Intersection count
		self.variableValues = [None for _ in range(listOfVariableCounts[0])]

		#names of normalized variables like wasteFactor
		self.normalizedVariableValues = [None for _ in range(listOfVariableCounts[1])]

		#Variables that are multiplied cachline they use
		self.variableNodeCachelinesValues = [None for _ in range(listOfVariableCounts[2])]

		self.totalTime = None
		self.nodeTime = None
		self.leafTime = None

class everything:

	def __init__(self, wide):
		#the folder all the scene folders are in: (leave empty if no folder)
		self.folder = "ResultsStorage/Data/IntersectionResults/"
		#self.folder = "ResultsStorage/Data/AllIntersections/"
		self.outputFolder = "Summary/"

		#names of the sceneFolders
		self.names = [4,8,9,10,12]
		#self.names = [9]
		
		#prefixTo the folderNames like this "_4To16"
		#self.prefix = "_4To16"
		self.prefix = ""
		#Prefix to the output txt (so its sceneNamePrefix.txt)
		self.outputPrefix = ""

		self.workGroupSize = 16
		#Version -1 = not wide, 0 = old version, 1 = new version
		self.wide = wide
		if self.wide:
			self.intermediateFolderName = "WorkGroupSize_" + str(self.workGroupSize) + "_Wide/"
		else:
			self.intermediateFolderName = "WorkGroupSize_" + str(self.workGroupSize) + "_Normal/"
		


		# -1 for all, id otherwise (starting with 0)
		self.singeIdOverride = -1

		#maximum branchingfactor and max leafsite
		self.minBranchingFactor = 4
		self.maxBranchingFactor = 16
		self.minLeafSize = 4
		self.maxLeafSize = 16

		self.branchStep = 4
		self.leafStep = 4

		#number of subdivisions we test:
		self.subdivisionRange = [0, 0]
		self.subdivisionCount = self.subdivisionRange[1] - self.subdivisionRange[0] + 1

		# 0 = avx, sse = 1
		self.gangType = 1
		self.gangName = ["Avx", "Sse"]

		#temprary cost function.
		self.nodeCostFactor = 1
		self.leafCostFactor = 1

		self.cachelineSize = 128

		#names of variables like node Intersection count
		self.variableNames = [
			"primary intersections node:",
			"primary intersections leaf:",
			"primary aabb intersections:",
			"primary aabb success ration:",
			"primary primitive intersections:",
			"primary primitive success ratio:",

			"secondary intersections node:",
			"secondary intersections leaf:",
			"secondary aabb intersections:",
			"secondary aabb success ration:",
			"secondary primitive intersections:",
			"secondary primitive success ratio:",

			"sah of node:",
			"sah of leaf:",
			"end point overlap of node:",
			"end point overlap of leaf:",
			"volume of leafs:",
			"surface area of leafs:",
			"average child fullness:",
			"average leaf fullness:",
			"average bvh node fullness:",
			"average bvh leaf fullness:",
			"number of nodes:",
			"number of leafnodes:",
			"average leaf depth:",
			"tree depth:",



			]
		self.variableOutputNames = [
			"primaryNodeIntersections",
			"primaryLeafIntersections",
			"primaryAabb",
			"primaryAabbSuccessRatio",
			"primaryPrimitive",
			"primaryPrimitiveSuccessRatio",

			"secondaryNodeIntersections",
			"secondaryLeafIntersections",
			"secondaryAabb",
			"secondaryAabbSuccessRatio",
			"secondaryPrimitive",
			"secondaryPrimitiveSuccessRatio",

			"nodeSah",
			"leafSah",
			"nodeEpo",
			"leafEpo",
			"leafVolume",
			"leafSurfaceArea",
			"traversalNodeFullness",
			"traversalLeafFullness",
			"BVHNodeFullness",
			"BVHLeafFullness",
			"nodeCount",
			"leafCount",
			"averageLeafDepth",
			"treeDepth",
			]

		#names of normalized variables like wasteFactor
		self.normalizedVariableNames = [
			"primary waste factor:",
			"secondary waste factor:"
		]
		self.normalizedVariableOutputNames = [
			"primaryWasteFactor",
			"secondaryWasteFactor"
		]

		#Variables that are multiplied cachline they use
		self.variableNodeCachelinesNames = [
			"primary intersections node:",
			"secondary intersections node:"
		]
		self.variableNodeCachelinesOutputNames = [
			"primaryNodeCachelines",
			"secondaryNodeCachelines"
		]

		#fullness would be some special thing becuase its divided by leafsize
		# -> could do variables divided by leafsize and ones divided by branchFactor
		# -> and ones multiplied by it?

		#initialize storage
		self.storage = [None for _ in range(len(self.names))]

		#folder to the performance files. For now its the laptop per files
		self.perfFolder = "ResultsStorage/Data/PerfResult/"

		self.listVariableCount = [len(self.variableNames), len(self.normalizedVariableNames), len(self.variableNodeCachelinesNames)]

		if not os.path.exists(self.outputFolder):
			os.makedirs(self.outputFolder)


	def run(self):
		# now loop over all scenes to do the single scene file (and collect min max)
		# then loop over all and get averages

		firstLine = "branchFactor, leafSize, subdivision"
		for name in self.variableOutputNames:
			firstLine += ", " + name
		for name in self.normalizedVariableOutputNames:
			firstLine += ", " + name
		for name in self.variableNodeCachelinesOutputNames:
			firstLine += ", " + name
		firstLine += ", totalTime, nodeTime, leafTime, perAabbCost, perTriCost, sahNodeFactor"

		for loopId, nameId in enumerate(self.names):
			self.storage[loopId] = sceneContainer()
			self.storage[loopId].sceneName = allNames[nameId]
			self.storage[loopId].sceneNameId = nameId
			self.storage[loopId].subdivisions = [[] for _ in range(self.subdivisionCount)]

		#averageStorage = [[] for _ in range(self.subdivisionCount)]

		for loopId, nameId in enumerate(self.names):
			name = allNames[nameId]
			for s in range(self.subdivisionRange[1] - self.subdivisionRange[0] + 1):
				for b in range(0, self.maxBranchingFactor -(self.minBranchingFactor - 1), self.branchStep):
					for l in range(0, self.maxLeafSize - (self.minLeafSize - 1), self.leafStep):
						branch = b + self.minBranchingFactor
						leaf = l + self.minLeafSize

						storagePerSubdivision = storageType(s, branch, leaf, self.listVariableCount)

						
						if(self.subdivisionRange[1] == 0):
							fileName  = self.folder + name + self.prefix + "/" + name + "_b" + str(branch) + "_l" + str(leaf) + "_Info.txt"
							fileName2 = self.folder + name + self.prefix + "/" + name + "_b" + str(branch) + "_l" + str(leaf) + "_BVHInfo.txt"
							fileName3 = self.perfFolder + name + self.gangName[self.gangType] +"Perf" + self.prefix + "/" + self.intermediateFolderName + name + "_b" + str(branch) + "_l" + str(leaf) + "_mb" + str(branch) + "_ml" + str(leaf) + "_Perf.txt"
						else:
							fileName  = self.folder + name + "Sub" + str(s) + self.prefix + "/" + name + "_b" + str(branch) + "_l" + str(leaf) + "_Info.txt"
							fileName2 = self.folder + name + "Sub" + str(s) + self.prefix + "/" + name + "_b" + str(branch) + "_l" + str(leaf) + "_BVHInfo.txt"
							fileName3 = self.perfFolder + name + self.gangName[self.gangType] +"Perf" + "Sub" + str(s) + self.prefix + "/" + name + "_b" + str(branch) + "_l" + str(leaf) + "_mb" + str(branch) + "_ml" + str(leaf) + "_Perf.txt"

						anyFileExists = False
						if (path.exists(fileName)):
							#open file and read important values
							f = open(fileName, "r")
							if f.mode == 'r':
								self.gatherAll(storagePerSubdivision, f)
								anyFileExists = True

						if (path.exists(fileName2)):
							#open file and read important values
							f = open(fileName2, "r")
							if f.mode == 'r':
								self.gatherAll(storagePerSubdivision, f)
								anyFileExists = True

						if (path.exists(fileName3)):
							#open file and read important values
							f = open(fileName3, "r")
							if f.mode == 'r':
								self.gatherPerf(storagePerSubdivision, f)
								anyFileExists = True
						
						if anyFileExists:
							self.storage[loopId].subdivisions[s].append(storagePerSubdivision)

		#remove all empty fields (due to scenes with different max subdivision)
		for scenes in self.storage:
			for sub in reversed(range(self.subdivisionCount)):
				if (len(scenes.subdivisions[sub]) == 0):
					scenes.subdivisions.pop(sub)
		
		#loop over storage and do output
		for scenes in self.storage:
			#create file if i want one file for all subs
			name = scenes.sceneName

			if self.wide:
				wideString = "_Wide"
			else:
				wideString = "_Normal"
			#output file:
			if(self.subdivisionRange[1] == 0):
				fResult = open(self.outputFolder + name + self.prefix + "Table" + self.outputPrefix + wideString +".txt", "w+")
			else:
				fResult = open(self.outputFolder + name + "Sub" + self.prefix + "Table" + self.outputPrefix + wideString + ".txt", "w+")
			fResult.write(firstLine + "\n")
			for subId, sub in enumerate(scenes.subdivisions):
				for configStorage in sub:
					#empty line for table with space if branching factor changes

					#write results
					line = self.makeLine([configStorage.branch, configStorage.leaf, configStorage.subdivision])
					line += ", " + self.makeLine(configStorage.variableValues)
					line += ", " + self.makeLine(configStorage.normalizedVariableValues)
					line += ", " + self.makeLine(configStorage.variableNodeCachelinesValues)
					sahNodeFactor , nodeCost, leafCost = None, None, None
					if configStorage.totalTime != None and configStorage.variableValues[0] != None:
						#i calculate the cost for one node intersection and for one leaf intersection
						nodeCost = configStorage.nodeTime / (configStorage.variableValues[0] + configStorage.variableValues[2])
						leafCost = configStorage.leafTime / (configStorage.variableValues[1] + configStorage.variableValues[3])
						if(leafCost != 0):
							sahNodeFactor = nodeCost / leafCost

					line += ", " + self.makeLine([fixNone(configStorage.totalTime), fixNone(configStorage.nodeTime), fixNone(configStorage.leafTime), fixNone(nodeCost), fixNone(leafCost), fixNone(sahNodeFactor)])

					fResult.write(line + "\n")
			fResult.close()

		#one for each subdivision and one for each branc / leafsize combination
		#the more im thinking about it, average isnt that usefull
		#averageStorage = [[] for _ in range(self.subdivisionCount)]
		#average
		#sceneCount = len(self.names)
		#if sceneCount > 1:
			#fResult = open("AverageTableWithSpace.txt")
			#fResult2 = open("AverageTable.txt")




	def makeLine(self, array):
		line = "" + str(array[0])
		for element in array[1:]:
			line += ", " + str(element)
		return line

	def gatherAll(self, subStorage, file):
		for vId, keyToMatch in enumerate(self.variableNames):
			anyHit = False
			file.seek(0)
			for line in file:
				hit, value = self.gatherVariable(keyToMatch, line)
				if hit:
					if anyHit:
						print("ERROR: variable was found twice")
					anyHit = True
					subStorage.variableValues[vId] = value

		for vId, keyToMatch in enumerate(self.normalizedVariableNames):
			anyHit = False
			file.seek(0)
			for line in file:
				hit, value = self.gatherVariable(keyToMatch, line)
				if hit:
					if anyHit:
						print("ERROR: normalized variable was found twice")
					anyHit = True
					subStorage.normalizedVariableValues[vId] = value

		for vId, keyToMatch in enumerate(self.variableNodeCachelinesNames):
			anyHit = False
			file.seek(0)
			for line in file:
				hit, value = self.gatherNodeCachelineVariable(keyToMatch, line, subStorage.branch)
				if hit:
					if anyHit:
						print("ERROR: cacheline variable was found twice")
					anyHit = True
					subStorage.variableNodeCachelinesValues[vId] = value

	def gatherPerf(self, subStorage, file):
		anyHit = False
		file.seek(0)
		for line in file:
			hit, value = self.gatherVariable("Raytracer total time:", line)
			if hit:
				if anyHit:
					print("ERROR: total time was found twice")
				anyHit = True
				subStorage.totalTime = value
		anyHit = False
		file.seek(0)
		for line in file:
			hit, value = self.gatherVariable("Time all rays(sum) - triangle(sum):", line)
			if hit:
				if anyHit:
					print("ERROR: node time was found twice")
				anyHit = True
				subStorage.nodeTime = value
		anyHit = False
		file.seek(0)
		for line in file:
			hit, value = self.gatherVariable("Time for triangle intersections (SUM):", line)
			if hit:
				if anyHit:
					print("ERROR: leaf time was found twice")
				anyHit = True
				subStorage.leafTime = value

	def gatherVariable(self, keyToMatch, string):
		if(string.find(keyToMatch) != -1):
			for t in string.split():
				try:
					value = float(t)
					return True, value
					#only take first value (second one might be average)
					break
				except ValueError:
					pass
		return False, 0

	def gatherNodeCachelineVariable(self, keyToMatch, string, branch):
		if(string.find(keyToMatch) != -1):
			for t in string.split():
				try:
					byteNeeded = branch * 32
					factor = byteNeeded / self.cachelineSize
					value = float(t) * math.ceil(factor)
					return True, value
					#only take first value (second one might be average)
					break
				except ValueError:
					pass
		return False, 0

e = everything(wide = True)
e.run()
e = everything(wide = False)
e.run()