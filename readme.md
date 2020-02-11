## Analysis of different Bounding Volume Hierarchies for Raytracing

### Getting it to run:

Needed libary:

	-GLM in ../../Libraries/glm
	(or change visual studio setting if you dont like that location)
	
	-ISPC compiler needs to be installed (exe has to be in path)

Needed models: (currently not up to date)

	-For now downloadable here: https://www.dropbox.com/s/ri3azxcga87dt5q/RaytracerModels.zip?dl=0
	-unzip them to /models

### Settings:

The render and analysis settings can be changed in config.txt
	
### Analysis:
Analysis results are saved in /Analysis/Results/\<sceneName\>

The python file "DataManager.py" in /Analysis can gather the data from different scenes into one big table.

For now you have to edit the python code to choose what scene to summerize