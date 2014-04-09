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
tmp_copy = 'tmp_copy/'
tmp_cropped = 'tmp_cropped/'
output = 'output/'

#Check if the folders have been created and images cropped #Need to improve this
if( not os.path.exists(img_dir + tmp_copy) and not os.path.exists(img_dir + tmp_cropped) and  not os.path.exists(img_dir + output) ):

	image_files = [image_file for image_file in os.listdir(img_dir) if image_file.endswith('.JPG')]
	sort.sort_nicely(image_files)

	print('Creating temporary directories...')
	os.mkdir(img_dir + tmp_copy)
	os.mkdir(img_dir + tmp_cropped)
	os.mkdir(img_dir + output)

	align_image_stack_cmd = ["/Applications/Hugin.app/Contents/MacOS/align_image_stack", "-m", "-C", "-c", "4", "-a", img_dir + tmp_cropped]
	#align_image_stack_cmd = ["align_image_stack", "-m", "-l", "-C", "-c", "4", "-a", img_dir + tmp_cropped]

	for file in image_files:
		shutil.copyfile(img_dir + file, img_dir + tmp_copy + file)
		align_image_stack_cmd.append(img_dir + tmp_copy + file)

	#Align image files to remove focus magnification
	print('Aligning images...')
	subprocess.call(align_image_stack_cmd)


image_files = [image_file for image_file in os.listdir(img_dir + tmp_cropped) if image_file.endswith('.tif')]
sort.sort_nicely(image_files)
img = cv2.imread(img_dir + tmp_cropped + image_files[0], 0)
height, width = img.shape

#Create a new image stack
print('Creating image stack with dimensions', height, width)
stack = Pyimage_stack(height, width, len(image_files), 1000, img_dir + output)

#Add image files
start = time.time()
for image_file in image_files:
    print "Adding " + image_file
    stack.add(img_dir + tmp_cropped + image_file)
finish = time.time()
print 'Execution time =',finish-start

#Generate Depth map
start = time.time()
print('Generating Depth map...')
stack.create_depth_map()
finish = time.time()
print 'Execution time =',finish-start

#Generate an all in focus image
start = time.time()
print('Generating fuse focus image')
stack.fuse_focus()
finish = time.time()
print 'Execution time =',finish-start

#Refocus the image
try:
	while(True):

		focus_depth = int(raw_input("Choose depth to refocus to: "))
		print('refoucsing the image')
		start = time.time()
		stack.refocus(0, focus_depth)
		finish = time.time()
		print 'Execution time =',finish-start
except(KeyboardInterrupt):
	raise
#write image out
#cv2.imwrite( img_dir + output + 'out.jpg', stack.depth_map)

#Plot the image
#ax = plt.gca()
#fig = plt.gcf()
#implot = ax.imshow(stack.depth_map, cmap = cm.Greys_r)

#plot the focus graph
#fig2 = plt.figure()
#ax2 = fig2.add_subplot(111)
#ax2.set_ylim([0,1e9])
#ax2.set_xlim([0,stack.depth - 1])
#line1, = ax2.plot(np.zeros(stack.depth))

#This function draws a graph of the focus through the stack
#It updates when the user clicks the mouse on a pixel
#def onclick(event):
	#if event.xdata != None and event.ydata != None:
		#print(event.xdata, event.ydata)
		#y = int(event.xdata)
		#x = int(event.ydata)
		#data = stack.get_focus_data(y, x)
		#print(data)
		#line1.set_ydata(data)
		#fig2.canvas.draw()
#try:
#	cid = fig.canvas.mpl_connect('button_press_event', onclick)
#	plt.show()
#except KeyboardInterrupt:
#	raise
