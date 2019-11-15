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
		self.names = ["sanMiguel"]
		#prefixTo the folderNames
		#self.prefix = "SSESeqMemoryLeaf"
		self.prefix = "SSESeqMemoryNode"
		#Prefix to the output txt (so its sceneNamePrefix.txt)
		self.outputPrefix = ""

		#maximum branchingfactor and max leafsite
		self.minBranchingFactor = 2
		self.maxBranchingFactor = 64
		self.minLeafSize = 1
		self.maxLeafSize = 64

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
			"Raytracer total time:",
			"Time for all rays (SUM):",
			"Time for triangle intersections (SUM):",
			"Time all rays(sum) - triangle(sum):",
		]
		self.normalizedVariableOutputNames = [
			"raytracerTotalTime",
			"rayTimeSum",
			"triangleIntersectionSum",
			"rayTimeSumWithoutTri",
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

		self.possibleMemorySizes = [4,8,12,16,20,24,28,32,40,48,56,64]

	def resetArrays(self):
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

	#This run only goes over "optimal" ml and mb combinations
	def run(self):
		# now loop over all scenes to do the single scene file (and collect min max)
		# then loop over all and get averages

		firstLine = self.getFirstLine()
		
		#first loop over different possible memory sizes for l and b
		self.resetArrays()
		anyFound = False
		#now do what normal data manger does, loop over b and l and write files
		for i in range(len(self.names)):
			fileName1 = self.names[i] + self.prefix + "optimal" + "TableWithSpace" + self.outputPrefix + ".txt"
			fileName2 = self.names[i] + self.prefix + "optimal" + "Table" + self.outputPrefix + ".txt"
			fResult = open(fileName1, "w+")
			fResult2 = open(fileName2, "w+")
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
					fileName = self.folder + self.names[i] + self.prefix + "/" + self.names[i] + "_b" + str(branch) + "_l" + str(leaf) + "_mb" + str(branch) + "_ml" + str(leaf)+ "_Perf.txt"
					if (path.exists(fileName)):
						anyFound = True
						#open file and read important values
						f = open(fileName, "r")
						if f.mode == 'r':
							for x in f:
								#now loop over the different variables and call somefunctions to hande the rest
								self.gatherAll(i, x, branch, leaf)
						for v in range(len(self.costMetricNames)):
							self.costMetricsMin[i][v] = min(self.costMetrics[i][v], self.costMetricsMin[i][v])
							self.costMetricsMax[i][v] = max(self.costMetrics[i][v], self.costMetricsMax[i][v])

						#write file:
						res = self.getLine(i, branch, leaf)
						fResult.write(res)
						fResult2.write(res)
					#else:
					#	print("Was not able to open file: " + fileName)
			fResult.close()
			fResult2.close()
			if(not anyFound):
				os.remove(fileName1)
				os.remove(fileName2)

		if (len(self.names) == 1):
			return

		fileName1 = "AverageTable" + self.prefix + "optimal" + "TableWithSpace" + self.outputPrefix + ".txt"
		fileName2 = "AverageTable" + self.prefix + "optimal" + "Table" + self.outputPrefix + ".txt"
		#now loop over b and l again and write average file:
		fResult = open(fileName1, "w+")
		fResult2 = open(fileName2, "w+")
		# write the first line in the table (the one with the variable names)
		fResult.write(firstLine + "\n")
		fResult2.write(firstLine + "\n")


		anyFound = False
		for b in range(self.maxBranchingFactor -(self.minBranchingFactor - 1)):
			#one empty line after each branching factor
			fResult.write("\n")
			for l in range(self.maxLeafSize - (self.minLeafSize - 1)):
				branch = b + self.minBranchingFactor
				leaf = l + self.minLeafSize
				#loop over scenes
				
				for i in range(len(self.names)):
					#i just test both files for all varialbe names
					fileName = self.folder + self.names[i] + self.prefix + "/" + self.names[i] + "_b" + str(branch) + "_l" + str(leaf) + "_mb" + str(mb) + "_ml" + str(ml)+ "_Perf.txt"
					if (path.exists(fileName)):
						anyFound = True
						#open file and read important values
						f = open(fileName, "r")
						if f.mode == 'r':
							for x in f:
								#now loop over the different variables and call somefunctions to hande the rest
								self.gatherAll(i, x, branch, leaf)
					#else:
					#	print("Was not able to open file: " + fileName)

				#write file:
				res = self.getAverageLine(branch, leaf)
				fResult.write(res)
				fResult2.write(res)
		
		if (not anyFound):
			#ugly but works
			os.remove(fileName1)
			os.remove(fileName2)

	#run 2 runs over all possible mB and mL combinations
	def run2(self):
		# now loop over all scenes to do the single scene file (and collect min max)
		# then loop over all and get averages

		firstLine = self.getFirstLine()
		
		#first loop over different possible memory sizes for l and b (and ml and mb)
		for mb in self.possibleMemorySizes:
			for ml in self.possibleMemorySizes:
				self.resetArrays()
				memoryText = "mb" + str(mb) + "ml" + str(ml)

				#now do what normal data manger does, loop over b and l and write files
				for i in range(len(self.names)):
					anyFound = False


					for b in range(self.maxBranchingFactor -(self.minBranchingFactor - 1)):
						for l in range(self.maxLeafSize - (self.minLeafSize - 1)):
							branch = b + self.minBranchingFactor
							leaf = l + self.minLeafSize

							#i just test both files for all varialbe names
							fileName = self.folder + self.names[i] + self.prefix + "/" + self.names[i] + "_b" + str(branch) + "_l" + str(leaf) + "_mb" + str(mb) + "_ml" + str(ml)+ "_Perf.txt"
							if (path.exists(fileName)):
								if not (anyFound):
									fileName1 = self.names[i] + self.prefix + memoryText + "Table" + self.outputPrefix + ".txt"
									fResult = open(fileName1, "w+")
									# write the first line in the table (the one with the variable names)
									fResult.write(firstLine + "\n")
								anyFound = True
								#open file and read important values
								f = open(fileName, "r")
								if f.mode == 'r':
									for x in f:
										#now loop over the different variables and call somefunctions to hande the rest
										self.gatherAll(i, x, branch, leaf)
								for v in range(len(self.costMetricNames)):
									self.costMetricsMin[i][v] = min(self.costMetrics[i][v], self.costMetricsMin[i][v])
									self.costMetricsMax[i][v] = max(self.costMetrics[i][v], self.costMetricsMax[i][v])

								#write file:
								res = self.getLine(i, branch, leaf)
								fResult.write(res)
							#else:
							#	print("Was not able to open file: " + fileName)
					if anyFound:
						fResult.close()

	#run 3 runs over all possible mB and mL combinations but in different order (first L and N then Mb and Ml)
	def run3(self):
		# now loop over all scenes to do the single scene file (and collect min max)
		# then loop over all and get averages

		firstLine = self.getFirstLine2()
		tmpNames = self.names.copy()
		if(len(tmpNames) != 1):
			tmpNames.append("Average")
		
		#loop over b and l
		for b in range(self.maxBranchingFactor -(self.minBranchingFactor - 1)):
			for l in range(self.maxLeafSize - (self.minLeafSize - 1)):
				self.resetArrays()
				branch = b + self.minBranchingFactor
				leaf = l + self.minLeafSize
				configText = "b" + str(branch) + "l" + str(leaf)

				#now do what normal data manger does, loop over b and l and write files
				for i in range(len(tmpNames)):
					anyFound = False

					#second loop over mb and ml
					for mb in self.possibleMemorySizes:

						for ml in self.possibleMemorySizes:
							memoryText = "mb" + str(mb) + "ml" + str(ml)

							#check the files we have written in run2
							fileName = self.names[i] + self.prefix + memoryText + "Table" + self.outputPrefix + ".txt"
							if (path.exists(fileName)):
								anyLineFound = False
								#open file and read important values
								f = open(fileName, "r")
								if f.mode == 'r':
									for x in f:
										#search for line with the correct branching and leafsize (first 2 values)
										#remove those first numbers and then give them our mb and ml values
										tmp = x.split(str(branch) + ", " + str(leaf)+", ", 1)
										if len(tmp) == 2:
											if len(tmp[0]) == 0:
												if not (anyFound):
													fileName1 = tmpNames[i] + self.prefix + configText + "Table" + self.outputPrefix + ".txt"
													fResult = open(fileName1, "w+")

													# write the first line in the table (the one with the variable names)
													fResult.write(firstLine + "\n")
												anyFound = True
												anyLineFound = True
												lineResult = tmp[1]

								#write file:
								if anyLineFound:
									#one empty line after each branching factor
									res = str(mb) +", "+ str(ml)+ ", "+ lineResult
									fResult.write(res)
					if anyLineFound:
						fResult.close()
					#if (not anyFound):
						#ugly but works
						#os.remove(fileName1)
						#os.remove(fileName2)

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
	
	#first line for files that loop over branchFactor 
	def getFirstLine(self):
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
		return firstLine

	def getFirstLine2(self):
		firstLine = "branchMemory, leafMemory"
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
		return firstLine

e = everything()
e.run()
e.run2()
e.run3()