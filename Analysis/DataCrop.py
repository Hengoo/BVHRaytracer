import os.path
import math
from os import path
#short python script to crop a table to the intresting collums

def work(folder, tableInput, tableOutput, collumsToKeep):
	#open table
	if (path.exists(folder + tableInput)):
		f = open(folder + tableInput, "r")
		if f.mode == 'r':

			#get ids of collums to keep from the first line.
			idsToKeep = []
			firstInputLine = f.readline()
			firstInputLine = firstInputLine.strip('\n')
			split = firstInputLine.split(", ")
			for c in collumsToKeep:
				for index, i in enumerate(split):
					if i == c:
						idsToKeep.append(index)

			#create file and write first line
			firstLine = collumsToKeep[0]
			for name in collumsToKeep[1:]:
				firstLine = firstLine + ", " + name
			outputFile = open(folder + tableOutput, "w+")
			outputFile.write(firstLine + "\n")

			#go trough all lines and write the values we want in the right order
			fiterator = iter(f)
			for x in fiterator:
				x = x.strip('\n')
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


work("Summary/", "amazonLumberyardInteriorTable_Normal.txt",   "rayTotalTime_Normal.txt", ["branchFactor", "leafSize","totalTime"])
work("Summary/", "amazonLumberyardInteriorTable_Wide.txt", "rayTotalTime_Wide.txt", ["branchFactor", "leafSize","totalTime"])
