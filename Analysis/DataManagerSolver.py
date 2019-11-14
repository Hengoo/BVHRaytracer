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

		# 1 = node, 0 = leaf (need to adjust when table change!) (i separate those since i dont want to do a combined performance test since it gets messy quite fast)
		self.workType = 0
		
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
							fiterator = iter(f)
							next(fiterator)
							for x in fiterator:
								# collect data points:
								for i in range(len(self.dataId)):
									dataPoints[i].append(float(x.split(", ")[self.dataId[i]]))
						#now convert data to np array
						y = np.array(dataPoints[self.workType])
						A = np.array([[ 1., 1.],
							[ 2., 1.],
							[ 3., 1.],
							[4., 1.]])
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
