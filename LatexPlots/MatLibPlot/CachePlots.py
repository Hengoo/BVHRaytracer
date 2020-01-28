import matplotlib.pyplot as plt
import numpy as np

inputFolder = "../Data/CachePlot/"
outputFolder = "../Plots/"

showImage = False

def endPlot():
	if showImage:
		plt.show()
	else:
		plt.close()

def perRayCacheMiss(cacheSize, bvhConfig):

	doStack = False
	filePath = inputFolder + "PerRayCache/"+ "amazonLumberyardInteriorPerRayCache_Cachesize" + str(cacheSize) + bvhConfig + ".txt"

	(rayId, stackCacheLoads, stackCacheMiss, heapCacheLoads,
		heapCacheMiss, secondaryStackCacheLoads, secondaryStackCacheMiss,
		secondaryHeapCacheLoads, secondaryHeapCacheMiss) =  np.loadtxt(filePath, delimiter=',', unpack=True, skiprows=1)

	plt.suptitle("Cache behavior per ray, Cachesize " + str(cacheSize))
	ax1 = plt.subplot(2,1,1)
	
	if doStack:	
		plt.plot(rayId, stackCacheMiss, label="stack Miss")
		plt.plot(rayId, stackCacheLoads, label="stack loads")


	plt.plot(rayId, heapCacheMiss, label="heap Miss")
	plt.plot(rayId, heapCacheLoads, label="heap loads")

	plt.xticks(np.arange(0, 256 + 16, step=16))
	plt.ylabel("Cache Misses")
	plt.xlabel("Ray Id")
	plt.legend()
	ax1.set_ylim(ymin=0, ymax = 160)


	ax2 = plt.subplot(2, 1, 2)
	
	if doStack:
		plt.plot(rayId, secondaryStackCacheMiss, label="stack Miss")
		plt.plot(rayId, secondaryStackCacheLoads, label="stack loads")

	
	plt.plot(rayId, secondaryHeapCacheMiss, label="heap Miss")
	plt.plot(rayId, secondaryHeapCacheLoads, label="heap loads")

	plt.xticks(np.arange(0, 256 + 16, step=16))
	plt.ylabel("Cache Misses")
	plt.xlabel("Ray Id")
	plt.legend()
	ax2.set_ylim(ymin=0, ymax = 90)
	plt.savefig(outputFolder + "PerRayCache" + str(cacheSize) + bvhConfig + ".pgf")
	plt.savefig(outputFolder + "PerRayCache" + str(cacheSize) + bvhConfig + ".pdf")
	endPlot()

	# do hitrate?

def differentCachesizeAnalysis(n,l):
	configString = "_b" + str(n) + "_l" + str(l) + "_mb" + str(n) + "_ml" + str(l)
	
	cacheSizes = [8, 16, 32, 64, 128, 256, 512]
	cacheSizes = np.array(cacheSizes)

	stackMisses0 = []
	stackMisses1 = []
	stackLoads0 = []
	stackLoads1 = []
	stackHitRate0 = []
	stackHitRate1 = []

	heapMisses0 = []
	heapMisses1 = []
	heapLoads0 = []
	heapLoads1 = []
	heapHitRate0 = []
	heapHitRate1 = []

	for i in range(len(cacheSizes)):
		cacheSize = cacheSizes[i]
		filePath0 = inputFolder + "WorkGroupNormal/amazonLumberyardInteriorPerWorkgroupCache_Cachesize" + str(cacheSize) + configString + ".txt"
		filePath1 = inputFolder + "WorkGroupWide1/amazonLumberyardInteriorPerWorkgroupCache_Cachesize" + str(cacheSize) + configString + ".txt"

		(workGroupId, stackCacheLoads, stackCacheMiss, heapCacheLoads, heapCacheMiss,
			secondaryStackCacheLoads, secondaryStackCacheMiss, secondaryHeapCacheLoads,
			secondaryHeapCacheMiss) = np.loadtxt(filePath0, delimiter=',', unpack=True, skiprows=1)


		stackLoads0.append(np.sum(stackCacheLoads) / len(workGroupId))
		stackMisses0.append(np.sum(stackCacheMiss) / len(workGroupId))
		heapLoads0.append(np.sum(heapCacheLoads) / len(workGroupId))
		heapMisses0.append(np.sum(heapCacheMiss) / len(workGroupId))

		(workGroupId, stackCacheLoads, stackCacheMiss, heapCacheLoads, heapCacheMiss,
			secondaryStackCacheLoads, secondaryStackCacheMiss, secondaryHeapCacheLoads,
			secondaryHeapCacheMiss) = np.loadtxt(filePath1, delimiter=',', unpack=True, skiprows=1)

		stackLoads1.append(np.sum(stackCacheLoads) / len(workGroupId))
		stackMisses1.append(np.sum(stackCacheMiss) / len(workGroupId))
		heapLoads1.append(np.sum(heapCacheLoads) / len(workGroupId))
		heapMisses1.append(np.sum(heapCacheMiss) / len(workGroupId))



	stackMisses0 = np.array(stackMisses0)
	stackMisses1 = np.array(stackMisses1)
	stackLoads0 = np.array(stackLoads0)
	stackLoads1 = np.array(stackLoads1)
	stackHitRate0 = np.array(stackHitRate0)
	stackHitRate1 = np.array(stackHitRate1)

	heapMisses0 = np.array(heapMisses0)
	heapMisses1 = np.array(heapMisses1)
	heapLoads0 = np.array(heapLoads0)
	heapLoads1 = np.array(heapLoads1)
	heapHitRate0 = np.array(heapHitRate0)
	heapHitRate1 = np.array(heapHitRate1)

	stackHitRate0 = 1 - stackMisses0 / stackLoads0
	heapHitRate0 = 1 - heapMisses0 / heapLoads0
	stackHitRate1 = 1 - stackMisses1 / stackLoads1
	heapHitRate1 = 1 - heapMisses1 / heapLoads1


	plt.title("Cache misses with different Cache sizes.")

	xAxis = np.arange(len(cacheSizes)) * 5
	
	plt.xticks(xAxis, (8,16,32,64,128,256,512))
	width = 0.4

	#Cache Hitrate
	plt.title("Cache hit rate with different Cache sizes.")
	plt.xticks(xAxis, (8, 16, 32, 64, 128, 256, 512))
	
	plt.axhline(y = 1,linewidth=1, color='0.5')
	plt.plot(xAxis, heapHitRate1, label="Wide V1 heap")
	plt.plot(xAxis, stackHitRate1, label="Wide V1 stack")
	plt.plot(xAxis, heapHitRate0, label="Single ray Traversal Heap")
	plt.plot(xAxis, stackHitRate0, label="Single ray Traversal Stack")	


	plt.ylabel("cache hit rate")
	plt.xlabel("Cache Size per Thread [cache lines]")
	plt.legend()

	plt.savefig(outputFolder + "CacheHitrate" + configString + ".pdf")
	plt.savefig(outputFolder + "CacheHitrate" + configString + ".pgf")
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



perRayCacheMiss(512, "_b4_l4_mb4_ml4")
perRayCacheMiss(256, "_b4_l4_mb4_ml4")
perRayCacheMiss(128, "_b4_l4_mb4_ml4")
perRayCacheMiss(64, "_b4_l4_mb4_ml4")
perRayCacheMiss(32, "_b4_l4_mb4_ml4")

differentCachesizeAnalysis(4, 4)
differentCachesizeAnalysis(8, 4)
differentCachesizeAnalysis(12, 4)
differentCachesizeAnalysis(16, 4)
