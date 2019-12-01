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
	def __init__(self):
		#the folder all the scene folders are in: (leave empty if no folder)
		#self.folder = "SavesSortedEarlyStop/"
		self.folder = ""
		#names of the sceneFolders
		#self.names = ["shiftHappens", "erato", "sponza", "rungholt"]
		self.names = [4]
		#prefixTo the folderNames
		#self.prefix = "Long" #< was for sponza long
		self.prefix = ""
		#Prefix to the output txt (so its sceneNamePrefix.txt)
		self.outputPrefix = "Sorted"

		# -1 for all, id otherwise (starting with 0)
		self.singeIdOverride = -1

		#maximum branchingfactor and max leafsite
		self.minBranchingFactor = 8
		self.maxBranchingFactor = 8
		self.minLeafSize = 8
		self.maxLeafSize = 8

		#number of subdivisions we test:
		self.subdivisionRange = [0, 20]
		self.subdivisionCount = self.subdivisionRange[1] - self.subdivisionRange[0] + 1

		#temprary cost function. needs replacement
		self.nodeCostFactor = 1
		self.leafCostFactor = 1

		self.cachelineSize = 128

		#names of variables like node Intersection count
		self.variableNames = [
			"primary intersections node:",
			"primary intersections leaf:",
			"secondary intersections node:",
			"secondary intersections leaf:",
			"average leaf depth:",
			"primary aabb intersections:",
			"primary primitive intersections:",
			"sah of node:",
			"sah of leaf:",
			"end point overlap of node:",
			"end point overlap of leaf:",
			"volume of leafs:",
			"surface area of leafs:",
			"average child fullness:",
			"primary aabb success ration:",
			"primary primitive success ratio:",
			"secondary aabb success ration:",
			"secondary primitive success ratio:",
			"secondary aabb intersections:",
			"secondary primitive intersections:",
			]
		self.variableOutputNames = [
			"primaryNodeIntersections",
			"primaryLeafIntersections",
			"secondaryNodeIntersections",
			"secondaryLeafIntersections",
			"averageLeafDepth",
			"primaryAabb",
			"primaryPrimitive",
			"nodeSah",
			"leafSah",
			"nodeEpo",
			"leafEpo",
			"leafVolume",
			"leafSurfaceArea",
			"nodeFullness",
			"primaryAabbSuccessRatio",
			"primaryTriangleSuccessRatio",
			"secondaryAabbSuccessRatio",
			"secondaryTriangleSuccessRatio",
			"secondaryAabb",
			"secondaryPrimitive",
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
		if(self.subdivisionRange[1] == 0):
			self.perfFolder = "SavesPerf/Laptop/NodeMemoryAvx/"
		else:
			self.perfFolder = "SavesPerf/Laptop/NodeMemorySubAvx/"


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
		firstLine += ", totalTime, nodeTime, leafTime, perNodeCost, perLeafCost, sahNodeFactor"

		storage = []

		for loopId, nameId in enumerate(self.names):
			self.storage[loopId] = sceneContainer()
			self.storage[loopId].sceneName = allNames[nameId]
			self.storage[loopId].sceneNameId = name
			self.storage[loopId].subdivisions = [[] for _ in range(self.subdivisionCount)]

		for loopId, nameId in enumerate(self.names):
			name = allNames[nameId]
			for s in range(self.subdivisionRange[1] - self.subdivisionRange[0] + 1):
				for b in range(self.maxBranchingFactor -(self.minBranchingFactor - 1)):
					for l in range(self.maxLeafSize - (self.minLeafSize - 1)):
						branch = b + self.minBranchingFactor
						leaf = l + self.minLeafSize

						storagePerSubdivision = storageType(s, branch, leaf, [len(self.variableNames), len(self.normalizedVariableNames), len(self.variableNodeCachelinesNames)])
						
						if(self.subdivisionRange[1] == 0):
							fileName  = self.folder + name + self.prefix + "/" + name + "_b" + str(branch) + "_l" + str(leaf) + "_Info.txt"
							fileName2 = self.folder + name + self.prefix + "/" + name + "_b" + str(branch) + "_l" + str(leaf) + "_BVHInfo.txt"
							fileName3 = self.perfFolder + name + self.prefix + "/" + name + "_b" + str(branch) + "_l" + str(leaf) + "_mb" + str(branch) + "_ml" + str(leaf) + "_Perf.txt"
						else:
							fileName  = self.folder + name + "Sub" + str(s) + self.prefix + "/" + name + "_b" + str(branch) + "_l" + str(leaf) + "_Info.txt"
							fileName2 = self.folder + name + "Sub" + str(s) + self.prefix + "/" + name + "_b" + str(branch) + "_l" + str(leaf) + "_BVHInfo.txt"
							fileName3 = self.perfFolder + name + "Sub" + str(s) + self.prefix + "/" + name + "_b" + str(branch) + "_l" + str(leaf) + "_mb" + str(branch) + "_ml" + str(leaf) + "_Perf.txt"

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

		#loop over storage and do output (and average?)
		for scenes in self.storage:
			#create file if i want one file for all subs
			name = scenes.sceneName

			#output file:
			if(self.subdivisionRange[1] == 0):
				fResult = open(name + self.prefix + "TableWithSpace" + self.outputPrefix + ".txt", "w+")
				fResult2 = open(name + self.prefix + "Table" + self.outputPrefix + ".txt", "w+")
			else:
				fResult = open(name + "Sub" + self.prefix + "TableWithSpace" + self.outputPrefix + ".txt", "w+")
				fResult2 = open(name + "Sub" + self.prefix + "Table" + self.outputPrefix + ".txt", "w+")
			fResult.write(firstLine + "\n")
			fResult2.write(firstLine + "\n")
			lastBranch = -1
			for subId, sub in enumerate(scenes.subdivisions):
				for configStorage in sub:
					#empty line for table with space if branching factor changes
					if lastBranch != configStorage.branch:
						fResult.write("\n")
					lastBranch = configStorage.branch

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
						sahNodeFactor = nodeCost / leafCost
					line += ", " + self.makeLine([configStorage.totalTime, configStorage.nodeTime, configStorage.leafTime, nodeCost, leafCost, sahNodeFactor])

					fResult.write(line + "\n")
					fResult2.write(line + "\n")

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
			hit, value = self.gatherVariable("Time for all rays (SUM):", line)
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

e = everything()
e.run()