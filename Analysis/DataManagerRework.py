import os.path
from os import path

class everything:
	def __init__(self):
		self.names = ["shiftHappens", "sponza", "rungholt", "erato"]
		self.prefix = "SaveSorted"

		# -1 for all, id otherwise (starting with 0)
		self.singeIdOverride = -1

		#maximum branchingfactor and max leafsite
		self.maxBranchingFactor = 16
		self.maxLeafSize = 16

		#temprary cost function. needs replacement
		self.nodeCostFactor = 1/4
		self.leafCostFactor = 1

		#names of variables like node Intersection count
		self.variableNames = [
			"primary intersections node:",
			"primary intersections leaf:",
			"secondary intersections node:",
			"secondary intersections leaf:"
			]
		self.variableOutputNames = [
			"primaryNodeIntersections:",
			"primaryLeafIntersections:",
			"secondaryNodeIntersections",
			"secondaryLeafIntersections"
			]
		#cost metrics that are split into node and leaf version. -> leaf * leaffactor and node * nodefactor to get total thing
		#convetion: its xxxbalablaxxx node: and xxxbalablaxxx leaf:
		self.costMetricNames = [
			"sah of ",
			"end point overlap of ",
			"primary intersections ",
			"secondary intersections "
			]
		self.costMetricOutputNames = [
			"Sah",
			"Epo",
			"PrimaryCost",
			"SecondaryCost"
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
		
		#fullness would be some special thing becuase its divided by leafsize
		# -> could do variables divided by leafsize and ones divided by branchFactor
		# -> and ones multiplied by it?


		#now for each type of variable: array[sceneId][nameId] -> maxValue
		self.variablesMax = [[0 for i in self.variableNames]for i in self.names]
		self.variablesMin = [[2000000 for i in self.variableNames]for i in self.names]
		self.costMetricsMax = [[0 for i in range(len(self.costMetricNames)*2)]for i in self.names]
		self.costMetricsMin = [[2000000 for i in range(len(self.costMetricNames)*2)]for i in self.names]
		self.normalizedVariablesMax = [[0 for i in self.normalizedVariableNames]for i in self.names]
		self.normalizedVariablesMin = [[2000000 for i in self.normalizedVariableNames] for i in self.names]
		#could also store min and max id (b and l)

		# for each type of variable: array[nameId] -> value of current file.
		# (so we loop over a file, fill this array and at the end print one line in the summary)
		self.variables = [[0 for i in self.variableNames]for i in self.names]
		self.costMetrics = [[0 for i in range(len(self.costMetricNames)*2)]for i in self.names]
		self.normalizedVariables = [[0 for i in self.normalizedVariableNames]for i in self.names]

	def run(self):
		# now loop over all scenes to do the single scene file (and collect min max)
		# then loop over all and get averages

		for i in range(len(self.names)):
			fResult = open(self.names[i] + self.prefix + "TableSorted.txt", "w+")
			# write the first line in the table (the one with the variable names)

			for b in range(self.maxBranchingFactor - 1):
				#one empty line after each branching factor
				fResult.write("\n")
				for l in range(self.maxLeafSize):
					branch = b + 2
					leaf = l + 1

					#i just test both files for all varialbe names
					fileName = self.names[i] + self.prefix + "/" + self.names[i] + "_b" + str(branch) + "_l" + str(leaf) + "_Info.txt"
					fileName2 = self.names[i] + self.prefix + "/" + self.names[i] + "_b" + str(branch) + "_l" + str(leaf) + "_BVHInfo.txt"
					if (path.exists(fileName)):
						#open file and read important values
						f = open(fileName, "r")
						if f.mode == 'r':
							for x in f:
								#now loop over the different variables and call somefunctions to hande the rest
								for v in range(len(self.variableNames)):
									self.gatherVariables(i,v,x)
								for v in range(len(self.costMetricNames)):
									self.gatherCostVariables(i,v,x)
								for v in range(len(self.normalizedVariableNames)):
									self.gatherNormalizedVariables(i, v, x)
					else:
						print("Was not able to open file: " + fileName)
					if (path.exists(fileName2)):
						#open file and read important values
						f = open(fileName2, "r")
						if f.mode == 'r':
							for x in f:
								#now loop over the different variables and call somefunctions to hande the rest
								for v in range(len(self.variableNames)):
									self.gatherVariables(i,v,x)
								for v in range(len(self.costMetricNames)):
									self.gatherCostVariables(i,v,x)
								for v in range(len(self.normalizedVariableNames)):
									self.gatherNormalizedVariables(i, v, x)
					else:
						print("Was not able to open file: " + fileName)
					
					#TODO: need to gather min and max of cost functions

					#write file:
					res = self.getLine(i, branch, leaf)
					fResult.write(res)

	def getLine(self, sceneId, branch, leaf):
		result = str(branch) + ", " + str(leaf)
		for value in self.variables[sceneId]:
			result += ", " + str(value)
		for value in self.costMetrics[sceneId]:
			result += ", " + str(value)
		for value in self.normalizedVariables[sceneId]:
			result += ", " + str(value)
		return result + "\n"

	def gatherVariables(self, sceneId, variableId, string):
		if(string.find(self.variableNames[variableId]) != -1):
			for t in string.split():
				try:
					value = float(t)
					self.variablesMax[sceneId][variableId] = max(self.variablesMax[sceneId][variableId], value)
					self.variablesMin[sceneId][variableId] = min(self.variablesMin[sceneId][variableId], value)
					self.variables[sceneId][variableId] = value
					#only take first value (second one might be average)
					break
				except ValueError:
					pass

	def gatherCostVariables(self, sceneId, variableId, string):
		if(string.find(self.costMetricNames[variableId] + "node:") != -1):
			for t in string.split():
				try:
					value = float(t)
					self.costMetrics[sceneId][variableId * 2] = value * self.nodeCostFactor
					#only take first value (second one might be average)
					break
				except ValueError:
					pass
		if(string.find(self.costMetricNames[variableId] + "leaf:") != -1):
			for t in string.split():
				try:
					value = float(t)
					self.costMetrics[sceneId][variableId * 2 + 1] = value * self.leafCostFactor
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
					self.normalizedVariables[sceneId][variableId] = value
					#only take first value (second one might be average)
					break
				except ValueError:
					pass

e = everything()
e.run()