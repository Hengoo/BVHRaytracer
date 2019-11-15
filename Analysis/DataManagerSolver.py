import os.path
import math
from os import path

import numpy as np

#loop over all possible tables we are intrested in:
#( nameSSEseq4l4bTable.txt ) are the tables we can use.

class everything:
	def __init__(self):

		#name of the table
		#self.names = ["shiftHappens", "erato", "sponza", "rungholt"]
		self.names = ["sanMiguel"]
		#prefix to the table
		self.prefix = ["SSESeqMemoryLeaf", "SSESeqMemoryNode"]
		self.prefix2 = "Table.txt"

		# 0 = leaf , 1 = node (need to adjust when table change!) (i separate those since i dont want to do a combined performance test since it gets messy quite fast)
		self.workType = 1
		self.memoryStepSize = 4

		#maximum branchingfactor and max leafsite
		self.minBranchingFactor = 4
		self.maxBranchingFactor = 64
		self.minLeafSize = 4
		self.maxLeafSize = 64

		#number of "," in the line before the number we want
		self.dataId = [
			4,
			5
		]
		self.dataOutName = [
			"leafComputeCost",
			"nodeComputeCost"
		]

	def run(self):
		#loop over names:
		for name in self.names:
			anyFound = False
			#loop over b and l
			for b in range(self.maxBranchingFactor -(self.minBranchingFactor - 1)):
				#one empty line after each branching factor
				for l in range(self.maxLeafSize - (self.minLeafSize - 1)):

					branch = b + self.minBranchingFactor
					leaf = l + self.minLeafSize
					#open table:
					tableName = name + self.prefix[self.workType] + "b" + str(branch) + "l" + str(leaf) + self.prefix2
					if (path.exists(tableName)):
						if not (anyFound):
							fileName = name + self.prefix[self.workType] + "ComputeCostTable.txt"
							fResult = open(fileName, "w+")
							firstLine = "branchFactor, leafSize, " + self.dataOutName[self.workType] +", memoryCost, " + self.dataOutName[self.workType] + "Norm, memoryCostNorm"
							fResult.write(firstLine + "\n")
						anyFound = True
						#open file and read important values
						f = open(tableName, "r")

						if f.mode == 'r':
							dataPoints = [[] for z in self.dataId]
							dataLeaf = []
							dataBranch = []
							fiterator = iter(f)
							next(fiterator)
							for x in fiterator:
								split = x.split(", ")
								# collect data points:
								for i in range(len(self.dataId)):
									dataPoints[i].append(float(split[self.dataId[i]]))
								dataLeaf.append(float(split[1]))
								dataBranch.append(float(split[0]))

						#now convert data to np array
						y = np.array(dataPoints[self.workType])
						if self.workType == 0:
							memoryPart = np.array(dataLeaf)
							computePart = np.array([float(leaf) for i in range(4)])
						else:
							memoryPart = np.array(dataBranch)
							computePart = np.array([float(branch) for i in range(4)])
						memoryPart /= self.memoryStepSize
						computePart /= self.memoryStepSize
						
						A = np.vstack([memoryPart, computePart]).T

						result, residual, rank, singular= np.linalg.lstsq(A, y,rcond=None)
						#print("result " + str(branch) + ", " + str(leaf) + ", " + str(m))
						computeCost = result[1]
						memoryCost = result[0]
						factor = 1 / (computeCost + memoryCost)
						computeNorm = factor * computeCost
						memoryNorm = factor * memoryCost
						
						fResult.write(str(branch)+", " + str(leaf)+ ", "+ str(computeCost)+", "+ str(memoryCost)+ ", "+ str(computeNorm)+", "+ str(memoryNorm)+ "\n")
			if anyFound:
				fResult.close()


program = everything()
program.run()