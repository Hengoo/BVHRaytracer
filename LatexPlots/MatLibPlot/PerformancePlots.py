import matplotlib.pyplot as plt
import numpy as np

inputFolder = "../Data/"
outputFolder = "../Plots/PerformancePlots/"

#"\n\$\\triangleleft$ less is better"
#"\nmore is better $\\triangleright$"

showImage = True

def endPlot():
	if showImage:
		plt.show()
	else:
		plt.close()

def hardCodedAnalysis():
	#plot of the sse vs avx plots for N2 - N8

	xAxis = np.array(range(2, 17))

	sse = np.array([4.83867, 3.62267, 3.24032, 3.77629, 3.53583, 3.36607, 3.25266, 3.87958, 3.71619, 3.58490, 3.47888, 3.86556, 3.78010, 3.70618, 3.68813])
	avx = np.array([4.91521, 3.61275, 3.15693, 2.89272, 2.70250, 2.56921, 2.46902, 3.10006, 2.99034, 2.88428, 2.75913, 2.72356, 2.69287, 2.66722, 2.62718])
	
	#fig = plt.figure(figsize=(12, 3.8))

	ax = plt.subplot(1,1,1)
	plt.plot(xAxis, sse, label="L4 SSE")
	plt.plot(xAxis, avx, label="L8 AVX")
	plt.ylabel('Render time [s]$\\triangleright$')
	plt.xlabel("Nodesize")
	plt.legend()
	ax.set_ylim(ymin= -0.2)

	plt.savefig(outputFolder + "SSEAVXComp.pdf", bbox_inches='tight')
	plt.savefig(outputFolder + "SSEAVXComp.pgf", bbox_inches='tight')
	endPlot()

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
	#fig.suptitle("General performance overview")
	plt.subplots_adjust(hspace = 0.4, wspace = 0.15)

	ax0 = plt.subplot(2,2,1)
	rayTotalAnalysisHelperOverview(ax0, totalTime, totalTimeV1, leaf, branch, 4, width)
	plt.ylabel("Render time [s]\n\$\\triangleleft$ less is better")
	plt.xlabel("Nodesize")

	ax1 = plt.subplot(2,2,2, sharex = ax0, sharey = ax0)
	rayTotalAnalysisHelperOverview(ax1, totalTime, totalTimeV1, leaf, branch, 8, width)
	plt.xlabel("Nodesize")

	ax2 = plt.subplot(2,2,3, sharex = ax0, sharey = ax0)
	rayTotalAnalysisHelperOverview(ax2, totalTime, totalTimeV1, leaf, branch, 12, width)
	plt.ylabel("Render time [s]\n\$\\triangleleft$ less is better")
	plt.xlabel("Nodesize")

	ax3 = plt.subplot(2,2,4, sharex = ax0, sharey = ax0)
	rayTotalAnalysisHelperOverview(ax3, totalTime, totalTimeV1, leaf, branch, 16, width)
	plt.xlabel("Nodesize")


	plt.savefig(outputFolder + "PerformanceOverview.pdf", bbox_inches='tight')
	plt.savefig(outputFolder + "PerformanceOverview.pgf", bbox_inches='tight')

	#Now plot that is normalized by "smallest" (single ray traversal)
	fig = plt.figure(figsize=(12, 8))
	fig.suptitle("Performance comparison of single ray traversal to wide traversal")
	plt.xlabel("Nodesize")
	totalTimeV1 = (totalTimeV1 / totalTime) - 1

	ax0 = plt.subplot(2, 2, 1)
	plt.xlabel("Nodesize")
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

	plt.savefig(outputFolder + "PerformanceComparison.pdf", bbox_inches='tight')
	plt.savefig(outputFolder + "PerformanceComparison.pgf", bbox_inches='tight')
	endPlot()

rayTotalAnalysis()
hardCodedAnalysis()