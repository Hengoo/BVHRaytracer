## Analysis of Different Bounding VolumeHierarchies and Ray Tracing Algorithms

This repository contains the the implementation part of my master thesis : [Thesis](http://share.splamy.de/hengo/20/03/AndreasLeitnerThesis.pdf)

Short Summary:

Analysis of effects of the effects of different BVH configurations like node size and leaf size on ray tracing. SIMD is used to compute the node and leaf intersections efficiently. We discuss two different approaches to traversing the BVH and evaluate how they are are effected by the different BVH configurations. The general goal is to find an algorithm and BVH configuration that is well suited to be implemented as function hardware on the GPU. The evaluation includes performance benchmarks and a cache simulator.

### Building and example:

Needed library to compile:

	-GLM in ../../Libraries/glm
	(or change visual studio setting if you dont like that location)
	
	-ISPC compiler needs to be installed (.exe has to be in path)

	-Python and Jinja2 is required.

Needed models:

	-Can be downloaded from here: [Dropbox Link](https://www.dropbox.com/s/gjjnz189hsuhnfr/MasterThesisModels.zip?dl=0)
	-unzip them to /models

### Settings:

The render and analysis settings can be changed in config.txt

The default config renders Bistro Interior with performance tests and instrumented renderer for N4L4 to N16L16 with steps of 4

The results are stored in /Analysis/Results/\<sceneName\> 

The evaluation of the data is done with the python scripts in \Analysis
