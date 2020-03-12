import matplotlib.pyplot as plt
import numpy as np

inputFolder = "../Data/CacheData/"
inputFolder2 = "../Data/CacheHeapOnly/"
inputFolder3 = "../Data/CacheStackOnly/"
outputFolder = "../Plots/CachePlots/"

showImage = False

def endPlot():
	if showImage:
		plt.show()
	else:
		plt.close()

def perRayCacheMiss(bvhConfig, workSize, yMaxPrim, yMaxSec):

	cacheSizes = [512, 256, 128, 64, 32]
	cacheSizes = np.array(cacheSizes)

	doStack = False

	fig = plt.figure(figsize=(13,11))
	plt.subplots_adjust(hspace = 0.4, wspace= 0.15)
	#plt.suptitle("Cache behavior per ray (" + str(workSize) + " * " + str(workSize) + " Work groups)")
	
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
			plt.plot(rayId, stackCacheLoads, label="stack loads")
			plt.plot(rayId, stackCacheMiss, label="stack Miss")


		plt.plot(rayId, heapCacheLoads, label="heap loads")
		plt.plot(rayId, heapCacheMiss, label="heap Miss")

		plt.xticks(np.arange(0, 256 + 16, step=16))
		ax.tick_params(axis='both', which='major', labelsize=8)
		plt.ylabel("\# Cache lines")
		if (iteration == len(cacheSizes) -1):
			plt.xlabel("Ray Id")
		#plt.legend()
		ax.set_ylim(ymin=0, ymax = yMaxPrim)

		if iteration == 0:
			ax = plt.subplot(5, 2, 2 + iteration * 2)
			secondaryRef = ax
		else:
			ax = plt.subplot(5, 2, 2 + iteration * 2, sharex = secondaryRef, sharey = secondaryRef)

		ax = plt.subplot(5, 2, 2 + iteration * 2)
		plt.title("Secondary Ray, Cache size " + str(cacheSize))
		
		if doStack:
			plt.plot(rayId, secondaryStackCacheLoads, label="stack Loads")
			plt.plot(rayId, secondaryStackCacheMiss, label="stack Miss")

		
		plt.plot(rayId, secondaryHeapCacheLoads, label="BVH cache Loads")
		plt.plot(rayId, secondaryHeapCacheMiss, label="BVH cache Misses")

		plt.xticks(np.arange(0, 256 + 16, step=16))
		ax.tick_params(axis='both', which='major', labelsize=8)
		#plt.ylabel("Cache Misses")
		if (iteration == len(cacheSizes) -1):
			plt.xlabel("Ray Id")
		#plt.legend()
		ax.set_ylim(ymin=0, ymax = yMaxSec)
		
	
	handles, labels = ax.get_legend_handles_labels()
	lgd = ax.legend(handles, labels, ncol=4, loc='upper center', bbox_to_anchor=(-0.1, 7.2))	

	plt.savefig(outputFolder + "PerRayCache_s" + str(workSize) + bvhConfig + ".pgf", bbox_extra_artists=(lgd,), bbox_inches='tight')
	plt.savefig(outputFolder + "PerRayCache_s" + str(workSize) + bvhConfig + ".pdf", bbox_extra_artists=(lgd,), bbox_inches='tight')

	endPlot()

	# do hitrate?

def differentCachesizeAnalysis(workSize):


	cacheSizes = [8, 16, 32, 64, 128, 256, 512]
	cacheSizes = np.array(cacheSizes)

	l = 4
	nodeSizes = [4, 8, 12, 16]

	fig = plt.figure(figsize=(13,11))
	#plt.suptitle("Cache hit rate with different Cache sizes and Node sizes. (" + str(workSize) + " * " + str(workSize) + " Work groups)")
	plt.subplots_adjust(hspace = 0.4, wspace = 0.15)

	for iteration, n in enumerate(nodeSizes):
		l = n
		configString = "_b" + str(n) + "_l" + str(l) + "_mb" + str(n) + "_ml" + str(l)
		titleConfigString = "N" + str(n) + "L" + str(l)
		stackMisses0, stackMisses1, stackLoads0, stackLoads1, stackHitRate0, stackHitRate1= (np.zeros(0) for i in range(6))
		heapMisses0, heapMisses1, heapLoads0, heapLoads1, heapHitRate0, heapHitRate1= (np.zeros(0) for i in range(6))
		stackSecondaryMisses0, stackSecondaryMisses1, stackSecondaryLoads0, stackSecondaryLoads1, stackSecondaryHitRate0, stackSecondaryHitRate1= (np.zeros(0) for i in range(6))
		heapSecondaryMisses0, heapSecondaryMisses1, heapSecondaryLoads0, heapSecondaryLoads1, heapSecondaryHitRate0, heapSecondaryHitRate1= (np.zeros(0) for i in range(6))

		for i in range(len(cacheSizes)):
			cacheSize = cacheSizes[i]
			filePath0 = inputFolder + "WorkGroupSize_" + str(workSize) + "_Normal/amazonLumberyardInterior_PerWorkgroupCache_Cache" + str(cacheSize) + configString + ".txt"
			filePath1 = inputFolder + "WorkGroupSize_" + str(workSize) + "_Wide/amazonLumberyardInterior_PerWorkgroupCache_Cache" + str(cacheSize) + configString + ".txt"
			#filePath2 = inputFolder2 + "WorkGroupSize_" + str(workSize) + "_Wide/amazonLumberyardInterior_PerWorkgroupCache_Cache" + str(cacheSize) + configString + ".txt"

			#filePath0 = inputFolder + "WorkGroupSize_" + str(workSize) + "_Normal_NoRayPad/amazonLumberyardInterior_PerWorkgroupCache_Cache" + str(cacheSize) + configString + ".txt"
			#filePath1 = inputFolder + "WorkGroupSize_" + str(workSize) + "_Wide_NoRayPad/amazonLumberyardInterior_PerWorkgroupCache_Cache" + str(cacheSize) + configString + ".txt"

			#single ray:
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

			#wide ray
			(workGroupId, stackCacheLoads, stackCacheMiss, heapCacheLoads, heapCacheMiss,
				secondaryStackCacheLoads, secondaryStackCacheMiss, secondaryHeapCacheLoads,
				secondaryHeapCacheMiss) = np.loadtxt(filePath1, delimiter=',', unpack=True, skiprows=1)

			#print("cache size " + str(cacheSizes[i]) + " nodeSize" + str(n) + " misses: "+ str(np.average(stackCacheMiss)))

			stackLoads1 = np.append(stackLoads1, np.average(stackCacheLoads))
			stackMisses1 = np.append(stackMisses1, np.average(stackCacheMiss))
			heapLoads1 = np.append(heapLoads1, np.average(heapCacheLoads))
			heapMisses1 = np.append(heapMisses1, np.average(heapCacheMiss))

			stackSecondaryLoads1 = np.append(stackSecondaryLoads1, np.average(secondaryStackCacheLoads))
			stackSecondaryMisses1 = np.append(stackSecondaryMisses1, np.average(secondaryStackCacheMiss))
			heapSecondaryLoads1 = np.append(heapSecondaryLoads1, np.average(secondaryHeapCacheLoads))
			heapSecondaryMisses1 = np.append(heapSecondaryMisses1, np.average(secondaryHeapCacheMiss))

		stackHitRate0 = (1 - stackMisses0 / stackLoads0) * 100
		heapHitRate0 = (1 - heapMisses0 / heapLoads0) * 100
		stackHitRate1 = (1 - stackMisses1 / stackLoads1) * 100
		heapHitRate1 = (1 - heapMisses1 / heapLoads1) * 100
 
		stackSecondaryHitRate0 = (1 - stackSecondaryMisses0 / stackSecondaryLoads0) * 100
		heapSecondaryHitRate0 = (1 - heapSecondaryMisses0 / heapSecondaryLoads0) * 100
		stackSecondaryHitRate1 = (1 - stackSecondaryMisses1 / stackSecondaryLoads1) * 100
		heapSecondaryHitRate1 = (1 - heapSecondaryMisses1 / heapSecondaryLoads1) * 100

		xAxis = np.arange(len(cacheSizes)) * 5
		#Cache Hitrate

		ax = plt.subplot(4,2,1 + iteration * 2)
		plt.title("Primary rays " + titleConfigString)
		
		#horizontal line at 0 and 1
		plt.axhline(y = 100, linewidth=0.5, color='0.6')
		plt.axhline(y = 0, linewidth=0.5, color='0.6')
		plt.plot(xAxis, heapHitRate1, label="Wide V1 heap")
		plt.plot(xAxis, stackHitRate1, label="Wide V1 stack")
		plt.plot(xAxis, heapHitRate0, label="Single ray Traversal Heap")

		print(np.average(stackHitRate0))

		plt.xticks(xAxis, (8, 16, 32, 64, 128, 256, 512))
		plt.ylabel("Cache line hit rate[%]\nmore is better $\\triangleright$")
		if (iteration == len(nodeSizes) -1):
			plt.xlabel("Cache Size per Thread [cache lines]")
		#plt.legend()

		ax = plt.subplot(4,2,2 + iteration * 2)
		plt.title("Secondary rays " + titleConfigString)
		#horizontal line at 0 and 1
		plt.axhline(y = 100, linewidth=0.5, color='0.6')
		plt.axhline(y = 0, linewidth=0.5, color='0.6')
		plt.plot(xAxis, heapSecondaryHitRate1, label="Wide traversal BVH")
		plt.plot(xAxis, stackSecondaryHitRate1, label="Wide traversal memory overhead")
		plt.plot(xAxis, heapSecondaryHitRate0, label="Single ray Traversal BVH")

		plt.xticks(xAxis, (8, 16, 32, 64, 128, 256, 512))
		#plt.ylabel("cache hit rate")
		if (iteration == len(nodeSizes) -1):
			plt.xlabel("Cache Size per Thread [cache lines]")
		#plt.legend()


	handles, labels = ax.get_legend_handles_labels()
	lgd = ax.legend(handles, labels, ncol=2, loc='lower center', bbox_to_anchor=(-0.1, 5.5))

	plt.savefig(outputFolder + "CacheHitrate_s" + str(workSize) + ".pdf", bbox_extra_artists=(lgd,), bbox_inches='tight')
	plt.savefig(outputFolder + "CacheHitrate_s" + str(workSize) + ".pgf", bbox_extra_artists=(lgd,), bbox_inches='tight')
	endPlot()

def stackMissrate(workSize):
	#separated stack only thing

	cacheSizes = [8, 16, 32, 64, 128, 256, 512]
	cacheSizes = np.array(cacheSizes)

	l = 4
	nodeSizes = [4]

	fig = plt.figure(figsize=(13,3))
	#plt.suptitle("Cache hit rate with different Cache sizes and Node sizes. (" + str(workSize) + " * " + str(workSize) + " Work groups)")
	plt.subplots_adjust(hspace = 0.4, wspace = 0.15)

	for iteration, n in enumerate(nodeSizes):
		#l = n
		configString = "_b" + str(n) + "_l" + str(l) + "_mb" + str(n) + "_ml" + str(l)
		titleConfigString = "N" + str(n) + "L" + str(l)
		stackMisses3, stackLoads3, stackHitRate3 = (np.zeros(0) for i in range(3))
		stackSecondaryMisses3, stackSecondaryLoads3, stackSecondaryHitRate3 = (np.zeros(0) for i in range(3))

		for i in range(len(cacheSizes)):
			cacheSize = cacheSizes[i]
			filePath3 = inputFolder3 + "WorkGroupSize_" + str(workSize) + "_Wide/amazonLumberyardInterior_PerWorkgroupCache_Cache" + str(cacheSize) + configString + ".txt"

			#wide special:
			(workGroupId, stackCacheLoads, stackCacheMiss, heapCacheLoads, heapCacheMiss,
				secondaryStackCacheLoads, secondaryStackCacheMiss, secondaryHeapCacheLoads,
				secondaryHeapCacheMiss) = np.loadtxt(filePath3, delimiter=',', unpack=True, skiprows=1)
			stackLoads3 = np.append(stackLoads3, np.average(stackCacheLoads))
			stackMisses3 = np.append(stackMisses3, np.average(stackCacheMiss))
			#heapLoads3 = np.append(heapLoads3, np.average(heapCacheLoads))
			#heapMisses3 = np.append(heapMisses3, np.average(heapCacheMiss))

			stackSecondaryLoads3 = np.append(stackSecondaryLoads3, np.average(secondaryStackCacheLoads))
			stackSecondaryMisses3 = np.append(stackSecondaryMisses3, np.average(secondaryStackCacheMiss))
			#heapSecondaryLoads3 = np.append(heapSecondaryLoads3, np.average(secondaryHeapCacheLoads))
			#heapSecondaryMisses3 = np.append(heapSecondaryMisses3, np.average(secondaryHeapCacheMiss))

		stackHitRate3 = (1 - stackMisses3 / stackLoads3) * 100

		stackSecondaryHitRate3 = (1 - stackSecondaryMisses3 / stackSecondaryLoads3) * 100


		xAxis = np.arange(len(cacheSizes)) * 5
		#Cache Hitrate

		#ax = plt.subplot(4,2,1 + iteration * 2)
		ax = plt.subplot(1,2,1)
		#plt.title("Primary rays " + titleConfigString)
		
		#horizontal line at 0 and 1
		plt.axhline(y = 100, linewidth=0.5, color='0.6')
		plt.axhline(y = 0, linewidth=0.5, color='0.6')

		plt.plot(xAxis, stackHitRate3, label="Wide traversal separated memory overhead")
		
		plt.xticks(xAxis, (8, 16, 32, 64, 128, 256, 512))
		plt.title("Primary rays")
		plt.ylabel("Cache line hit rate[%]\nmore is better $\\triangleright$")
		plt.xlabel("Cache Size per Thread [cache lines]")
		plt.legend(loc = "lower right")

		#ax = plt.subplot(4,2,2 + iteration * 2)
		ax = plt.subplot(1,2,2)
		#plt.title("Secondary rays " + titleConfigString)
		#horizontal line at 0 and 1
		plt.axhline(y=100, linewidth=0.5, color='0.6')
		plt.axhline(y = 0, linewidth=0.5, color='0.6')
		#plt.plot(xAxis, heapSecondaryLoads1, label="Wide heap")

		plt.plot(xAxis, stackSecondaryHitRate3, label= "Wide traversal separated memory overhead")
		plt.xticks(xAxis, (8, 16, 32, 64, 128, 256, 512))
		plt.title("Secondary rays")
		plt.xlabel("Cache Size per Thread [cache lines]")
		plt.legend(loc = "lower right")


	#handles, labels = ax.get_legend_handles_labels()
	#lgd = ax.legend(handles, labels, ncol=2, loc='lower center', bbox_to_anchor=(-0.1, 5.5))

	plt.savefig(outputFolder + "StackHitRate_s" + str(workSize) + ".pdf", bbox_inches='tight')
	plt.savefig(outputFolder + "StackHitRate_s" + str(workSize) + ".pgf", bbox_inches='tight')
	endPlot()
	
def differentCachesizeAnalysis2(workSize):


	cacheSizes = [8, 16, 32, 64, 128, 256, 512]
	cacheSizes = np.array(cacheSizes)

	l = 4
	nodeSizes = [4, 8, 12, 16]

	fig = plt.figure(figsize=(13,11))
	#plt.suptitle("Cache hit rate with different Cache sizes and Node sizes. (" + str(workSize) + " * " + str(workSize) + " Work groups)")
	plt.subplots_adjust(hspace = 0.4, wspace = 0.15)

	for iteration, n in enumerate(nodeSizes):
		l = n
		configString = "_b" + str(n) + "_l" + str(l) + "_mb" + str(n) + "_ml" + str(l)
		titleConfigString = "N" + str(n) + "L" + str(l)
		stackMisses0, stackMisses1, stackLoads0, stackLoads1, stackHitRate0, stackHitRate1= (np.zeros(0) for i in range(6))
		heapMisses0, heapMisses1, heapLoads0, heapLoads1, heapHitRate0, heapHitRate1= (np.zeros(0) for i in range(6))
		stackSecondaryMisses0, stackSecondaryMisses1, stackSecondaryLoads0, stackSecondaryLoads1, stackSecondaryHitRate0, stackSecondaryHitRate1= (np.zeros(0) for i in range(6))
		heapSecondaryMisses0, heapSecondaryMisses1, heapSecondaryLoads0, heapSecondaryLoads1, heapSecondaryHitRate0, heapSecondaryHitRate1= (np.zeros(0) for i in range(6))

		for i in range(len(cacheSizes)):
			cacheSize = cacheSizes[i]
			filePath0 = inputFolder + "WorkGroupSize_" + str(workSize) + "_Normal/amazonLumberyardInterior_PerWorkgroupCache_Cache" + str(cacheSize) + configString + ".txt"
			filePath1 = inputFolder + "WorkGroupSize_" + str(workSize) + "_Wide/amazonLumberyardInterior_PerWorkgroupCache_Cache" + str(cacheSize) + configString + ".txt"

			#single ray:
			(workGroupId, stackCacheLoads, stackCacheMiss, heapCacheLoads, heapCacheMiss,
				secondaryStackCacheLoads, secondaryStackCacheMiss, secondaryHeapCacheLoads,
				secondaryHeapCacheMiss) = np.loadtxt(filePath0, delimiter=',', unpack=True, skiprows=1)

			stackLoads0 = np.append(stackLoads0, np.average(stackCacheLoads / (workSize * workSize)))
			stackMisses0 = np.append(stackMisses0, np.average(stackCacheMiss / (workSize * workSize)))
			heapLoads0 = np.append(heapLoads0, np.average(heapCacheLoads / (workSize * workSize)))
			heapMisses0 = np.append(heapMisses0, np.average(heapCacheMiss / (workSize * workSize)))

			stackSecondaryLoads0 = np.append(stackSecondaryLoads0, np.average(secondaryStackCacheLoads / (workSize * workSize)))
			stackSecondaryMisses0 = np.append(stackSecondaryMisses0, np.average(secondaryStackCacheMiss / (workSize * workSize)))
			heapSecondaryLoads0 = np.append(heapSecondaryLoads0, np.average(secondaryHeapCacheLoads / (workSize * workSize)))
			heapSecondaryMisses0 = np.append(heapSecondaryMisses0, np.average(secondaryHeapCacheMiss / (workSize * workSize)))

			#wide ray
			(workGroupId, stackCacheLoads, stackCacheMiss, heapCacheLoads, heapCacheMiss,
				secondaryStackCacheLoads, secondaryStackCacheMiss, secondaryHeapCacheLoads,
				secondaryHeapCacheMiss) = np.loadtxt(filePath1, delimiter=',', unpack=True, skiprows=1)


			stackLoads1 = np.append(stackLoads1, np.average(stackCacheLoads / (workSize * workSize)))
			stackMisses1 = np.append(stackMisses1, np.average(stackCacheMiss / (workSize * workSize)))
			heapLoads1 = np.append(heapLoads1, np.average(heapCacheLoads / (workSize * workSize)))
			heapMisses1 = np.append(heapMisses1, np.average(heapCacheMiss / (workSize * workSize)))

			stackSecondaryLoads1 = np.append(stackSecondaryLoads1, np.average(secondaryStackCacheLoads / (workSize * workSize)))
			stackSecondaryMisses1 = np.append(stackSecondaryMisses1, np.average(secondaryStackCacheMiss / (workSize * workSize)))
			heapSecondaryLoads1 = np.append(heapSecondaryLoads1, np.average(secondaryHeapCacheLoads / (workSize * workSize)))
			heapSecondaryMisses1 = np.append(heapSecondaryMisses1, np.average(secondaryHeapCacheMiss / (workSize * workSize)))

		xAxis = np.arange(len(cacheSizes)) * 5
		#Cache Hitrate

		ax = plt.subplot(4,2,1 + iteration * 2)
		plt.title("Primary rays " + titleConfigString)
		
		#horizontal line at 0 and 1
		plt.axhline(y = 0, linewidth=0.5, color='0.6')
		plt.plot(xAxis, heapMisses1 + stackMisses1)
		plt.plot(xAxis, heapMisses0 + stackMisses0)

		plt.xticks(xAxis, (8, 16, 32, 64, 128, 256, 512))
		plt.ylabel("\# Cache line misses\n$\\triangleleft$ less is better")
		if (iteration == len(nodeSizes) -1):
			plt.xlabel("Cache Size per Thread [cache lines]")
		#plt.legend()

		ax = plt.subplot(4,2,2 + iteration * 2)
		plt.title("Secondary rays " + titleConfigString)
		#horizontal line at 0 and 1
		plt.axhline(y = 0, linewidth=0.5, color='0.6')
		#plt.plot(xAxis, heapSecondaryLoads1, label="Wide heap")

		plt.plot(xAxis, heapSecondaryMisses1 + stackSecondaryMisses1, label= "\# Wide traversal")
		plt.plot(xAxis, heapSecondaryMisses0 + stackSecondaryMisses0, label= "\# Single ray traversal")
		plt.xticks(xAxis, (8, 16, 32, 64, 128, 256, 512))
		#plt.ylabel("cache hit rate")
		if (iteration == len(nodeSizes) -1):
			plt.xlabel("Cache Size per Thread [cache lines]")
		#plt.legend()


	handles, labels = ax.get_legend_handles_labels()
	lgd = ax.legend(handles, labels, ncol=2, loc='lower center', bbox_to_anchor=(-0.1, 5.5))

	plt.savefig(outputFolder + "CacheLoads" + str(workSize) + ".pdf", bbox_extra_artists=(lgd,), bbox_inches='tight')
	plt.savefig(outputFolder + "CacheLoads" + str(workSize) + ".pgf", bbox_extra_artists=(lgd,), bbox_inches='tight')
	endPlot()

def differentCachesizeAnalysis3(workSize):


	cacheSizes = [8, 16, 32, 64, 128, 256, 512]
	cacheSizes = np.array(cacheSizes)

	l = 4
	nodeSizes = [4, 8, 12, 16]

	fig = plt.figure(figsize=(13,11))
	#plt.suptitle("Cache hit rate with different Cache sizes and Node sizes. (" + str(workSize) + " * " + str(workSize) + " Work groups)")
	plt.subplots_adjust(hspace = 0.4, wspace = 0.15)

	for iteration, n in enumerate(nodeSizes):
		#l = n
		configString = "_b" + str(n) + "_l" + str(l) + "_mb" + str(n) + "_ml" + str(l)
		titleConfigString = "N" + str(n) + "L" + str(l)
		stackMisses0, stackMisses1,stackMisses2, stackLoads0, stackLoads1,stackLoads2, stackHitRate0, stackHitRate1, stackHitRate2 = (np.zeros(0) for i in range(9))
		heapMisses0, heapMisses1, heapMisses2, heapLoads0, heapLoads1, heapLoads2, heapHitRate0, heapHitRate1, heapHitRate2 = (np.zeros(0) for i in range(9))
		stackSecondaryMisses0, stackSecondaryMisses1, stackSecondaryMisses2, stackSecondaryLoads0, stackSecondaryLoads1, stackSecondaryLoads2, stackSecondaryHitRate0, stackSecondaryHitRate1, stackSecondaryHitRate2 = (np.zeros(0) for i in range(9))
		heapSecondaryMisses0, heapSecondaryMisses1, heapSecondaryMisses2, heapSecondaryLoads0, heapSecondaryLoads1, heapSecondaryLoads2, heapSecondaryHitRate0, heapSecondaryHitRate1, heapSecondaryHitRate2 = (np.zeros(0) for i in range(9))

		stackMisses3, stackLoads3, stackHitRate3 = (np.zeros(0) for i in range(3))
		stackSecondaryMisses3, stackSecondaryLoads3, stackSecondaryHitRate3 = (np.zeros(0) for i in range(3))

		for i in range(len(cacheSizes)):
			cacheSize = cacheSizes[i]
			filePath0 = inputFolder + "WorkGroupSize_" + str(workSize) + "_Normal/amazonLumberyardInterior_PerWorkgroupCache_Cache" + str(cacheSize) + configString + ".txt"
			filePath1 = inputFolder + "WorkGroupSize_" + str(workSize) + "_Wide/amazonLumberyardInterior_PerWorkgroupCache_Cache" + str(cacheSize) + configString + ".txt"
			filePath2 = inputFolder2 + "WorkGroupSize_" + str(workSize) + "_Wide/amazonLumberyardInterior_PerWorkgroupCache_Cache" + str(cacheSize) + configString + ".txt"
			filePath3 = inputFolder3 + "WorkGroupSize_" + str(workSize) + "_Wide/amazonLumberyardInterior_PerWorkgroupCache_Cache" + str(cacheSize) + configString + ".txt"

			#filePath0 = inputFolder + "WorkGroupSize_" + str(workSize) + "_Normal_NoRayPad/amazonLumberyardInterior_PerWorkgroupCache_Cache" + str(cacheSize) + configString + ".txt"
			#filePath1 = inputFolder + "WorkGroupSize_" + str(workSize) + "_Wide_NoRayPad/amazonLumberyardInterior_PerWorkgroupCache_Cache" + str(cacheSize) + configString + ".txt"

			#single ray:
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

			#wide ray
			(workGroupId, stackCacheLoads, stackCacheMiss, heapCacheLoads, heapCacheMiss,
				secondaryStackCacheLoads, secondaryStackCacheMiss, secondaryHeapCacheLoads,
				secondaryHeapCacheMiss) = np.loadtxt(filePath1, delimiter=',', unpack=True, skiprows=1)

			#print("cache size " + str(cacheSizes[i]) + " nodeSize" + str(n) + " misses: "+ str(np.average(stackCacheMiss)))

			stackLoads1 = np.append(stackLoads1, np.average(stackCacheLoads))
			stackMisses1 = np.append(stackMisses1, np.average(stackCacheMiss))
			heapLoads1 = np.append(heapLoads1, np.average(heapCacheLoads))
			heapMisses1 = np.append(heapMisses1, np.average(heapCacheMiss))

			stackSecondaryLoads1 = np.append(stackSecondaryLoads1, np.average(secondaryStackCacheLoads))
			stackSecondaryMisses1 = np.append(stackSecondaryMisses1, np.average(secondaryStackCacheMiss))
			heapSecondaryLoads1 = np.append(heapSecondaryLoads1, np.average(secondaryHeapCacheLoads))
			heapSecondaryMisses1 = np.append(heapSecondaryMisses1, np.average(secondaryHeapCacheMiss))

			#heap only:
			(workGroupId, stackCacheLoads, stackCacheMiss, heapCacheLoads, heapCacheMiss,
				secondaryStackCacheLoads, secondaryStackCacheMiss, secondaryHeapCacheLoads,
				secondaryHeapCacheMiss) = np.loadtxt(filePath2, delimiter=',', unpack=True, skiprows=1)
			#stackLoads2 = np.append(stackLoads2, np.average(stackCacheLoads))
			#stackMisses2 = np.append(stackMisses2, np.average(stackCacheMiss))
			heapLoads2 = np.append(heapLoads2, np.average(heapCacheLoads))
			heapMisses2 = np.append(heapMisses2, np.average(heapCacheMiss))

			stackSecondaryLoads2 = np.append(stackSecondaryLoads2, np.average(secondaryStackCacheLoads))
			stackSecondaryMisses2 = np.append(stackSecondaryMisses2, np.average(secondaryStackCacheMiss))
			heapSecondaryLoads2 = np.append(heapSecondaryLoads2, np.average(secondaryHeapCacheLoads))
			heapSecondaryMisses2 = np.append(heapSecondaryMisses2, np.average(secondaryHeapCacheMiss))

			#stack only
			(workGroupId, stackCacheLoads, stackCacheMiss, heapCacheLoads, heapCacheMiss,
				secondaryStackCacheLoads, secondaryStackCacheMiss, secondaryHeapCacheLoads,
				secondaryHeapCacheMiss) = np.loadtxt(filePath3, delimiter=',', unpack=True, skiprows=1)

			stackLoads3 = np.append(stackLoads3, np.average(stackCacheLoads))
			stackMisses3 = np.append(stackMisses3, np.average(stackCacheMiss))

			stackSecondaryLoads3 = np.append(stackSecondaryLoads3, np.average(secondaryStackCacheLoads))
			stackSecondaryMisses3 = np.append(stackSecondaryMisses3, np.average(secondaryStackCacheMiss))

		xAxis = np.arange(len(cacheSizes)) * 5
		#Cache Hitrate

		ax = plt.subplot(4,2,1 + iteration * 2)
		plt.title("Primary rays " + titleConfigString)
		
		#horizontal line at 0 and 1
		plt.axhline(y = 100, linewidth=0.5, color='0.6')
		plt.axhline(y = 0, linewidth=0.5, color='0.6')
		
		plt.plot(xAxis, stackMisses1)
		plt.plot(xAxis, heapMisses1)
		#plt.plot(xAxis, heapMisses1 + stackMisses1)
		#plt.plot(xAxis, heapMisses2[1] + stackMisses3)
		#plt.plot(xAxis, heapMisses2 + stackMisses3[1])
		

		plt.xticks(xAxis, (8, 16, 32, 64, 128, 256, 512))
		plt.ylabel("\# Cache line misses\n$\\triangleleft$ less is better")
		if (iteration == len(nodeSizes) -1):
			plt.xlabel("Cache Size per Thread [cache lines]")
		#plt.legend()

		ax = plt.subplot(4,2,2 + iteration * 2)
		plt.title("Secondary rays " + titleConfigString)
		#horizontal line at 0 and 1
		plt.axhline(y=100, linewidth=0.5, color='0.6')
		plt.axhline(y = 0, linewidth=0.5, color='0.6')
		#plt.plot(xAxis, heapSecondaryLoads1, label="Wide heap")

		
		plt.plot(xAxis, stackSecondaryMisses1, label="\# Wide traversal memory overhead")
		plt.plot(xAxis, heapSecondaryMisses1, label="\# Wide BVH data")
		#plt.plot(xAxis, heapSecondaryMisses1 + stackSecondaryMisses1, label="\# Wide traversal")
		#plt.plot(xAxis, heapSecondaryMisses2[1] + stackSecondaryMisses3, label="\# Wide traversal Separated cache. BVH cache limited to 16 cachelines")
		#plt.plot(xAxis, heapSecondaryMisses2 + stackSecondaryMisses3[1], label="\# Wide traversal Separated cache. Memory overhead cache limited 16 cachelines ")

		plt.xticks(xAxis, (8, 16, 32, 64, 128, 256, 512))
		#plt.ylabel("cache hit rate")
		if (iteration == len(nodeSizes) -1):
			plt.xlabel("Cache Size per Thread [cache lines]")
		#plt.legend()


	handles, labels = ax.get_legend_handles_labels()
	lgd = ax.legend(handles, labels, loc='lower center', bbox_to_anchor=(-0.1, 5.5))

	plt.savefig(outputFolder + "CacheLoadsSpecial" + str(workSize) + ".pdf", bbox_extra_artists=(lgd,), bbox_inches='tight')
	plt.savefig(outputFolder + "CacheLoadsSpecial" + str(workSize) + ".pgf", bbox_extra_artists=(lgd,), bbox_inches='tight')
	endPlot()

def differentCachesizeAnalysis4(workSize):


	cacheSizes = [8, 16, 32, 64, 128]#, 256, 512]
	cacheSizes = np.array(cacheSizes)

	l = 4
	nodeSizes = [4, 8, 12, 16]

	fig = plt.figure(figsize=(13,13))
	#plt.suptitle("Cache hit rate with different Cache sizes and Node sizes. (" + str(workSize) + " * " + str(workSize) + " Work groups)")
	plt.subplots_adjust(hspace = 0.4, wspace = 0.15)

	for iteration in range(len(cacheSizes)):
		stackMisses0, stackMisses1, stackLoads0, stackLoads1, stackHitRate0, stackHitRate1= (np.zeros(0) for i in range(6))
		heapMisses0, heapMisses1, heapLoads0, heapLoads1, heapHitRate0, heapHitRate1= (np.zeros(0) for i in range(6))
		stackSecondaryMisses0, stackSecondaryMisses1, stackSecondaryLoads0, stackSecondaryLoads1, stackSecondaryHitRate0, stackSecondaryHitRate1= (np.zeros(0) for i in range(6))
		heapSecondaryMisses0, heapSecondaryMisses1, heapSecondaryLoads0, heapSecondaryLoads1, heapSecondaryHitRate0, heapSecondaryHitRate1= (np.zeros(0) for i in range(6))

		cacheSize = cacheSizes[iteration]
		titleConfigString = "Cache size " + str(cacheSize)
		for n in nodeSizes:
			l = n
			configString = "_b" + str(n) + "_l" + str(l) + "_mb" + str(n) + "_ml" + str(l)
			
			filePath0 = inputFolder + "WorkGroupSize_" + str(workSize) + "_Normal/amazonLumberyardInterior_PerWorkgroupCache_Cache" + str(cacheSize) + configString + ".txt"
			filePath1 = inputFolder + "WorkGroupSize_" + str(workSize) + "_Wide/amazonLumberyardInterior_PerWorkgroupCache_Cache" + str(cacheSize) + configString + ".txt"

			#single ray:
			(workGroupId, stackCacheLoads, stackCacheMiss, heapCacheLoads, heapCacheMiss,
				secondaryStackCacheLoads, secondaryStackCacheMiss, secondaryHeapCacheLoads,
				secondaryHeapCacheMiss) = np.loadtxt(filePath0, delimiter=',', unpack=True, skiprows=1)

			stackLoads0 = np.append(stackLoads0, np.average(stackCacheLoads / (workSize * workSize)))
			stackMisses0 = np.append(stackMisses0, np.average(stackCacheMiss / (workSize * workSize)))
			heapLoads0 = np.append(heapLoads0, np.average(heapCacheLoads / (workSize * workSize)))
			heapMisses0 = np.append(heapMisses0, np.average(heapCacheMiss / (workSize * workSize)))

			stackSecondaryLoads0 = np.append(stackSecondaryLoads0, np.average(secondaryStackCacheLoads / (workSize * workSize)))
			stackSecondaryMisses0 = np.append(stackSecondaryMisses0, np.average(secondaryStackCacheMiss / (workSize * workSize)))
			heapSecondaryLoads0 = np.append(heapSecondaryLoads0, np.average(secondaryHeapCacheLoads / (workSize * workSize)))
			heapSecondaryMisses0 = np.append(heapSecondaryMisses0, np.average(secondaryHeapCacheMiss / (workSize * workSize)))

			#wide ray
			(workGroupId, stackCacheLoads, stackCacheMiss, heapCacheLoads, heapCacheMiss,
				secondaryStackCacheLoads, secondaryStackCacheMiss, secondaryHeapCacheLoads,
				secondaryHeapCacheMiss) = np.loadtxt(filePath1, delimiter=',', unpack=True, skiprows=1)


			stackLoads1 = np.append(stackLoads1, np.average(stackCacheLoads / (workSize * workSize)))
			stackMisses1 = np.append(stackMisses1, np.average(stackCacheMiss / (workSize * workSize)))
			heapLoads1 = np.append(heapLoads1, np.average(heapCacheLoads / (workSize * workSize)))
			heapMisses1 = np.append(heapMisses1, np.average(heapCacheMiss / (workSize * workSize)))

			stackSecondaryLoads1 = np.append(stackSecondaryLoads1, np.average(secondaryStackCacheLoads / (workSize * workSize)))
			stackSecondaryMisses1 = np.append(stackSecondaryMisses1, np.average(secondaryStackCacheMiss / (workSize * workSize)))
			heapSecondaryLoads1 = np.append(heapSecondaryLoads1, np.average(secondaryHeapCacheLoads / (workSize * workSize)))
			heapSecondaryMisses1 = np.append(heapSecondaryMisses1, np.average(secondaryHeapCacheMiss / (workSize * workSize)))

		xAxis = nodeSizes
		#Cache Hitrate

		ax = plt.subplot(5,2,1 + iteration * 2)
		plt.title("Primary rays, " + titleConfigString)
		
		#horizontal line at 0 and 1
		plt.axhline(y = 0, linewidth=0.5, color='0.6')
		plt.plot(xAxis, heapMisses1 + stackMisses1)
		plt.plot(xAxis, heapMisses0 + stackMisses0)

		plt.xticks(xAxis, (4, 8, 12, 16,))
		plt.ylabel("\# Cache line misses\n$\\triangleleft$ less is better")
		if (iteration == len(cacheSizes) -1):
			plt.xlabel("Node and Leaf size")
		#plt.legend()

		ax = plt.subplot(5,2,2 + iteration * 2)
		plt.title("Secondary rays, " + titleConfigString)
		#horizontal line at 0 and 1
		plt.axhline(y = 0, linewidth=0.5, color='0.6')

		plt.plot(xAxis, heapSecondaryMisses1 + stackSecondaryMisses1, label= "\# Wide traversal")
		plt.plot(xAxis, heapSecondaryMisses0 + stackSecondaryMisses0, label= "\# Single ray traversal")
		plt.xticks(xAxis, (4, 8, 12, 16,))
		#plt.ylabel("cache hit rate")
		if (iteration == len(cacheSizes) -1):
			plt.xlabel("Node and Leaf size")
		#plt.legend()


	handles, labels = ax.get_legend_handles_labels()
	lgd = ax.legend(handles, labels, ncol=2, loc='lower center', bbox_to_anchor=(-0.1, 6.8))

	plt.savefig(outputFolder + "CacheLoadsPerNode" + str(workSize) + ".pdf", bbox_extra_artists=(lgd,), bbox_inches='tight')
	plt.savefig(outputFolder + "CacheLoadsPerNode" + str(workSize) + ".pgf", bbox_extra_artists=(lgd,), bbox_inches='tight')
	endPlot()

#perRayCacheMiss("_b4_l4_mb4_ml4", 16, 155, 90)
#perRayCacheMiss("_b16_l16_mb16_ml16", 16, 300, 150)

differentCachesizeAnalysis(16)
differentCachesizeAnalysis(8)
differentCachesizeAnalysis(32)
#
differentCachesizeAnalysis2(8)
differentCachesizeAnalysis2(16)
differentCachesizeAnalysis2(32)
#differentCachesizeAnalysis3(16)

differentCachesizeAnalysis4(8)
differentCachesizeAnalysis4(16)
differentCachesizeAnalysis4(32)


#stackMissrate(16)