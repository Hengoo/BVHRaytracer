## Analysis of different Bounding Volume Hierarchies for Raytracing

### Getting it to run:

Needed libary:

	-GLM in ../../Libraries/glm
	(or change visual studio setting if you dont like that location)

Needed models:

	-For now downloadable here: https://www.dropbox.com/s/ri3azxcga87dt5q/RaytracerModels.zip?dl=0
	-unzip them to /models

Its planned to have some sort of modelConfig file to add new models without changing the source code.

### Settings:

The render and analysis settings can be changed in config.txt
	
### Analysis:
Analysis results are saved in /Analysis/\<sceneName\>

The python file "DataManager.py" in /Analysis can gather the needed numbers from the folders.

For now you have to edit the python code to choose what scene to summerize
-> the resulting \<sceneName\>.txt is used in the Latex Project. (normal save folder for the data: /LatexPlots/Data )



TODO: more here
