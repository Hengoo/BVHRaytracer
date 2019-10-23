import os.path
from os import path

#collects the NAME_bx_lx_Info.txt data and writes them into a .txt to use in latex pgfplot

#I usualy rename the Analysis folder to something like nameSave or nameSaveSorted.
names = ["shiftHappens", "sponza", "rungholt" , "erato", "lizard"]
#folderNames = ["shiftHappensSave", "sponzaSaveSorted", "rungholtSave", "eratoSave", "lizard"]
folderNames = ["shiftHappensSaveSorted", "sponzaSaveSorted", "rungholtSaveSorted", "eratoSaveSorted"]

#if a singe id is chosen [2] it creates the table for this one scene
#if multiple ids [0,1,2,3] are given then it creates the average table
idsToSum = [0,1,2,3]


#maximum branchingfactor and max leafsite
maxBranchingFactor = 16
maxLeafSize = 16

#temprary cost function. needs replacement
nodeCostFactor = 1
leafCostFactor = 4

if (len(idsToSum) == 1):
	fResult = open(names[idsToSum[0]] + "TableSorted.txt", "w+")
else:
	fResult = open("AverageTableSorted.txt", "w+")

#i just always calculate the min and max values -> might write down b and l of min??
minNodeInter = [200000000] * len(names)
minLeafInter = [200000000] * len(names)
minShadowNodeInter = [200000000] * len(names)
minShadowLeafInter = [200000000] * len(names)
minCost = [200000000] * len(names)
minShadowCost = [200000000] * len(names)
minEpo = [200000000] * len(names)
minSah = [200000000] * len(names)

maxNodeInter = [0] * len(names)
maxLeafInter = [0] * len(names)
maxShadowNodeInter = [0] * len(names)
maxShadowLeafInter = [0] * len(names)
maxCost = [0] * len(names)
maxShadowCost = [0] * len(names)
maxEpo = [0] * len(names)
maxSah = [0] * len(names)

fResult.write("branchFactor, leafSize, nodeIntersections, shadowNodeIntersections, allNodeIntersections, leafIntersections, shadowLeafIntersections, allLeafIntersections, leafFullness, cost, shadowCost, sah, epo, wasteFactor\n")

#First loop, calc min and max + and if only one file write file
for b in range(maxBranchingFactor - 1):
	#a space after each x axis -> needed so pgfplot uses the data correct
	if(len(idsToSum) == 1):
		fResult.write("\n")
	for l in range(maxLeafSize):
		branch = b + 2
		leaf = l + 1

		nodeInter = 0
		leafInter = 0
		shadowNodeInter = 0
		shadowLeafInter = 0
		fullness = 0
		cost = 0
		shadowCost = 0
		epo = 0
		sah = 0
		wasteFactor = 0
		shadowWasteFactor = 0

		#branching factor from 2 to 16, leafsize from 1 to 16
		for id in idsToSum:
			#need tmp value for things that use the cost factor
			tmpCost = 0
			tmpShadowCost = 0
			tmpEpo = 0
			tmpSah = 0
			fileName = folderNames[id] + "/" + names[id] + "_b" + str(branch) + "_l" + str(leaf) + "_Info.txt"
			fileName2 = folderNames[id] + "/" + names[id] + "_b" + str(branch) + "_l" + str(leaf) + "_BVHInfo.txt"
			if(path.exists(fileName2)):
				#open file and read important values
				f = open(fileName2, "r")
				if f.mode == 'r':
					for x in f:
						#approach for all keywords:
						#check if keyword fits ->#find number -> add number * factor to cost    (later also gather color)

						if (x.find("Sah of Nodes:") != -1):
							for t in x.split():
								try:
									tmpSah += float(t) * nodeCostFactor
									#take first value (second one is the average)
									break
								except ValueError:
									pass
						if (x.find("Sah of leafs:") != -1):
							for t in x.split():
								try:
									tmpSah += float(t) * leafCostFactor
									#take first value (second one is the average)
									break
								except ValueError:
									pass
						if (x.find("End-Point Overlap of Nodes:") != -1):
							for t in x.split():
								try:
									tmpEpo += float(t) * nodeCostFactor
									#take first value (second one is the average)
									break
								except ValueError:
									pass
						if (x.find("End-Point Overlap of Leafs:") != -1):
							for t in x.split():
								try:
									tmpEpo += float(t) * leafCostFactor
									#take first value (second one is the average)
									break
								except ValueError:
									pass
						minSah[id] = min(tmpSah, minSah[id])
						maxSah[id] = max(tmpSah, maxSah[id])
						minEpo[id] = min(tmpEpo, minEpo[id])
						maxEpo[id] = max(tmpEpo, maxEpo[id])
				sah += tmpSah
				epo += tmpEpo

			if(path.exists(fileName)):
				#open file and read important values
				f = open(fileName, "r")
				if f.mode == 'r':
					for x in f:
						#approach for all keywords:
						#check if keyword fits ->#find number -> add number * factor to cost    (later also gather color)
						

						#node intersections (normal and shadowray)
						if (x.find("shadow node intersections:") != -1):
							for t in x.split():
								try:
									value = float(t)
									shadowNodeInter += value
									minShadowNodeInter[id] = min(value, minShadowNodeInter[id])
									maxShadowNodeInter[id] = max(value, maxShadowNodeInter[id])
									tmpShadowCost += value * nodeCostFactor
								except ValueError:
									pass
						elif (x.find("node intersections:") != -1):
							for t in x.split():
								try:
									value = float(t)
									nodeInter += value
									minNodeInter[id] = min(value, minNodeInter[id])
									maxNodeInter[id] = max(value, maxNodeInter[id])
									tmpCost += value * nodeCostFactor
									
								except ValueError:
									pass

						#leaf intersections (normal and shadowray)
						if (x.find("shadow leaf intersections:") != -1):
							for t in x.split():
								try:
									value = float(t)
									shadowLeafInter += value
									minShadowLeafInter[id] = min(value, minShadowLeafInter[id])
									maxShadowLeafInter[id] = max(value, maxShadowLeafInter[id])
									tmpShadowCost += value * leafCostFactor

								except ValueError:
									pass
						elif (x.find("leaf intersections:") != -1):
							for t in x.split():
								try:
									value = float(t)
									leafInter += value
									minLeafInter[id] = min(value, minLeafInter[id])
									maxLeafInter[id] = max(value, maxLeafInter[id])
									tmpCost += value * leafCostFactor
								except ValueError:
									pass

						#fullness: (for now i use leaf fullness for memory efficiency (child fullness is not that important i think))
						if (x.find("averag leaf fullness:") != -1):
							for t in x.split():
								try:
									#normalize fullness by leafsize
									fullness = float(t) / leaf
								except ValueError:
									pass

						#waste Factor
						if (x.find("shadow waste factor:") != -1):
							for t in x.split():
								try:
									shadowWasteFactor += float(t)
								except ValueError:
									pass
						elif (x.find("waste factor:") != -1):
							for t in x.split():
								try:
									wasteFactor = float(t)
								except ValueError:
									pass
						minShadowCost[id] = min(tmpShadowCost, minShadowCost[id])
						maxShadowCost[id] = max(tmpShadowCost, maxShadowCost[id])
						minCost[id] = min(tmpCost, minCost[id])
						maxCost[id] = max(tmpCost, maxCost[id])
				cost += tmpCost
				shadowCost += tmpShadowCost

			else:
				print("Was not able to open file: " + folderNames[id] + "/" + names[id] + "_b" + str(branch) + "_l" + str(leaf) + "_Info.txt")

		if len(idsToSum) == 1:
			fResult.write(str(branch) + ", " + str(leaf) + ", " + str(nodeInter) + ", " + str(shadowNodeInter) + ", " + str(shadowNodeInter + nodeInter)
			+ ", " + str(leafInter) + ", " + str(shadowLeafInter) + ", " + str(leafInter + shadowLeafInter) + ", " + str(fullness) + ", " + str(cost) + ", " + str(shadowCost)
			+ ", " + str(sah) + ", " + str(epo) + ", " + str(wasteFactor) + "\n")
if len(idsToSum) > 1:
	#Second loop over everything, uses min and max to return normalised average
	for b in range(maxBranchingFactor - 1):
		#a space after each x axis -> needed so pgfplot uses the data correct
		fResult.write("\n")
		for l in range(maxLeafSize):
			branch = b + 2
			leaf = l + 1

			nodeInter = 0
			leafInter = 0
			shadowNodeInter = 0
			shadowLeafInter = 0
			fullness = 0
			cost = 0
			shadowCost = 0
			epo = 0
			sah = 0
			wasteFactor = 0
			shadowWasteFactor = 0

			#branching factor from 2 to 16, leafsize from 1 to 16
			for id in idsToSum:
				fileName = folderNames[id] + "/" + names[id] + "_b" + str(branch) + "_l" + str(leaf) + "_Info.txt"
				fileName2 = folderNames[id] + "/" + names[id] + "_b" + str(branch) + "_l" + str(leaf) + "_BVHInfo.txt"
				if(path.exists(fileName2)):
					#open file and read important values
					f = open(fileName2, "r")
					if f.mode == 'r':
						for x in f:
							#approach for all keywords:
							#check if keyword fits ->#find number -> add number * factor to cost    (later also gather color)

							if (x.find("Sah of Nodes:") != -1):
								for t in x.split():
									try:
										sah += (float(t) * nodeCostFactor) / maxSah[id]
										#take first value (second one is the average)
										break
									except ValueError:
										pass
							if (x.find("Sah of leafs:") != -1):
								for t in x.split():
									try:
										sah += (float(t) * leafCostFactor) / maxSah[id]
										#take first value (second one is the average)
										break
									except ValueError:
										pass
							if (x.find("End-Point Overlap of Nodes:") != -1):
								for t in x.split():
									try:
										epo += (float(t) * nodeCostFactor) / maxEpo[id]
										#take first value (second one is the average)
										break
									except ValueError:
										pass
							if (x.find("End-Point Overlap of Leafs:") != -1):
								for t in x.split():
									try:
										epo += (float(t) * leafCostFactor) / maxEpo[id]
										#take first value (second one is the average)
										break
									except ValueError:
										pass

				if(path.exists(fileName)):
					#open file and read important values
					f = open(fileName, "r")
					if f.mode == 'r':
						for x in f:
							#approach for all keywords:
							#check if keyword fits ->#find number -> add number * factor to cost    (later also gather color)

							#node intersections (normal and shadowray)
							if (x.find("shadow node intersections:") != -1):
								for t in x.split():
									try:
										value = float(t)
										shadowNodeInter += value / maxShadowNodeInter[id]
										shadowCost += (value * nodeCostFactor) / maxShadowCost[id]
									except ValueError:
										pass
							elif (x.find("node intersections:") != -1):
								for t in x.split():
									try:
										value = float(t)
										nodeInter += value / maxNodeInter[id]
										cost += (value * nodeCostFactor) / maxCost[id]
									except ValueError:
										pass

							#leaf intersections (normal and shadowray)
							if (x.find("shadow leaf intersections:") != -1):
								for t in x.split():
									try:
										value = float(t)
										shadowLeafInter += value / maxShadowLeafInter[id]
										shadowCost += (value * leafCostFactor) / maxShadowCost[id]
									except ValueError:
										pass
							elif (x.find("leaf intersections:") != -1):
								for t in x.split():
									try:
										value = float(t)
										leafInter += value / maxLeafInter[id]
										cost += (value * leafCostFactor) / maxCost[id]
									except ValueError:
										pass

							#fullness: (for now i use leaf fullness for memory efficiency (child fullness is not that important i think))
							if (x.find("averag leaf fullness:") != -1):
								for t in x.split():
									try:
										#normalize fullness by leafsize
										fullness += float(t) / leaf
									except ValueError:
										pass
							
							#waste Factor
							if (x.find("shadow waste factor:") != -1):
								for t in x.split():
									try:
										shadowWasteFactor += float(t)
									except ValueError:
										pass
							elif (x.find("waste factor:") != -1):
								for t in x.split():
									try:
										wasteFactor += float(t)
									except ValueError:
										pass
					#leafInter += (tmpLeafInter - minLeafInter[id]) / (maxLeafInter[id] - minLeafInter[id])
					#nodeInter += (tmpNodeInter - minNodeInter[id]) / (maxNodeInter[id] - minNodeInter[id])
					#shadowLeafInter += (tmpShadowLeafInter - minShadowLeafInter[id]) / (maxShadowLeafInter[id] - minShadowLeafInter[id])
					#shadowNodeInter += (tmpShadowNodeInter - minShadowNodeInter[id]) / (maxShadowNodeInter[id] - minShadowNodeInter[id])
					#cost += (tmpCost - minCost[id]) / (maxCost[id] - minCost[id])
					#shadowCost += (tmpShadowCost - minShadowCost[id]) / (maxShadowCost[id] - minShadowCost[id])

					#leafInter += tmpLeafInter / maxLeafInter[id]
					#nodeInter += tmpNodeInter / maxNodeInter[id]
					#shadowLeafInter += tmpShadowLeafInter / maxShadowLeafInter[id]
					#shadowNodeInter += tmpShadowNodeInter / maxShadowNodeInter[id]
					#cost += tmpCost / maxCost[id]
					#shadowCost += tmpShadowCost / maxShadowCost[id]
					#fullness += tmpFullness
				else:
					print("Was not able to open file: " + folderNames[id] + "/" + names[id] + "_b" + str(branch) + "_l" + str(leaf) + "_Info.txt")
			leafInter /= len(idsToSum)
			shadowLeafInter /= len(idsToSum)
			nodeInter /= len(idsToSum)
			shadowNodeInter /= len(idsToSum)
			fullness /= len(idsToSum)
			cost /= len(idsToSum)
			shadowCost /= len(idsToSum)
			sah /= len(idsToSum)
			epo /= len(idsToSum)
			wasteFactor /= len(idsToSum)
			fResult.write(str(branch) + ", " + str(leaf) + ", " + str(nodeInter) + ", " + str(shadowNodeInter) + ", " + str(shadowNodeInter + nodeInter)
			+ ", " + str(leafInter) + ", " + str(shadowLeafInter) + ", " + str(leafInter + shadowLeafInter) + ", " + str(fullness) + ", " + str(cost) + ", " + str(shadowCost)
			+ ", " + str(sah) + ", " + str(epo) + ", " + str(wasteFactor) + "\n")