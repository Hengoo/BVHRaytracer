import os.path
from os import path

#collects the NAME_bx_lx_Info.txt data and writes them into a .txt to use in latex pgfplot

#name = "shiftHappens"
#folderName = "shiftHappensSave"
name = "sponza"
folderName = "sponzaSave"
maxBranchingFactor = 16
maxLeafSize = 16

fResult = open(name+ "Table.txt", "w+")
fResult.write("branchFactor, leafSize, nodeIntersections, leafIntersections, leafFullness\n")
for b in range(maxBranchingFactor - 1):
	#i think this is needed so pgfplot makes a surface instead of some strange line
	fResult.write("\n")
	for l in range(maxLeafSize):
		branch = b + 2
		leaf = l + 1
		#branching factor from 2 to 16, leafsize from 1 to 16
		fileName = folderName + "/" + name + "_b" + str(branch) + "_l" + str(leaf) + "_Info.txt"
		#print(fileName)

		nodeInter = 0
		leafInter = 0
		fullness = 0
		if(path.exists(fileName)):
			#open file and read important values
			f = open(fileName, "r")
			if f.mode == 'r':
				for x in f:
					#approach for all keywords:
					#check if keyword fits ->#find number -> add number * factor to cost    (later also gather color)

					#node intersections (normal and shadowray)
					if (x.find("node intersections:") != -1):
						for t in x.split():
							try:
								nodeInter += float(t)
							except ValueError:
								pass

					#leaf intersections (normal and shadowray)
					if (x.find("leaf intersections:") != -1):
						for t in x.split():
							try:
								leafInter += float(t)
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

					#contents = f.read()
					#print(contents)
			fResult.write(str(branch) + ", " + str(leaf)+ ", " + str(nodeInter)+ ", " +str(leafInter) + ", " + str(fullness) + "\n")

			