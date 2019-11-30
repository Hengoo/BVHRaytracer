import os.path
import math
from os import path
#short python script to crop a table to the intresting collums

tableInput = "amazonLumberyardInteriorTableSorted.txt"
tableOutput = "amazonLumberyardInteriorTable.txt"
#tableInput = "amazonLumberyardExteriorTableSorted.txt"
#tableOutput = "amazonLumberyardExteriorTable.txt"
collumsToKeep = ["branchFactor", "leafSize", "primaryNodeIntersections", "primaryAabb", "primaryLeafIntersections", "primaryPrimitive", "primaryAabbSuccessRatio", "primaryTriangleSuccessRatio","secondaryNodeIntersections", "secondaryAabb", "secondaryLeafIntersections", "secondaryPrimitive", "secondaryAabbSuccessRatio", "secondaryTriangleSuccessRatio"]

#open table
if (path.exists(tableInput)):
	f = open(tableInput, "r")
	if f.mode == 'r':

		#get ids of collums to keep from the first line.
		idsToKeep = []
		firstInputLine = f.readline()
		split = firstInputLine.split(", ")
		for c in collumsToKeep:
			for index, i in enumerate(split):
				if i == c:
					idsToKeep.append(index)

		#create file and write first line
		firstLine = collumsToKeep[0]
		for name in collumsToKeep[1:]:
			firstLine = firstLine + ", " + name
		outputFile = open(tableOutput, "w+")
		outputFile.write(firstLine + "\n")

		#go trough all lines and write the values we want in the right order
		fiterator = iter(f)
		for x in fiterator:
			split = x.split(", ")
			line = split[idsToKeep[0]]
			for i in idsToKeep[1:]:
				line = line + ", " + split[i]
			outputFile.write(line + "\n")
		outputFile.close()
	else:
		print("problem reading file")
else:
	print("problem finding file")



