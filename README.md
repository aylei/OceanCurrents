# OceanCurrents README

##Build
use cmake to build it.

This project depends on flowing libaries:

- GLEW -1.13.0
- GLFW -3.1.2
- GLM -0.9.7.1
- assimp -3.0.1270

I add texture file, obj file and thirdparty libary to .ignore due to their large size, so you have to collect those libaries yourself in ./external/**, and use your own model&texture to feed the program.

##Then...
this project is still in progress, if you have any suggestions or questions
just post an issue or mailto:rayingecho@hotmail.com

##Pseudocode

###OLIC
'''python
# 1. init context
size = WITDH * HEIGHT 
dropletsTexture = generateRandDropletImg(rate=0.05, dimPixel=3, width=WIDTH, height=HEIGHT)
relateDroplets = list(size)  		# store related droplet for each pixel

# 2. pre-calculation
for pixel in outputTextureRange:
	


'''




