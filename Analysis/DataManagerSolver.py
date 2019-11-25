import os.path
import math
from os import path

import numpy as np
import scipy
import scipy.optimize

#loop over all possible tables we are intrested in:
#( nameSSEseq4l4bTable.txt ) are the tables we can use.

class everything:
	def __init__(self):

		# 0 = leaf , 1 = node (need to adjust when table change!) (i separate those since i dont want to do a combined performance test since it gets messy quite fast)
		self.workType = 0
		self.workName = ["Leaf", "Node"]
		# 0 = avx, sse = 1
		self.gangType = 1
		self.gangName = ["Avx", "Sse"]

		self.subdivisionRange = [0, 20]

		#name of the table
		#self.names = ["shiftHappens", "erato", "sponza", "rungholt"]
		#self.names = ["breakfast","sanMiguel", "gallery", "amazonLumberyardCombinedExterior", "amazonLumberyardInterior","amazonLumberyardExterior"]
		self.names = ["sponza"]
		#prefix to the table
		#self.prefix = ["SSESeqMemoryLeaf", "SSESeqMemoryNode"]
		#self.prefix = ["AVXSeqMemoryLeaf", "AVXSeqMemoryNode"]
		self.prefix = self.workName[self.workType] + "Memory" + self.gangName[self.gangType]
		self.prefix2 = "Table.txt"

		self.outputFolderName = "SavesPerf/Laptop/Summary/"

		self.memoryStepSize = 4

		#maximum branchingfactor and max leafsite
		self.minBranchingFactorList = [[8,2],[4,2]]
		self.maxBranchingFactorList = [[8,64],[4,64]]
		self.minLeafSizeList = [[1,8],[1,4]]
		self.maxLeafSizeList = [[64, 8], [64, 4]]

		self.minBranchingFactor = self.minBranchingFactorList[self.gangType][self.workType]
		self.maxBranchingFactor = self.maxBranchingFactorList[self.gangType][self.workType]
		self.minLeafSize = self.minLeafSizeList[self.gangType][self.workType]
		self.maxLeafSize = self.maxLeafSizeList[self.gangType][self.workType]

		#number of "," in the line before the number we want
		self.dataId = [
			2,
			3,
			6,
			7,
		]
		#names that change with leaf / node
		self.dataOutName = [
			"leafComputeCost",
			"nodeComputeCost"
		]

	def run(self):
		#loop over names:

		#arrays that will be used later for tree depth solver stuff
		#b,l,scene, datList
		#datalist is: [b,l,totaltime, memoryRelative]
		
		storage = []

		for nameId, name in enumerate(self.names):
			for s in range(self.subdivisionRange[1] - self.subdivisionRange[0] + 1):
				anyFound = False
				#loop over b and l
				storagePerName = []
				for b in range(self.maxBranchingFactor -(self.minBranchingFactor - 1)):
					#one empty line after each branching factor
					for l in range(self.maxLeafSize - (self.minLeafSize - 1)):

						branch = b + self.minBranchingFactor
						leaf = l + self.minLeafSize
						#open table:
						if (self.subdivisionRange[1] == 0):
							tableName = "SavesPerf/Laptop/Summary/" + name + "/" + name + self.prefix + "b" + str(branch) + "l" + str(leaf) + self.prefix2
						else:
							tableName = "SavesPerf/Laptop/Summary/" + name + "Sub" + str(s) + "/" + name + self.prefix + "b" + str(branch) + "l" + str(leaf) + self.prefix2
						if (path.exists(tableName)):
							if not (anyFound):
								if (self.subdivisionRange[1] == 0):
									fileName = "SavesPerf/Laptop/Summary/" + name + "/" + name + self.prefix + "ComputeCostTable.txt"
								else:
									fileName = "SavesPerf/Laptop/Summary/" + name + "Sub" + str(s) + "/" + name + self.prefix + "ComputeCostTable.txt"
								fResult = open(fileName, "w+")
								firstLine = "branchFactor, leafSize, triangleCount, averageBvhDepth, memorySize, " + self.dataOutName[self.workType] +", memoryCost, " + self.dataOutName[self.workType] + "Norm, memoryCostNorm, memoryRelative"
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
							y = np.array(dataPoints[self.workType + 2])
							if self.workType == 0:
								memoryPart = np.array(dataLeaf)
								computePart = np.array([float(leaf) for i in range(4)])
							else:
								memoryPart = np.array(dataBranch)
								computePart = np.array([float(branch) for i in range(4)])
							
							A = np.vstack([memoryPart, computePart]).T

							result, residual, rank, singular = np.linalg.lstsq(A, y, rcond=None)

							computeCost = result[1]
							memoryCost = result[0]
							normFactor = 1 / (computeCost + memoryCost)
							computeNorm = normFactor * computeCost
							memoryNorm = normFactor * memoryCost
							memoryFactor = memoryCost / computeCost

							#store data for second iteration
							storagePerName.append([branch, leaf, dataPoints[0][0], dataPoints[1][0] , dataPoints[self.workType + 2][0], memoryFactor, name, s])

							"""
							#rework version reformed as linear system
							A2 = np.vstack([-memoryPart, y]).T
							y2 = computePart
							result2, residual, rank, singular = np.linalg.lstsq(A2, y2, rcond=None)
							res2 = 1 / result2[1]
							res = result2[0] * res2
							"""
							
							"""
							#i keep this here in case i need non linear least squares later
							#scipy.optimize.leastsq for non linear least squares.
							#good explanation: https://stackoverflow.com/questions/19791581/how-to-use-leastsq-function-from-scipy-optimize-in-python-to-fit-both-a-straight
							#n = nodes, pm = padMemory
							n = computePart[0]
							func = lambda tpl, pm: tpl[0] * (tpl[1] * pm + n)
							
							errorFunc = lambda tpl, pm, y: func(tpl, pm) - y
							
							#initial tupel values
							tplInitial = (1.0, 1.0)
							
							#tplFinal,success= scipy.optimize.leastsq(errorFunc,tplInitial[:],args=(memoryPart,y))
							#above is the soon depricated version
							
							result= scipy.optimize.least_squares(errorFunc,tplInitial[:],args=(memoryPart,y))
							"""
							fResult.write(str(branch) + ", " + str(leaf) + ", " + str(memoryPart[0]) + ", " + str(computeCost) + ", " + str(memoryCost) + ", " + str(computeNorm) + ", " + str(memoryNorm) + ", " + str(memoryFactor) + "\n")
				if len(storagePerName) != 0:
					storage.append(storagePerName)
		if anyFound:
			fResult.close()
		#now loop over the different scenes and do analysis depending on tree depth

	
		for b in range(self.maxBranchingFactor -(self.minBranchingFactor - 1)):
			#one empty line after each branching factor
			for l in range(self.maxLeafSize - (self.minLeafSize - 1)):
				branch = b + self.minBranchingFactor
				leaf = l + self.minLeafSize

				memoryRelative = []
				totalTime = []
				triangleCount = []
				averageBvhDepth = []
				anyFound = False
				for nameId, name in enumerate(self.names):
					for sub in range(self.subdivisionRange[1] - self.subdivisionRange[0] + 1):
						sceneStorage = storage[nameId * (self.subdivisionRange[1] - self.subdivisionRange[0] + 1) + sub]
						for s in sceneStorage:
							if s[0] == branch and s[1] == leaf:
								if not anyFound:
									anyFound = True

								triangleCount.append(s[2])
								averageBvhDepth.append(s[3])
								totalTime.append(s[4])
								memoryRelative.append(s[5])
								

				if anyFound:
					fResult.close()
					#calculates the relative memory depending on tri count
					#memoryRelative = x * triCount
					
					ones = np.ones(len(memoryRelative))


					#A = np.vstack([triangleCount]).T
					A = np.vstack([averageBvhDepth]).T

					result, residual, rank, singular = np.linalg.lstsq(A, memoryRelative, rcond=None)
					if (residual > 0.1):
						print("residual a bit high")


					#calculates the influence of the tri count to total time. (assumes linear function, not sure if it is -> residual seems quite high?)
					#totalTime = x * triCount * memoryRelative + x * triCount
					#for calculating i use : totalTime /(triangleCount * (memoryRelative + 1)) = x
					m = np.array(memoryRelative)
					#t = np.array(triangleCount)
					t = np.array(averageBvhDepth)
					cT = np.array(totalTime)
					y = cT / (t * (m + 1))
					A = np.vstack([ones]).T
					result2, residual2, rank, singular = np.linalg.lstsq(A, y, rcond=None)

					breakpointHolder = 0

					#TODO: test different things to above version (non linear function or else, not sure yet)
					#(also) try to normalize with real intersection count of the scene? not sure what to expect with this one but i want to know how it look

					#current idea: render the same scene with different subdivion settings. (so once render the scene with x triangles, x*2, x*3, x*4 ,...)
					#might be able to calculate scene/camrea complexity with this?
					#TODO: output


					#what i want from output:
					#output per N and B combination.
					#put name in it and tri count / average bvh depth so i can show graphs for each scene

		#final iteration over storage and print one file for each branching factor / leafsize (for node or leaf tests)
		for b in range(self.maxBranchingFactor -(self.minBranchingFactor - 1)):
			#one empty line after each branching factor
			for l in range(self.maxLeafSize - (self.minLeafSize - 1)):
				branch = b + self.minBranchingFactor
				leaf = l + self.minLeafSize

				anyFound = False
				for nameId, name in enumerate(self.names):
					for sub in range(self.subdivisionRange[1] - self.subdivisionRange[0] + 1):
						sceneStorage = storage[nameId * (self.subdivisionRange[1] - self.subdivisionRange[0] + 1) + sub]
						for s in sceneStorage:
							if s[0] == branch and s[1] == leaf:
								if not anyFound:
									anyFound = True

									#overfiew over multiple scenes:
									fileName = "SavesPerf/Laptop/Summary/" + self.prefix + "Perf_N" + str(branch) +"L" + str(leaf) + ".txt"
									fResult = open(fileName, "w+")
									firstLine = "name, sub, triangleCount, averageBvhDepth, computeTime, memoryRelative"
									fResult.write(firstLine + "\n")

								#TODO: add second storage array when its finished
								fResult.write( str(s[6])+", "+ str(s[7])+", "+ str(s[2])+", "+ str(s[3])+", "+ str(s[4])+", "+ str(s[5]) + "\n")
								





program = everything()
program.run()
