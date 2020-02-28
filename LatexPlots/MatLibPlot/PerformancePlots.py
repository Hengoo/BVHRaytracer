import matplotlib.pyplot as plt
import numpy as np

inputFolder = "../Data/"
outputFolder = "../Plots/PerformancePlots/"

showImage = True

def endPlot():
	if showImage:
		plt.show()
	else:
		plt.close()

def rayTotalAnalysis():
	def rayTotalAnalysisHelperComparison(ax, totalTimeV1, leaf, branch, leafSize, width):
		maskV1 = np.ma.masked_where(leaf != leafSize, totalTimeV1)
		maskBranch = np.ma.masked_where(leaf != leafSize, branch)
		maskV1 = maskV1.compressed()
		maskBranch = maskBranch.compressed()

		plt.xticks(np.arange(4, 20, step=4))
		plt.title("Leafsize " + str(leafSize))
		ax.bar(maskBranch, maskV1, width=width, bottom=1, label="Wide renderer")
		ax.axhline(linewidth=1, y = 1, color='0.5')
		plt.legend()

	def rayTotalAnalysisHelperOverview(ax, totalTime, totalTimeV1, leaf, branch, leafSize, width):
		mask = np.ma.masked_where(leaf != leafSize, totalTime)
		maskV1 = np.ma.masked_where(leaf != leafSize, totalTimeV1)
		maskBranch = np.ma.masked_where(leaf != leafSize, branch)
		mask = mask.compressed()
		maskV1 = maskV1.compressed()
		maskBranch = maskBranch.compressed()

		plt.xticks(np.arange(4, 20, step=4))
		plt.title("Leafsize " + str(leafSize))

		ax.bar(maskBranch - width / 2, maskV1, width=width, label="Wide ray")
		ax.bar(maskBranch + width / 2, mask, width=width, label="Single ray Traversal")
		
		#ax.axhline(linewidth=1, y = 0, color='0.5')
		plt.legend(loc = 'lower left')
	

	#overview of the 3 raytracer version, normal , wideV0 and wideV1 for N,L 4 to 16
	(branch, leaf, totalTime) = np.loadtxt(inputFolder + "rayTotalTime_Normal.txt" , delimiter=',', unpack=True, skiprows = 1)
	(branch, leaf, totalTimeV1) = np.loadtxt(inputFolder + "rayTotalTime_Wide.txt" , delimiter=',', unpack=True, skiprows = 1)

	width = 1

	#first do not normalized plot
	fig = plt.figure(figsize=(12, 8))
	fig.suptitle("General performance overview")

	ax0 = plt.subplot(2,2,1)
	rayTotalAnalysisHelperOverview(ax0, totalTime, totalTimeV1, leaf, branch, 4, width)
	plt.ylabel("Render time in seconds")

	ax1 = plt.subplot(2,2,2, sharex = ax0, sharey = ax0)
	rayTotalAnalysisHelperOverview(ax1, totalTime, totalTimeV1, leaf, branch, 8, width)

	ax2 = plt.subplot(2,2,3, sharex = ax0, sharey = ax0)
	rayTotalAnalysisHelperOverview(ax2, totalTime, totalTimeV1, leaf, branch, 12, width)
	plt.ylabel("Render time in seconds")
	plt.xlabel("Nodesize")

	ax3 = plt.subplot(2,2,4, sharex = ax0, sharey = ax0)
	rayTotalAnalysisHelperOverview(ax3, totalTime, totalTimeV1, leaf, branch, 16, width)
	plt.xlabel("Nodesize")


	plt.savefig(outputFolder + "PerformanceOverview.pdf")
	plt.savefig(outputFolder + "PerformanceOverview.pgf")

	#Now plot that is normalized by "smallest" (single ray traversal)
	fig = plt.figure(figsize=(12, 8))
	fig.suptitle("Performance comparison of single ray traversal to wide traversal")
	totalTimeV1 = (totalTimeV1 / totalTime) - 1

	ax0 = plt.subplot(2,2,1)
	plt.ylabel("time relative to single ray traversal")
	rayTotalAnalysisHelperComparison(ax0, totalTimeV1, leaf, branch, 4, width)

	ax1 = plt.subplot(2,2,2, sharex = ax0, sharey = ax0)
	rayTotalAnalysisHelperComparison(ax1, totalTimeV1, leaf, branch, 8, width)

	ax2 = plt.subplot(2,2,3, sharex = ax0, sharey = ax0)
	plt.xlabel("Nodesize")
	plt.ylabel("time relative to single ray traversal")
	rayTotalAnalysisHelperComparison(ax2, totalTimeV1, leaf, branch, 12, width)

	ax3 = plt.subplot(2,2,4, sharex = ax0, sharey = ax0)
	plt.xlabel("Nodesize")
	rayTotalAnalysisHelperComparison(ax3, totalTimeV1, leaf, branch, 16, width)

	plt.savefig(outputFolder + "PerformanceComparison.pdf")
	plt.savefig(outputFolder + "PerformanceComparison.pgf")
	endPlot()

rayTotalAnalysis()