import os.path
from os import path

#collects the NAME_bx_lx_Info.txt data and writes them into a .txt to use in latex pgfplot

#name = "shiftHappens"
#folderName = "shiftHappensSave"
names = ["shiftHappens", "sponza", "rungholt" , "erato"]
folderNames = ["shiftHappensSave", "sponzaSave", "rungholtSave", "eratoSave"]

idsToSum = [0, 1, 2, 3]
maxBranchingFactor = 16
maxLeafSize = 16

if (len(idsToSum) == 1):
	fResult = open(names[idsToSum[0]] + "Table.txt", "w+")
else:
	fResult = open("AverageTable.txt", "w+")

#i just always calculate the min and max values -> might write down b and l of min??
minNodeInter = [200000000] * len(idsToSum)
minLeafInter = [200000000] * len(idsToSum)
minShadowNodeInter = [200000000] * len(idsToSum)
minShadowLeafInter = [200000000] * len(idsToSum)
maxNodeInter = [0] * len(idsToSum)
maxLeafInter = [0] * len(idsToSum)
maxShadowNodeInter = [0] * len(idsToSum)
maxShadowLeafInter = [0] * len(idsToSum)
fResult.write("branchFactor, leafSize, nodeIntersections, shadowNodeIntersections, allNodeIntersections, leafIntersections, shadowLeafIntersections, allLeafIntersections, leafFullness\n")

#First loop, calc min and max + and if only one file write file
for b in range(maxBranchingFactor - 1):
	#a space after each x axis -> needed so pgfplot uses the data correct
	if(len(idsToSum) == 2):
		fResult.write("\n")
	for l in range(maxLeafSize):
		branch = b + 2
		leaf = l + 1

		nodeInter = 0
		leafInter = 0
		shadowNodeInter = 0
		shadowLeafInter = 0
		fullness = 0

		#branching factor from 2 to 16, leafsize from 1 to 16
		for id in idsToSum:
			tmpNodeInter = 0
			tmpLeafInter = 0
			tmpShadowNodeInter = 0
			tmpShadowLeafInter = 0
			tmpFullness = 0
			fileName = folderNames[id] + "/" + names[id] + "_b" + str(branch) + "_l" + str(leaf) + "_Info.txt"

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
									tmpShadowNodeInter += value
									minShadowNodeInter[id] = min(value, minShadowNodeInter[id])
									maxShadowNodeInter[id] = max(value, maxShadowNodeInter[id])
								except ValueError:
									pass
						elif (x.find("node intersections:") != -1):
							for t in x.split():
								try:
									value = float(t)
									tmpNodeInter += value
									minNodeInter[id] = min(value, minNodeInter[id])
									maxNodeInter[id] = max(value, maxNodeInter[id])
								except ValueError:
									pass

						#leaf intersections (normal and shadowray)
						if (x.find("shadow leaf intersections:") != -1):
							for t in x.split():
								try:
									value = float(t)
									tmpShadowLeafInter += value
									minShadowLeafInter[id] = min(value, minShadowLeafInter[id])
									maxShadowLeafInter[id] = max(value, maxShadowLeafInter[id])
								except ValueError:
									pass
						elif (x.find("leaf intersections:") != -1):
							for t in x.split():
								try:
									value = float(t)
									tmpLeafInter += value
									minLeafInter[id] = min(value, minLeafInter[id])
									maxLeafInter[id] = max(value, maxLeafInter[id])
								except ValueError:
									pass

						#fullness: (for now i use leaf fullness for memory efficiency (child fullness is not that important i think))
						if (x.find("averag leaf fullness:") != -1):
							for t in x.split():
								try:
									#normalize fullness by leafsize
									tmpFullness = float(t) / leaf
								except ValueError:
									pass
				leafInter += tmpLeafInter
				nodeInter += tmpNodeInter
				shadowLeafInter += tmpShadowLeafInter
				shadowNodeInter += tmpShadowNodeInter
				fullness += tmpFullness
			else:
				print("Was not able to open file: " + folderNames[id] + "/" + names[id] + "_b" + str(branch) + "_l" + str(leaf) + "_Info.txt")
		if len(idsToSum) == 1:
			fResult.write(str(branch) + ", " + str(leaf)+ ", " + str(nodeInter) + ", " + str(shadowNodeInter) + ", " + str(shadowNodeInter + nodeInter) + ", " +str(leafInter) +", " +str(shadowLeafInter) + ", " +str(leafInter + shadowLeafInter) +  ", " + str(fullness) + "\n")
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

			#branching factor from 2 to 16, leafsize from 1 to 16
			for id in idsToSum:
				tmpNodeInter = 0
				tmpLeafInter = 0
				tmpShadowNodeInter = 0
				tmpShadowLeafInter = 0
				tmpFullness = 0
				fileName = folderNames[id] + "/" + names[id] + "_b" + str(branch) + "_l" + str(leaf) + "_Info.txt"


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
										tmpShadowNodeInter += value
									except ValueError:
										pass
							elif (x.find("node intersections:") != -1):
								for t in x.split():
									try:
										value = float(t)
										tmpNodeInter += value
									except ValueError:
										pass

							#leaf intersections (normal and shadowray)
							if (x.find("shadow leaf intersections:") != -1):
								for t in x.split():
									try:
										value = float(t)
										tmpShadowLeafInter += value
									except ValueError:
										pass
							elif (x.find("leaf intersections:") != -1):
								for t in x.split():
									try:
										value = float(t)
										tmpLeafInter += value
									except ValueError:
										pass

							#fullness: (for now i use leaf fullness for memory efficiency (child fullness is not that important i think))
							if (x.find("averag leaf fullness:") != -1):
								for t in x.split():
									try:
										#normalize fullness by leafsize
										tmpFullness = float(t) / leaf
									except ValueError:
										pass
					leafInter += (tmpLeafInter - minLeafInter[id]) / (maxLeafInter[id] - minLeafInter[id])
					nodeInter += (tmpNodeInter - minNodeInter[id]) / (maxNodeInter[id] - minNodeInter[id])
					shadowLeafInter += (tmpShadowLeafInter - minShadowLeafInter[id]) / (maxShadowLeafInter[id] - minShadowLeafInter[id])
					shadowNodeInter += (tmpShadowNodeInter - minShadowNodeInter[id]) / (maxShadowNodeInter[id] - minShadowNodeInter[id])
					fullness += tmpFullness
				else:
					print("Was not able to open file: " + folderNames[id] + "/" + names[id] + "_b" + str(branch) + "_l" + str(leaf) + "_Info.txt")
			leafInter /= len(idsToSum)
			shadowLeafInter /= len(idsToSum)
			nodeInter /= len(idsToSum)
			shadowNodeInter /= len(idsToSum)
			fullness /= len(idsToSum)
			fResult.write(str(branch) + ", " + str(leaf)+ ", " + str(nodeInter) + ", " + str(shadowNodeInter) + ", " + str(shadowNodeInter + nodeInter) + ", " +str(leafInter) +", " +str(shadowLeafInter) + ", " +str(leafInter + shadowLeafInter) +  ", " + str(fullness) + "\n")