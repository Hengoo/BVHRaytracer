import matplotlib.pyplot as plt
import numpy as np

inputFolder = "../Data/CacheData/"
outputFolder = "../Plots/CachePlots/"

showImage = True

def endPlot():
	if showImage:
		plt.show()
	else:
		plt.close()

def perRayCacheMiss(bvhConfig, workSize):

	cacheSizes = [512, 256, 128, 64, 32]
	cacheSizes = np.array(cacheSizes)

	doStack = False

	fig = plt.figure(figsize=(13,11))
	plt.subplots_adjust(hspace = 0.4)
	plt.suptitle("Cache behavior per ray (" + str(workSize) + " * " + str(workSize) + " Work groups)")
	
	for iteration, cacheSize in enumerate(cacheSizes):
		filePath = inputFolder + "WorkGroupSize_" + str(workSize) + "_Normal/"+ "amazonLumberyardInterior_PerRayCache_Cache" + str(cacheSize) + bvhConfig + ".txt"

		(rayId, stackCacheLoads, stackCacheMiss, heapCacheLoads,
			heapCacheMiss, secondaryStackCacheLoads, secondaryStackCacheMiss,
			secondaryHeapCacheLoads, secondaryHeapCacheMiss) = np.loadtxt(filePath, delimiter=',', unpack=True, skiprows=1)

		if iteration == 0:
			ax = plt.subplot(5, 2, 1 + iteration * 2)
			primaryRef = ax
		else:
			ax = plt.subplot(5, 2, 1 + iteration * 2, sharex = primaryRef, sharey = primaryRef)
		
		plt.title("Primary Ray, Cache size " + str(cacheSize))
		if doStack:	
			plt.plot(rayId, stackCacheMiss, label="stack Miss")
			plt.plot(rayId, stackCacheLoads, label="stack loads")


		plt.plot(rayId, heapCacheMiss, label="heap Miss")
		plt.plot(rayId, heapCacheLoads, label="heap loads")

		plt.xticks(np.arange(0, 256 + 16, step=16))
		ax.tick_params(axis='both', which='major', labelsize=8)
		plt.ylabel("Cache line misses")
		if (iteration == len(cacheSizes) -1):
			plt.xlabel("Ray Id")
		#plt.legend()
		ax.set_ylim(ymin=0)

		if iteration == 0:
			ax = plt.subplot(5, 2, 2 + iteration * 2)
			secondaryRef = ax
		else:
			ax = plt.subplot(5, 2, 2 + iteration * 2, sharex = secondaryRef, sharey = secondaryRef)

		ax = plt.subplot(5, 2, 2 + iteration * 2)
		plt.title("Secondary Ray, Cache size " + str(cacheSize))
		
		if doStack:
			plt.plot(rayId, secondaryStackCacheMiss, label="stack Miss")
			plt.plot(rayId, secondaryStackCacheLoads, label="stack loads")

		
		plt.plot(rayId, secondaryHeapCacheMiss, label="heap Miss")
		plt.plot(rayId, secondaryHeapCacheLoads, label="heap loads")

		plt.xticks(np.arange(0, 256 + 16, step=16))
		ax.tick_params(axis='both', which='major', labelsize=8)
		#plt.ylabel("Cache Misses")
		if (iteration == len(cacheSizes) -1):
			plt.xlabel("Ray Id")
		#plt.legend()
		ax.set_ylim(ymin=0)
		
	
	handles, labels = ax.get_legend_handles_labels()
	fig.legend(handles, labels, ncol=4, loc='upper center', bbox_to_anchor=(0.5, 0.95))	

	plt.savefig(outputFolder + "PerRayCache_s" + str(workSize) + bvhConfig + ".pgf")
	plt.savefig(outputFolder + "PerRayCache_s" + str(workSize) + bvhConfig + ".pdf")

	endPlot()

	# do hitrate?

def differentCachesizeAnalysis(workSize):


	cacheSizes = [8, 16, 32, 64, 128, 256, 512]
	cacheSizes = np.array(cacheSizes)

	l = 4
	nodeSizes = [4, 8, 12, 16]

	fig = plt.figure(figsize=(13,11))
	#figure.tight_layout(False)
	plt.suptitle("Cache hit rate with different Cache sizes and Node sizes. (" + str(workSize) + " * " + str(workSize) + " Work groups)")
	plt.subplots_adjust(hspace = 0.4)

	for iteration, n in enumerate(nodeSizes):
		l = n
		n = 4
		configString = "_b" + str(n) + "_l" + str(l) + "_mb" + str(n) + "_ml" + str(l)
		titleConfigString = "N" + str(n) + "L" + str(l)
		stackMisses0, stackMisses1, stackLoads0, stackLoads1, stackHitRate0, stackHitRate1 = (np.zeros(0) for i in range(6))
		heapMisses0, heapMisses1, heapLoads0, heapLoads1, heapHitRate0, heapHitRate1 = (np.zeros(0) for i in range(6))
		stackSecondaryMisses0, stackSecondaryMisses1, stackSecondaryLoads0, stackSecondaryLoads1, stackSecondaryHitRate0, stackSecondaryHitRate1 = (np.zeros(0) for i in range(6))
		heapSecondaryMisses0, heapSecondaryMisses1, heapSecondaryLoads0, heapSecondaryLoads1, heapSecondaryHitRate0, heapSecondaryHitRate1 = (np.zeros(0) for i in range(6))

		for i in range(len(cacheSizes)):
			cacheSize = cacheSizes[i]
			filePath0 = inputFolder + "WorkGroupSize_" + str(workSize) + "_Normal/amazonLumberyardInterior_PerWorkgroupCache_Cache" + str(cacheSize) + configString + ".txt"
			filePath1 = inputFolder + "WorkGroupSize_" + str(workSize) + "_Wide/amazonLumberyardInterior_PerWorkgroupCache_Cache" + str(cacheSize) + configString + ".txt"

			#filePath0 = inputFolder + "WorkGroupSize_" + str(workSize) + "_Normal_NoRayPad/amazonLumberyardInterior_PerWorkgroupCache_Cache" + str(cacheSize) + configString + ".txt"
			#filePath1 = inputFolder + "WorkGroupSize_" + str(workSize) + "_Wide_NoRayPad/amazonLumberyardInterior_PerWorkgroupCache_Cache" + str(cacheSize) + configString + ".txt"

			(workGroupId, stackCacheLoads, stackCacheMiss, heapCacheLoads, heapCacheMiss,
				secondaryStackCacheLoads, secondaryStackCacheMiss, secondaryHeapCacheLoads,
				secondaryHeapCacheMiss) = np.loadtxt(filePath0, delimiter=',', unpack=True, skiprows=1)

			stackLoads0 = np.append(stackLoads0, np.average(stackCacheLoads))
			stackMisses0 = np.append(stackMisses0, np.average(stackCacheMiss))
			heapLoads0 = np.append(heapLoads0, np.average(heapCacheLoads))
			heapMisses0 = np.append(heapMisses0, np.average(heapCacheMiss))

			stackSecondaryLoads0 = np.append(stackSecondaryLoads0, np.average(secondaryStackCacheLoads))
			stackSecondaryMisses0 = np.append(stackSecondaryMisses0, np.average(secondaryStackCacheMiss))
			heapSecondaryLoads0 = np.append(heapSecondaryLoads0, np.average(secondaryHeapCacheLoads))
			heapSecondaryMisses0 = np.append(heapSecondaryMisses0, np.average(secondaryHeapCacheMiss))

			(workGroupId, stackCacheLoads, stackCacheMiss, heapCacheLoads, heapCacheMiss,
				secondaryStackCacheLoads, secondaryStackCacheMiss, secondaryHeapCacheLoads,
				secondaryHeapCacheMiss) = np.loadtxt(filePath1, delimiter=',', unpack=True, skiprows=1)

			print("cache size " + str(cacheSizes[i]) + " nodeSize" + str(n) + " misses: "+ str(np.average(stackCacheMiss)))

			stackLoads1 = np.append(stackLoads1, np.average(stackCacheLoads))
			stackMisses1 = np.append(stackMisses1, np.average(stackCacheMiss))
			heapLoads1 = np.append(heapLoads1, np.average(heapCacheLoads))
			heapMisses1 = np.append(heapMisses1, np.average(heapCacheMiss))

			stackSecondaryLoads1 = np.append(stackSecondaryLoads1, np.average(secondaryStackCacheLoads))
			stackSecondaryMisses1 = np.append(stackSecondaryMisses1, np.average(secondaryStackCacheMiss))
			heapSecondaryLoads1 = np.append(heapSecondaryLoads1, np.average(secondaryHeapCacheLoads))
			heapSecondaryMisses1 = np.append(heapSecondaryMisses1, np.average(secondaryHeapCacheMiss))

		stackHitRate0 = 1 - stackMisses0 / stackLoads0
		heapHitRate0 = 1 - heapMisses0 / heapLoads0
		stackHitRate1 = 1 - stackMisses1 / stackLoads1
		heapHitRate1 = 1 - heapMisses1 / heapLoads1

		stackSecondaryHitRate0 = 1 - stackSecondaryMisses0 / stackSecondaryLoads0
		heapSecondaryHitRate0 = 1 - heapSecondaryMisses0 / heapSecondaryLoads0
		stackSecondaryHitRate1 = 1 - stackSecondaryMisses1 / stackSecondaryLoads1
		heapSecondaryHitRate1 = 1 - heapSecondaryMisses1 / heapSecondaryLoads1

		xAxis = np.arange(len(cacheSizes)) * 5
		#Cache Hitrate

		ax = plt.subplot(4,2,1 + iteration * 2)
		plt.title("Primary rays " + titleConfigString)
		
		#horizontal line at 0 and 1
		plt.axhline(y = 1, linewidth=0.5, color='0.6')
		plt.axhline(y = 0, linewidth=0.5, color='0.6')
		plt.plot(xAxis, heapHitRate1, label="Wide V1 heap")
		plt.plot(xAxis, stackHitRate1, label="Wide V1 stack")
		plt.plot(xAxis, heapHitRate0, label="Single ray Traversal Heap")
		plt.plot(xAxis, stackHitRate0, label="Single ray Traversal Stack")	

		plt.xticks(xAxis, (8, 16, 32, 64, 128, 256, 512))
		plt.ylabel("Cache line hit rate")
		if (iteration == len(nodeSizes) -1):
			plt.xlabel("Cache Size per Thread [cache lines]")
		#plt.legend()

		ax = plt.subplot(4,2,2 + iteration * 2)
		plt.title("Secondary rays " + titleConfigString)
		#horizontal line at 0 and 1
		plt.axhline(y = 1, linewidth=0.5, color='0.6')
		plt.axhline(y = 0, linewidth=0.5, color='0.6')
		plt.plot(xAxis, heapSecondaryHitRate1, label="Wide V1 heap")
		plt.plot(xAxis, stackSecondaryHitRate1, label="Wide V1 stack")
		plt.plot(xAxis, heapSecondaryHitRate0, label="Single ray Traversal Heap")
		plt.plot(xAxis, stackSecondaryHitRate0, label="Single ray Traversal Stack")	

		plt.xticks(xAxis, (8, 16, 32, 64, 128, 256, 512))
		#plt.ylabel("cache hit rate")
		if (iteration == len(nodeSizes) -1):
			plt.xlabel("Cache Size per Thread [cache lines]")
		#plt.legend()


	handles, labels = ax.get_legend_handles_labels()
	fig.legend(handles, labels, ncol=4, loc='lower center', bbox_to_anchor=(0.5, 0.91))	

	plt.savefig(outputFolder + "CacheHitrate_s" + str(workSize) + ".pdf")
	plt.savefig(outputFolder + "CacheHitrate_s" + str(workSize) + ".pgf")
	endPlot()

	#Cache Misses
	#plt.axhline(y = np.average(stackLoads0 + heapLoads0) ,linewidth=1, color='0.3', label = "Single ray Loads")
	#plt.bar(xAxis - width * 0.5, stackMisses0 + heapMisses0 , width=width, label="Single ray Traversal")
	#plt.bar(xAxis + width * 0.5, +stackMisses1 + heapMisses1, width=width, label="Wide Traversal")
	#plt.axhline(y = np.average(stackLoads1 + heapLoads1) ,linewidth=1, color='0.8', label = "Wide Loads")
#
	#plt.ylabel("Cache Misses / Loads")
	#plt.xlabel("Cache Size per Thread [cache lines]")
	#plt.legend()
#
	#plt.savefig(outputFolder + "CacheMisses.pdf")
	#plt.savefig(outputFolder + "CacheMisses.pgf")
	#endPlot()



#perRayCacheMiss("_b4_l4_mb4_ml4", 16)
#perRayCacheMiss("_b16_l16_mb16_ml16", 16)

differentCachesizeAnalysis(16)
