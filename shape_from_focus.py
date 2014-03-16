#!/usr/bin/python

#import library of functions
import cv2
from matplotlib import pyplot as plt
from matplotlib import cm
import os
import time
import thread
import sort
import sys
import shutil
import subprocess
from depth_map_methods_py import *

#Parse the input arguments
#Add this latter

#main section
img_dir = sys.argv[1]
output = 'output/'

image_files = [image_file for image_file in os.listdir(img_dir) if image_file.endswith('.pgm')]
sort.sort_nicely(image_files)
img = cv2.imread(img_dir + image_files[0], 0)
height, width = img.shape

#Create a new image stack
print('Creating image stack with dimensions', height, width)
stack = Pyimage_stack(height, width, len(image_files), 1000, img_dir + output)

#Add image files
start = time.time()
for image_file in image_files:
    print "Adding " + image_file
    stack.add(img_dir + image_file)
finish = time.time()
print 'Execution time =',finish-start

#Generate Depth map
start = time.time()
print('Generating Depth map...')
stack.create_depth_map()
finish = time.time()
print 'Execution time =',finish-start