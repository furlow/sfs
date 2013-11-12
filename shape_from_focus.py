#!/usr/bin/python

#import library of functions
from shape_from_focus_lib import *
import cv2
from matplotlib import pyplot as plt
import os
import time
import sort

#main section

img_dir = 'images/stack/'

#list of image files and depth
image_files = os.listdir(img_dir)
sort.sort_nicely(image_files)
stack_depth = len(image_files)

#Create a new image stack
stack = image_stack(stack_depth, 302, 302)

#Add all the images in the folder to the stack
for file in image_files:
	print(file)
	stack.add(img_dir + file)

#for position in range(0, stack.depth):
#	plt.imshow(stack.image[position])
#	plt.show()

#Generate Depth map
start = time.time()

create_depth_map(stack)

finish = time.time()

print 'Execution time =',finish-start

cv2.imwrite( img_dir + 'out.jpg', stack.depth_map)
