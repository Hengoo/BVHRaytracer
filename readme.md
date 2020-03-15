## Analysis of different Bounding Volume Hierarchies for Raytracing

This is the implementaion part of my master thesis : [Thesis](http://share.splamy.de/hengo/20/03/AndreasLeitnerThesis.pdf)

### To get it rumning:

Needed libary to compile:

	-GLM in ../../Libraries/glm
	(or change visual studio setting if you dont like that location)
	
	-ISPC compiler needs to be installed (exe has to be in path)

Needed models:

	-Can be downloaded from here: [Dropbox Link](https://www.dropbox.com/s/gjjnz189hsuhnfr/MasterThesisModels.zip?dl=0)
	-unzip them to /models

### Settings:

The render and analysis settings can be changed in config.txt

The default config renders Bistro Interior with performance tests and instrumented renderer for N4L4 to N16L16 with steps of 4

The results are stored in /Analysis/Results/\<sceneName\> 

The evaluation of the data is done with the python scripts in \Analysis
