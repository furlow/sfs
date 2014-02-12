#!/usr/bin/python

#import library of functions
from shape_from_focus_lib import *
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
from depth_map_methods import *

#Parse the input arguments
#Add this latter

#main section
img_dir = sys.argv[1]
tmp_copy = 'tmp_copy/'
tmp_cropped = 'tmp_cropped/'
output = 'output/'

#Check if the folders have been created and images cropped #Need to improve this
if( not os.path.exists(img_dir + tmp_copy) and not os.path.exists(img_dir + tmp_cropped) and  not os.path.exists(img_dir + output) ):

	image_files = os.listdir(img_dir)
	sort.sort_nicely(image_files)

	print('Creating temporary directories...')
	os.mkdir(img_dir + tmp_copy)
	os.mkdir(img_dir + tmp_cropped)
	os.mkdir(img_dir + output)
	
	align_image_stack_cmd = ["align_image_stack", "-m", "-C", "-c", "4", "-a", img_dir + tmp_cropped,]
	
	for file in image_files:
		shutil.copyfile(img_dir + file, img_dir + tmp_copy + file)
		align_image_stack_cmd.append(img_dir + tmp_copy + file)
	
	#Align image files to remove focus magnification
	print('Aligning images...')
	subprocess.call(align_image_stack_cmd)

#Create a new image stack
print('Creating image stack...')
stack = image_stack(img_dir + tmp_cropped, cv2.CV_32F, np.float32)

#Generate Depth map
start = time.time()
print('Generating Depth map...')
create_depth_map(stack)
finish = time.time()
print 'Execution time =',finish-start

#write image out
cv2.imwrite( img_dir + output + 'out.jpg', stack.depth_map)

#Plot the image
ax = plt.gca()
fig = plt.gcf()
implot = ax.imshow(stack.depth_map, cmap = cm.Greys_r)

#plot the focus graph
fig2 = plt.figure()
ax2 = fig2.add_subplot(111)
ax2.set_ylim([0,1e9])
ax2.set_xlim([0,stack.depth - 1])
line1, = ax2.plot(np.zeros(stack.depth))

#This function draws a graph of the focus through the stack
#It updates when the user clicks the mouse on a pixel
def onclick(event):
    	if event.xdata != None and event.ydata != None:
	    	print(event.xdata, event.ydata)
	    	y = int(event.xdata)
	    	x = int(event.ydata)
	    	data = stack.get_focus_data(y, x)
	    	print(data)
	    	line1.set_ydata(data)
    		fig2.canvas.draw()

try:
	cid = fig.canvas.mpl_connect('button_press_event', onclick)
	plt.show() 
except KeyboardInterrupt:
	raise
