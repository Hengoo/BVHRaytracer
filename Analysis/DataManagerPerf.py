import os.path
import math
from os import path

class sceneContainer:
	def __init__(self):
		self.sceneName = ""
		self.subdivisions = []

class storageType:
	def __init__(self):
		self.branch = 0
		self.leaf = 0
		self.branchMemory = 0
		self.leafMemory = 0
		self.name = ""
		self.subdivision = 0
		self.triangleCount = 0
		self.averageBvhDepth = 0
		self.totalTime = 0
		self.timeRaySum = 0
		self.timeTriangleSum = 0
		self.timeNodeSum = 0


class everything:
	def __init__(self):
		#maximum branchingfactor and max leafsite
		self.minBranchingFactorList = [[8,2],[4,2]]
		self.maxBranchingFactorList = [[8,64],[4,64]]
		self.minLeafSizeList = [[1,8],[1,4]]
		self.maxLeafSizeList = [[64, 8], [64, 4]]
		
		#number of subdivisions we test:
		self.subdivisionRange = [0, 20]
		self.subdivisionCount = self.subdivisionRange[1] - self.subdivisionRange[0] + 1

		# 0 = leaf , 1 = node (need to adjust when table change!) (i separate those since i dont want to do a combined performance test since it gets messy quite fast)
		self.workType = 1
		self.workName = ["Leaf", "Node"]
		# 0 = avx, sse = 1
		self.gangType = 0
		self.gangName = ["Avx", "Sse"]

		#the folder all the scene folders are in: (leave empty if no folder)
		#self.folder = "SavesPerf/Laptop/NodeMemoryAvx/"
		if(self.subdivisionRange[1] == 0):
			self.folder = "SavesPerf/Laptop/" +self.workName[self.workType] + "Memory" + self.gangName[self.gangType] +"/"
		else:
			self.folder = "SavesPerf/Laptop/" +self.workName[self.workType] + "MemorySub" + self.gangName[self.gangType] +"/"

		#real outputFolder is outputFolderName + names + outputPrefix
		self.outputFolderName = "SavesPerf/Laptop/Summary/"

		#names of the sceneFolders
		#self.names = ["shiftHappens", "erato", "sponza", "rungholt"]
		#self.names = ["breakfast","sanMiguel", "gallery", "amazonLumberyardCombinedExterior", "amazonLumberyardInterior","amazonLumberyardExterior"]
		self.names = ["sponza"]

		#prefixTo the folderNames
		#self.prefix = "SSESeqMemoryLeaf"
		self.prefix = ""
		#Prefix to the output txt (so its sceneNamePrefix.txt)
		self.outputPrefix = self.workName[self.workType] + "Memory" + self.gangName[self.gangType]
		
		self.minBranchingFactor = self.minBranchingFactorList[self.gangType][self.workType]
		self.maxBranchingFactor = self.maxBranchingFactorList[self.gangType][self.workType]
		self.minLeafSize = self.minLeafSizeList[self.gangType][self.workType]
		self.maxLeafSize = self.maxLeafSizeList[self.gangType][self.workType]

		self.storage = [None for i in range(len(self.names))]

		self.possibleMemorySizes = [4, 8, 12, 16, 20, 24, 28, 32, 40, 48, 56, 64]

	def readDataFromFiles(self):
		# now loop over all scenes to do the single scene file (and collect min max)
		# then loop over all and get averages


		for nameId, name in enumerate(self.names):
			self.storage[nameId] = sceneContainer()
			self.storage[nameId].sceneName = name
			self.storage[nameId].subdivisions = [[] for _ in range(self.subdivisionCount)]
		
		firstLine = self.getFirstLine()
		
		#first loop over different possible memory sizes for l and b (and ml and mb)
		for mb in self.possibleMemorySizes:
			for ml in self.possibleMemorySizes:
				#now do what normal data manger does, loop over b and l and write files
				for i in range(len(self.names)):
					for s in range(self.subdivisionCount):
						#anyFound = False

						for b in range(self.maxBranchingFactor -(self.minBranchingFactor - 1)):
							for l in range(self.maxLeafSize - (self.minLeafSize - 1)):
								branch = b + self.minBranchingFactor
								leaf = l + self.minLeafSize

								#test all variables of this file
								if(self.subdivisionRange[1] == 0):
									fileName = self.folder + self.names[i] + self.prefix + "/" + self.names[i] + "_b" + str(branch) + "_l" + str(leaf) + "_mb" + str(mb) + "_ml" + str(ml) + "_Perf.txt"
								else:
									fileName = self.folder + self.names[i] +"Sub"+ str(s) + self.prefix + "/" + self.names[i] + "_b" + str(branch) + "_l" + str(leaf) + "_mb" + str(mb) + "_ml" + str(ml) + "_Perf.txt"
								if (path.exists(fileName)):

									#open file and read important values
									f = open(fileName, "r")
									if f.mode == 'r':
										storagePart = self.fillStorage(f, self.names[i], branch, leaf, mb, ml, s)
										self.storage[i].subdivisions[s].append(storagePart)


		#remove all empty fields (due to scenes with different max subdivision)
		for s in self.storage:
			for i in reversed(range(self.subdivisionCount)):
				if (len(s.subdivisions[i]) == 0):
					s.subdivisions.pop(i)
		abc = 0

	def printEverything(self):
		#prints all files:
		firstLine = "branchMemory, leafMemory, triangleCount, averageBvhDepth, raytracerTotalTime, rayTimeSum, triangleIntersectionSum, rayTimeSumWithoutTri"

		for scene in self.storage:
			for sub in scene.subdivisions:
				for b in range(self.maxBranchingFactor -(self.minBranchingFactor - 1)):
					for l in range(self.maxLeafSize - (self.minLeafSize - 1)):
						branch = b + self.minBranchingFactor
						leaf = l + self.minLeafSize
						foundAny = False
						for obj in sub:
							if (obj.leaf != leaf) or (obj.branch != branch):
								continue
							if not foundAny:
								configText = "b" + str(obj.branch) + "l" + str(obj.leaf)
								if(self.subdivisionRange[1] == 0):
									fileName = self.outputFolderName + obj.name + "/" + obj.name + self.prefix + self.outputPrefix + configText + "Table.txt"
									if not os.path.exists(self.outputFolderName + obj.name):
										os.makedirs(self.outputFolderName + obj.name)
								else:
									fileName = self.outputFolderName + obj.name + "Sub" + str(obj.subdivision) + "/" + obj.name + self.prefix + self.outputPrefix + configText + "Table.txt"
									if not os.path.exists(self.outputFolderName + obj.name+ "Sub" + str(obj.subdivision)):
										os.makedirs(self.outputFolderName + obj.name + "Sub" + str(obj.subdivision))
								fResult = open(fileName, "w+")
								fResult.write(firstLine + "\n")
								foundAny = True

							line = self.makeLine([obj.branchMemory, obj.leafMemory, obj.triangleCount, obj.averageBvhDepth, obj.totalTime, obj.timeRaySum, obj.timeTriangleSum, obj.timeNodeSum])
							fResult.write(line + "\n")
						if (foundAny):
							fResult.close()
	
	def printEverythingOneFile(self):
		#idea: everything of storage array in one file
		later = 0
		firstLine = "name, subdivision, branch, branchMemory, leaf, leafMemory, triangleCount, averageBvhDepth, raytracerTotalTime, rayTimeSum, triangleIntersectionSum, rayTimeSumWithoutTri"

		fileName = self.outputFolderName + "total" + self.workName[self.workType] + self.gangName[self.gangType]+ "PerfTable.txt"
		fResult = open(fileName, "w+")
		fResult.write(firstLine + "\n")

		for scene in self.storage:
			for sub in scene.subdivisions:
				for obj in sub:
					line = self.makeLine([obj.name, obj.subdivision, obj.branch, obj.branchMemory, obj.leaf, obj.leafMemory, obj.triangleCount, obj.averageBvhDepth, obj.totalTime, obj.timeRaySum, obj.timeTriangleSum, obj.timeNodeSum])
					fResult.write(line + "\n")

	def makeLine(self, array):
		line = "" + str(array[0])
		array.pop(0)
		for element in array:
			line += ", " + str(element)
		return line

	def gatherValue(self, string, key):
		result = 0
		foundAnything = False

		if(string.find(key) != -1):
			for t in string.split():
				try:
					result = float(t)
					foundAnything = True
					break
				except ValueError:
					pass

		return foundAnything, result

	def fillStorage(self, file, name, branch, leaf, branchMemory, leafMemory, subdivision):
		storage = storageType()
		storage.name = name
		storage.branch = branch
		storage.leaf = leaf
		storage.subdivision = subdivision
		storage.branchMemory = branchMemory
		storage.leafMemory = leafMemory
		
		for x in file:
			#check our variables. Since the performance stuff doesnt change that much its hardcoded.
			t = self.gatherValue(x, "Triangle Count:")
			if t[0] :
				storage.triangleCount = t[1]
			t = self.gatherValue(x, "Average BVH depth:")
			if t[0] :
				storage.averageBvhDepth = t[1]
			t = self.gatherValue(x, "Raytracer total time:")
			if t[0] :
				storage.totalTime = t[1]
			t = self.gatherValue(x, "Time for all rays (SUM):")
			if t[0] :
				storage.timeRaySum = t[1]
			t = self.gatherValue(x, "Time for triangle intersections (SUM):")
			if t[0] :
				storage.timeTriangleSum = t[1]
			t = self.gatherValue(x, "Time all rays(sum) - triangle(sum):")
			if t[0] :
				storage.timeNodeSum = t[1]

		
		breakPoint = 0

		return storage
			

	
	#first line for files that loop over branchFactor 
	def getFirstLine(self):
		firstLine = "branchFactor, leafSize"
		return firstLine

e = everything()
e.readDataFromFiles()
e.printEverything()
e.printEverythingOneFile()