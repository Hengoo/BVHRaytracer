import os.path
import math
from os import path


#To use this:

class everything:
	def __init__(self):
		#the folder all the scene folders are in: (leave empty if no folder)
		self.folder = "SavesPerf/"
		#names of the sceneFolders
		#self.names = ["shiftHappens", "erato", "sponza", "rungholt"]
		self.names = ["sponza"]
		#prefixTo the folderNames
		self.prefix = "See"
		#Prefix to the output txt (so its sceneNamePrefix.txt)
		self.outputPrefix = ""

		# -1 for all, id otherwise (starting with 0)
		self.singeIdOverride = -1

		#maximum branchingfactor and max leafsite
		self.minBranchingFactor = 2
		self.maxBranchingFactor = 16
		self.minLeafSize = 1
		self.maxLeafSize = 16

		#temprary cost function. needs replacement
		self.nodeCostFactor = 1
		self.leafCostFactor = 1

		self.cachelineSize = 128

		#names of variables like node Intersection count
		self.variableNames = [
			]
		self.variableOutputNames = [
			]

		#cost metrics that are split into node and leaf version. -> leaf * leaffactor and node * nodefactor to get total thing
		#convetion: its xxxbalablaxxx node: and xxxbalablaxxx leaf:
		self.costMetricNames = [
			]
		self.costMetricOutputNames = [
			]

		#names of normalized variables like wasteFactor
		self.normalizedVariableNames = [
			"Time for all rays (SUM):",
			"Time for triangle intersections (SUM):",
			"Time all rays(sum) - triangle(sum):",
		]
		self.normalizedVariableOutputNames = [
			"rayTimeSum",
			"triangleIntersectionSum",
			"rayTimeWithoutTri",
		]

		#Variables that are multiplied by branching factor:
		self.variableMultBranchNames = [
		]
		self.variableMultBranchOutputNames = [
		]
		#Variables that are multiplied by leaf size:
		self.variableMultLeafNames = [
		]
		self.variableMultLeafOutputNames = [
		]

		#Variables that are multiplied cachline they use
		self.variableNodeCachelinesNames = [
		]
		self.variableNodeCachelinesOutputNames = [
		]

		#fullness would be some special thing becuase its divided by leafsize
		# -> could do variables divided by leafsize and ones divided by branchFactor
		# -> and ones multiplied by it?


		#now for each type of variable: array[sceneId][nameId] -> maxValue
		self.variablesMax = [[0 for i in self.variableNames]for i in self.names]
		self.variablesMin = [[2000000 for i in self.variableNames]for i in self.names]
		self.costMetricsMax = [[0 for i in self.costMetricNames]for i in self.names]
		self.costMetricsMin = [[2000000 for i in self.costMetricNames]for i in self.names]
		self.normalizedVariablesMax = [[0 for i in self.normalizedVariableNames]for i in self.names]
		self.normalizedVariablesMin = [[2000000 for i in self.normalizedVariableNames] for i in self.names]
		self.variableMultBranchMax = [[0 for i in self.variableMultBranchNames]for i in self.names]
		self.variableMultBranchMin = [[2000000 for i in self.variableMultBranchNames] for i in self.names]
		self.variableMultLeafMax = [[0 for i in self.variableMultBranchNames]for i in self.names]
		self.variableMultLeafMin = [[2000000 for i in self.variableMultBranchNames] for i in self.names]
		self.variableNodeCachelineMax = [[0 for i in self.variableMultBranchNames]for i in self.names]
		self.variableNodeCachelineMin = [[2000000 for i in self.variableMultBranchNames] for i in self.names]
		#could also store min and max id (b and l)

		# for each type of variable: array[nameId] -> value of current file.
		# (so we loop over a file, fill this array and at the end print one line in the summary)
		self.variables = [[0 for i in self.variableNames]for i in self.names]
		self.costMetrics = [[0 for i in self.costMetricNames]for i in self.names]
		self.normalizedVariables = [[0 for i in self.normalizedVariableNames] for i in self.names]
		self.variableMultBranch = [[0 for i in self.variableMultBranchNames] for i in self.names]
		self.variableMultLeaf = [[0 for i in self.variableMultLeafNames] for i in self.names]
		self.variableNodeCacheline = [[0 for i in self.variableMultBranchNames] for i in self.names]


	def run(self):
		# now loop over all scenes to do the single scene file (and collect min max)
		# then loop over all and get averages

		firstLine = "branchFactor, leafSize"
		for name in self.variableOutputNames:
			firstLine += ", " + name
		for name in self.costMetricOutputNames:
			firstLine += ", " + name
		for name in self.normalizedVariableOutputNames:
			firstLine += ", " + name
		for name in self.variableMultBranchOutputNames:
			firstLine += ", " + name
		for name in self.variableMultLeafOutputNames:
			firstLine += ", " + name
		for name in self.variableNodeCachelinesOutputNames:
			firstLine += ", " + name

		for i in range(len(self.names)):
			fResult = open(self.names[i] + self.prefix + "TableWithSpace" + self.outputPrefix + ".txt", "w+")
			fResult2 = open(self.names[i] + self.prefix + "Table" + self.outputPrefix + ".txt", "w+")
				# write the first line in the table (the one with the variable names)
			fResult.write(firstLine + "\n")
			fResult2.write(firstLine + "\n")

			for b in range(self.maxBranchingFactor -(self.minBranchingFactor - 1)):
				#one empty line after each branching factor
				fResult.write("\n")
				for l in range(self.maxLeafSize - (self.minLeafSize - 1)):
					branch = b + self.minBranchingFactor
					leaf = l + self.minLeafSize

					#i just test both files for all varialbe names
					fileName = self.folder + self.names[i] + self.prefix + "/" + self.names[i] + "_b" + str(branch) + "_l" + str(leaf) + "_Perf.txt"
					if (path.exists(fileName)):
						#open file and read important values
						f = open(fileName, "r")
						if f.mode == 'r':
							for x in f:
								#now loop over the different variables and call somefunctions to hande the rest
								self.gatherAll(i, x, branch, leaf)
					else:
						print("Was not able to open file: " + fileName)
					
					#TODO: need to gather min and max of cost functions
					for v in range(len(self.costMetricNames)):
						self.costMetricsMin[i][v] = min(self.costMetrics[i][v], self.costMetricsMin[i][v])
						self.costMetricsMax[i][v] = max(self.costMetrics[i][v], self.costMetricsMax[i][v])

					#write file:
					res = self.getLine(i, branch, leaf)
					fResult.write(res)
					fResult2.write(res)

		if (len(self.names) == 1):
			return

		#now loop over b and l again and write average file:
		fResult = open("AverageTableWithSpace" + self.outputPrefix + ".txt", "w+")
		fResult2 = open("AverageTable" + self.outputPrefix + ".txt", "w+")
		# write the first line in the table (the one with the variable names)
		fResult.write(firstLine + "\n")
		fResult2.write(firstLine + "\n")
		for b in range(self.maxBranchingFactor -(self.minBranchingFactor - 1)):
			#one empty line after each branching factor
			fResult.write("\n")
			for l in range(self.maxLeafSize - (self.minLeafSize - 1)):
				branch = b + self.minBranchingFactor
				leaf = l + self.minLeafSize
				#loop over scenes
				for i in range(len(self.names)):
					#i just test both files for all varialbe names
					fileName = self.folder + self.names[i] + self.prefix + "/" + self.names[i] + "_b" + str(branch) + "_l" + str(leaf) + "_Info.txt"
					fileName2 = self.folder + self.names[i] + self.prefix + "/" + self.names[i] + "_b" + str(branch) + "_l" + str(leaf) + "_BVHInfo.txt"
					if (path.exists(fileName)):
						#open file and read important values
						f = open(fileName, "r")
						if f.mode == 'r':
							for x in f:
								#now loop over the different variables and call somefunctions to hande the rest
								self.gatherAll(i, x, branch, leaf)
					else:
						print("Was not able to open file: " + fileName)

				#write file:
				res = self.getAverageLine(branch, leaf)
				fResult.write(res)
				fResult2.write(res)

		
	#returns the line that is written in the averagetable.txt
	def getAverageLine(self, branch, leaf):
		sceneNumber = len(self.names)
		result = str(branch) + ", " + str(leaf)
		for valueId in range(len(self.variableNames)):
			value = 0
			for i in range(len(self.names)):
				value += self.variables[i][valueId] / self.variablesMax[i][valueId]
				self.variables[i][valueId] = 0
			result += ", " + str(value / sceneNumber)

		for valueId in range(len(self.costMetricNames)):
			value = 0
			for i in range(len(self.names)):
				value += self.costMetrics[i][valueId] / self.costMetricsMax[i][valueId]
				self.costMetrics[i][valueId] = 0
			result += ", " + str(value / sceneNumber)

		for valueId in range(len(self.normalizedVariableNames)):
			value = 0
			for i in range(len(self.names)):
				value += self.normalizedVariables[i][valueId]
				self.normalizedVariables[i][valueId] = 0
			result += ", " + str(value / sceneNumber)

		for valueId in range(len(self.variableMultBranchNames)):
			value = 0
			for i in range(len(self.names)):
				value += self.variableMultBranch[i][valueId] / self.variableMultBranchMax[i][valueId]
				self.variableMultBranch[i][valueId] = 0
			result += ", " + str(value / sceneNumber)

		for valueId in range(len(self.variableMultLeafNames)):
			value = 0
			for i in range(len(self.names)):
				value += self.variableMultLeaf[i][valueId] /self.variableMultLeafMax[i][valueId]
				self.variableMultLeaf[i][valueId] = 0
			result += ", " + str(value / sceneNumber)

		for valueId in range(len(self.variableNodeCachelinesNames)):
			value = 0
			for i in range(len(self.names)):
				value += self.variableNodeCacheline[i][valueId] / self.variableNodeCachelineMax[i][valueId]
				self.variableNodeCacheline[i][valueId] = 0
			result += ", " + str(value / sceneNumber)

		return result + "\n"

	#returns the line to for the table.txt and resets the current values
	def getLine(self, sceneId, branch, leaf):
		result = str(branch) + ", " + str(leaf)
		for id in range(len(self.variableNames)):
			value = self.variables[sceneId][id]
			result += ", " + str(value)
			self.variables[sceneId][id] = 0
		for id in range(len(self.costMetricNames)):
			value = self.costMetrics[sceneId][id]
			result += ", " + str(value)
			self.costMetrics[sceneId][id] = 0
		for id in range(len(self.normalizedVariableNames)):
			value = self.normalizedVariables[sceneId][id]
			result += ", " + str(value)
			self.normalizedVariables[sceneId][id] = 0
		for id in range(len(self.variableMultBranchNames)):
			value = self.variableMultBranch[sceneId][id]
			result += ", " + str(value)
			self.variableMultBranch[sceneId][id] = 0
		for id in range(len(self.variableMultLeafNames)):
			value = self.variableMultLeaf[sceneId][id]
			result += ", " + str(value)
			self.variableMultLeaf[sceneId][id] = 0
		for id in range(len(self.variableNodeCachelinesNames)):
			value = self.variableNodeCacheline[sceneId][id]
			result += ", " + str(value)
			self.variableNodeCacheline[sceneId][id] = 0
		return result + "\n"

	def gatherAll(self, i, x, branch, leaf):
		for v in range(len(self.variableNames)):
			self.gatherVariables(i,v,x)
		for v in range(len(self.costMetricNames)):
			self.gatherCostVariables(i,v,x)
		for v in range(len(self.normalizedVariableNames)):
			self.gatherNormalizedVariables(i, v, x)
		for v in range(len(self.variableMultBranchNames)):
			self.gatherMultBranchVariables(i, v, x, branch)
		for v in range(len(self.variableMultLeafNames)):
			self.gatherMultLeafVariables(i, v, x, leaf)
		for v in range(len(self.variableMultBranchNames)):
			self.gatherNodeCachelineVariables(i, v, x, branch)

	def gatherVariables(self, sceneId, variableId, string):
		if(string.find(self.variableNames[variableId]) != -1):
			for t in string.split():
				try:
					value = float(t)
					self.variablesMax[sceneId][variableId] = max(self.variablesMax[sceneId][variableId], value)
					self.variablesMin[sceneId][variableId] = min(self.variablesMin[sceneId][variableId], value)
					self.variables[sceneId][variableId] += value
					#only take first value (second one might be average)
					break
				except ValueError:
					pass

	def gatherCostVariables(self, sceneId, variableId, string):
		if(string.find(self.costMetricNames[variableId] + "node:") != -1):
			for t in string.split():
				try:
					value = float(t)
					self.costMetrics[sceneId][variableId] += value * self.nodeCostFactor
					#only take first value (second one might be average)
					break
				except ValueError:
					pass
		if(string.find(self.costMetricNames[variableId] + "leaf:") != -1):
			for t in string.split():
				try:
					value = float(t)
					self.costMetrics[sceneId][variableId] += value * self.leafCostFactor
					#only take first value (second one might be average)
					break
				except ValueError:
					pass

	def gatherNormalizedVariables(self, sceneId, variableId, string):
		if(string.find(self.normalizedVariableNames[variableId]) != -1):
			for t in string.split():
				try:
					value = float(t)
					self.normalizedVariablesMax[sceneId][variableId] = max(self.normalizedVariablesMax[sceneId][variableId], value)
					self.normalizedVariablesMin[sceneId][variableId] = min(self.normalizedVariablesMin[sceneId][variableId], value)
					self.normalizedVariables[sceneId][variableId] += value
					#only take first value (second one might be average)
					break
				except ValueError:
					pass
	def gatherMultBranchVariables(self, sceneId, variableId, string, branch):
		if(string.find(self.variableMultBranchNames[variableId]) != -1):
			for t in string.split():
				try:
					value = float(t) * branch
					self.variableMultBranchMax[sceneId][variableId] = max(self.variableMultBranchMax[sceneId][variableId], value)
					self.variableMultBranchMin[sceneId][variableId] = min(self.variableMultBranchMin[sceneId][variableId], value)
					self.variableMultBranch[sceneId][variableId] += value
					#only take first value (second one might be average)
					break
				except ValueError:
					pass

	def gatherMultLeafVariables(self, sceneId, variableId,string, leaf):
		if(string.find(self.variableMultLeafNames[variableId]) != -1):
			for t in string.split():
				try:
					value = float(t) * leaf
					self.variableMultLeafMax[sceneId][variableId] = max(self.variableMultLeafMax[sceneId][variableId], value)
					self.variableMultLeafMin[sceneId][variableId] = min(self.variableMultLeafMin[sceneId][variableId], value)
					self.variableMultLeaf[sceneId][variableId] += value
					#only take first value (second one might be average)
					break
				except ValueError:
					pass
	
	def gatherNodeCachelineVariables(self, sceneId, variableId, string, branch):
		if(string.find(self.variableMultBranchNames[variableId]) != -1):
			for t in string.split():
				try:
					byteNeeded = branch * 32
					factor = byteNeeded / self.cachelineSize
					value = float(t) * math.ceil(factor)
					self.variableNodeCachelineMax[sceneId][variableId] = max(self.variableMultBranchMax[sceneId][variableId], value)
					self.variableNodeCachelineMin[sceneId][variableId] = min(self.variableMultBranchMin[sceneId][variableId], value)
					self.variableNodeCacheline[sceneId][variableId] += value
					#only take first value (second one might be average)
					break
				except ValueError:
					pass
e = everything()
e.run()