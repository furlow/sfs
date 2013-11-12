#!/usr/bin/python

#import library of functions
from shape_from_focus_lib import *
import cv2
from matplotlib import pyplot as plt
from matplotlib import cm
import os
import time
import sort
import subprocess

#main section
print('Creating temporary directories...')
img_dir = 'images/book/'
tmp_copy = 'tmp_copy/'
tmp_cropped = 'tmp_cropped/'
output = 'output/'
image_files = os.listdir(img_dir)
print(image_files)
sort.sort_nicely(image_files)
print(image_files)

#Check if the folders have been created and images cropped
if( not os.path.exists(img_dir + tmp_copy) and not os.path.exists(img_dir + tmp_cropped) and  not os.path.exists(img_dir + output) ):
	os.mkdir(img_dir + tmp_copy)
	os.mkdir(img_dir + tmp_cropped)
	os.mkdir(img_dir + output)
	
	align_image_stack_cmd = ["align_image_stack", "-v", "-m", "-C", "-a", img_dir + tmp_cropped,]
	
	for file in image_files:
		shutil.copyfile(img_dir + file, img_dir + tmp_copy + file)
		align_image_stack_cmd.append(img_dir + tmp_copy + file)
	
	#Align image files to remove focus magnification
	print('Aligning images...')
	subprocess.call(align_image_stack_cmd)

#Create a new image stack
print('Creating image stack...')
stack = image_stack(img_dir + tmp_cropped) #setup an image stack

#Display images
#for position in range(0, stack.depth):
#	plt.imshow(stack.image[position], cmap = cm.Greys_r)
#	plt.show()

#Generate Depth map, need to add time calculation 
start = time.time()
print('Generating Depth map...')
create_depth_map(stack)
finish = time.time()

print 'Execution time =',finish-start

cv2.imwrite( img_dir + output + 'out.jpg', stack.depth_map)
