import os.path
import math
from os import path

import numpy as np
import scipy
import scipy.optimize


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

#loop over all possible tables we are intrested in:
#( nameSSEseq4l4bTable.txt ) are the tables we can use.

class storageType:
	def __init__(self, nameId, subdivision, branch, leaf, triangleCount, averageBvhDepth,totalTime, computeTime, memoryTime, memoryRelative):
		self.branch = branch
		self.leaf = leaf
		self.nameId = nameId
		self.subdivision = subdivision
		self.triangleCount = triangleCount
		self.averageBvhDepth = averageBvhDepth
		self.totalTime = totalTime
		self.computeTime = computeTime
		self.memoryTime = memoryTime
		self.memoryRelative = memoryRelative

class everything:
	def __init__(self, workType = 0, gangType = 0):

		# 0 = leaf , 1 = node (need to adjust when table change!) (i separate those since i dont want to do a combined performance test since it gets messy quite fast)
		self.workType = workType
		self.workName = ["Leaf", "Node"]
		# 0 = avx, sse = 1
		self.gangType = gangType
		self.gangName = ["Avx", "Sse"]

		self.subdivisionRange = [0, 0]

		#nameIds of the tables:
		#self.names = [4, 9]
		self.names = [7,8,9,10,11,12]

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
		
		storage = [[] for _ in range(len(self.names))]

		for loopId, nameId in enumerate(self.names):
			name = allNames[nameId]
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
							#storagePerName.append([branch, leaf, dataPoints[0][0], dataPoints[1][0], dataPoints[self.workType + 2][0], computeCost, memoryCost, memoryFactor, name, s])

							storagePerName.append(storageType(nameId, s, branch, leaf, dataPoints[0][0], dataPoints[1][0], dataPoints[self.workType + 2][0], computeCost, memoryCost, memoryFactor, ))
							

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
					storage[loopId].append(storagePerName)
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
				for sceneStorage in storage:
					for subStorage in sceneStorage:
						for s in subStorage:
							if s.branch == branch and s.leaf == leaf:
								if not anyFound:
									anyFound = True

								triangleCount.append(s.triangleCount)
								averageBvhDepth.append(s.averageBvhDepth)
								totalTime.append(s.totalTime)
								memoryRelative.append(s.memoryRelative)
								

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

					#current idea: render the same scene with different subdivision settings. (so once render the scene with x triangles, x*2, x*3, x*4 ,...)
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
				for sceneStorage in storage:
					for subStorage in sceneStorage:
						for s in subStorage:
							if s.branch == branch and s.leaf == leaf:
								if not anyFound:
									anyFound = True

									#overfiew over multiple scenes:
									fileName = "SavesPerf/Laptop/Summary/" + self.prefix + "Perf_N" + str(branch) +"L" + str(leaf) + ".txt"
									fResult = open(fileName, "w+")
									firstLine = "name, nameId, subdivision, triangleCount, averageBvhDepth, totalTime, computeTime, memoryTime , memoryRelative"
									fResult.write(firstLine + "\n")

								#TODO: aupdate when second part is done
								line = self.makeLine([allNames[s.nameId], s.nameId, s.subdivision, s.triangleCount, s.averageBvhDepth, s.totalTime, s.computeTime, s.memoryTime, s.memoryRelative])
								fResult.write( line + "\n")
	def makeLine(self, array):
		line = "" + str(array[0])
		array.pop(0)
		for element in array:
			line += ", " + str(element)
		return line

doAll = True
# 0 = leaf , 1 = node (need to adjust when table change!) (i separate those since i dont want to do a combined performance test since it gets messy quite fast)
workType = 0
# 0 = avx, sse = 1
gangType = 0
if doAll:
	for i in range(2):
		for j in range(2):
			program = everything(i,j)
			program.run()
else:
	program = everything(workType, gangType)
	program.run()
