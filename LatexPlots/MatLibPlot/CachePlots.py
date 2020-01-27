import matplotlib.pyplot as plt
import numpy as np

inputFolder = "../Data/CachePlot/"
outputFolder = "../Plots/"

showImage = True

def endPlot():
	if showImage:
		plt.show()
	else:
		plt.cla()

def perRayCacheMiss(cacheSize, bvhConfig):

	doHeap = False
	filePath = inputFolder + "PerRayCache/"+ "amazonLumberyardInteriorPerRayCache_Cachesize" + str(cacheSize) + bvhConfig + ".txt"

	(rayId, stackCacheLoads, stackCacheMiss, heapCacheLoads,
		heapCacheMiss, secondaryStackCacheLoads, secondaryStackCacheMiss,
		secondaryHeapCacheLoads, secondaryHeapCacheMiss) =  np.loadtxt(filePath, delimiter=',', unpack=True, skiprows=1)

	plt.suptitle("Cache behavior per ray, Cachesize " + str(cacheSize))
	plt.subplot(2,1,1)
	
	plt.plot(rayId, stackCacheMiss, label="stack Miss")
	plt.plot(rayId, stackCacheLoads, label="stack loads")

	if doHeap:
		plt.plot(rayId, heapCacheMiss, label="heap Miss")
		plt.plot(rayId, heapCacheLoads, label="heap loads")

	plt.xticks(np.arange(0, 256 + 16, step=16))
	plt.ylabel("Cache Misses")
	plt.xlabel("Ray Id")
	plt.legend()


	plt.subplot(2,1,2)
	plt.plot(rayId, secondaryStackCacheMiss, label="stack Miss")
	plt.plot(rayId, secondaryStackCacheLoads, label="stack loads")

	if doHeap:
		plt.plot(rayId, secondaryHeapCacheMiss, label="heap Miss")
		plt.plot(rayId, secondaryHeapCacheLoads, label="heap loads")

	plt.xticks(np.arange(0, 256 + 16, step=16))
	plt.ylabel("Cache Misses")
	plt.xlabel("Ray Id")
	plt.legend()

	plt.savefig(outputFolder + "PerRayCache" + str(cacheSize) + bvhConfig + ".pgf")
	plt.savefig(outputFolder + "PerRayCache" + str(cacheSize) + bvhConfig + ".pdf")
	endPlot()

	# do hitrate?

def differentCachesizeAnalysis():
	#cacheSizes = [8, 16, 32, 64, 128, 256, 512]
	cacheSizes = [128, 256]
	cacheSizes = np.array(cacheSizes)

	misses0 = []
	misses1 = []
	misses2 = []
	hitRate0 = []
	hitRate1 = []
	hitRate2 = []
	for i in range(len(cacheSizes)):
		cacheSize = cacheSizes[i]
		filePath0 = inputFolder + "WorkGroupNormal/amazonLumberyardInteriorPerWorkgroupCacheMiss_Cachesize" + str(cacheSize) + "_c1.txt"
		filePath1 = inputFolder + "WorkGroupWide0/amazonLumberyardInteriorPerWorkgroupCacheMiss_Cachesize" + str(cacheSize) + "_c1.txt"
		filePath2 = inputFolder + "WorkGroupWide1/amazonLumberyardInteriorPerWorkgroupCacheMiss_Cachesize" + str(cacheSize) + "_c1.txt"

		(workgroupId, load0, hit0) = np.loadtxt(filePath0, delimiter=',', unpack=True, skiprows=1)
		(workgroupId, load1, hit1) = np.loadtxt(filePath1, delimiter=',', unpack=True, skiprows=1)
		(workgroupId, load2, hit2) = np.loadtxt(filePath2, delimiter=',', unpack=True, skiprows=1)

		misses0.append(np.sum(load0 - hit0) / len(workgroupId))
		misses1.append(np.sum(load1 - hit1) / len(workgroupId))
		misses2.append(np.sum(load2 - hit2) / len(workgroupId))

		hitRate0.append(np.sum(hit0 / load0) / len(workgroupId))
		hitRate1.append(np.sum(hit1 / load1) / len(workgroupId))
		hitRate2.append(np.sum(hit2 / load2) / len(workgroupId))

	plt.title("Cache misses with different Cache sizes.")

	xAxis = np.arange(len(cacheSizes)) * 5
	
	plt.xticks(xAxis, (8,16,32,64,128,256,512))
	width = 0.4

	#Cache Hitrate
	plt.title("Cache hit rate with different Cache sizes.")
	plt.xticks(xAxis, (8,16,32,64,128,256,512))
	plt.bar(xAxis - width, hitRate0, width=width, label="Single ray Traversal")
	plt.bar(xAxis, hitRate1, width=width, label="Wide V0")
	plt.bar(xAxis + width, hitRate2, width=width, label="Wide V1")

	plt.ylabel("cache hit rate")
	plt.xlabel("Cache Size per Thread [cache lines]")
	plt.legend()

	plt.savefig(outputFolder + "CacheHitrate.pdf")
	plt.savefig(outputFolder + "CacheHitrate.pgf")
	endPlot()

	#Cache Misses
	plt.bar(xAxis - width, misses0, width=width, label="Single ray Traversal")
	plt.bar(xAxis, misses1, width=width, label="Wide V0")
	plt.bar(xAxis + width, misses2, width=width, label="Wide V1")

	plt.ylabel("Cache Misses")
	plt.xlabel("Cache Size per Thread [cache lines]")
	plt.legend()

	plt.savefig(outputFolder + "CacheMisses.pdf")
	plt.savefig(outputFolder + "CacheMisses.pgf")
	endPlot()



perRayCacheMiss(512, "_b4_l4_mb4_ml4")
perRayCacheMiss(256, "_b4_l4_mb4_ml4")
perRayCacheMiss(128, "_b4_l4_mb4_ml4")
perRayCacheMiss(64, "_b4_l4_mb4_ml4")
#differentCachesizeAnalysis()