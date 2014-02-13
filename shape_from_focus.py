#!/usr/bin/python

#import library of functions
from shape_from_focus_lib import *
import cv2
from matplotlib import pyplot as plt
from matplotlib import cm
from mpl_toolkits.mplot3d import Axes3D
import os
import time
import thread
import sort
import sys
import shutil
import subprocess
from depth_map_methods import *


def shape_from_focus(img_dir, k_size):

	#Parse the input arguments
	#Add this latter

	print(img_dir)

	#main section
	tmp_copy = 'tmp_copy/'
	tmp_cropped = 'tmp_cropped/'
	output = 'output/'

	#Check if the folders have been created and images cropped #Need to improve this
	if( not os.path.exists(img_dir + tmp_copy) and not os.path.exists(img_dir + tmp_cropped) and  not os.path.exists(img_dir + output) ):
		 
		image_files = [ file for file in os.listdir(img_dir) if os.path.isfile(img_dir + file) ]

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
		start = time.time()
		subprocess.call(align_image_stack_cmd)
                finish = time.time()
                print 'Image alignment execution time =',finish-start

	step = 50

	#Create a new image stack
	print('Creating image stack...')
	stack = image_stack(img_dir + tmp_cropped, cv2.CV_32F, np.float32, k_size) 

	#Generate Depth map
	start = time.time()
	print('Generating Depth map...')
	create_depth_map(stack)
	finish = time.time()
	print 'Execution time =',finish-start

	#write image out
	cv2.imwrite( img_dir + output + "SML_" + str(k_size) + "_depth_map.tif", stack.depth_map)
	width = stack.width
	height = stack.height
	depth_map = stack.depth_map
	#else:
		#depth_map = cv2.imread(img_dir + output + 'out.jpg', 0)
		#height, width = depth_map.shape

	#Invert the depth map
	depth_map = cv2.bitwise_not(depth_map)
	
	#Generate 3D surfrace plot
	x_steps = width / step
	y_steps = height / step
	x_cordinates = np.arange(0, x_steps)
	y_cordinates = np.arange(0, y_steps)
	zz = np.empty([y_steps, x_steps])
	xx, yy = np.meshgrid(x_cordinates,y_cordinates)

	for x in x_cordinates:
		for y in y_cordinates:
			zz[y,x] = depth_map[y * step, x * step]

	fig = plt.figure()
	ax = fig.gca(projection = '3d')
	surf = ax.plot_surface(xx,yy,zz, rstride = 1, cstride = 1, cmap=cm.coolwarm, linewidth = 0, antialiased = False)
	ax.view_init(elev=70, azim=45)
	fig.colorbar(surf)
	plt.savefig(img_dir + output + "SML_" + str(k_size) + "surface_plot.png")

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
	#    	if event.xdata != None and event.ydata != None:
	#	    	print(event.xdata, event.ydata)
	#	    	y = int(event.xdata)
	#	    	x = int(event.ydata)
	#	    	data = stack.get_focus_data(y, x)
	#	    	print(data)
	#	    	line1.set_ydata(data)
	#   		fig2.canvas.draw()
	#
	#try:
	#	cid = fig.canvas.mpl_connect('button_press_event', onclick)
	#	plt.show() 
	#except KeyboardInterrupt:
	#	raise

shape_from_focus(sys.argv[1], 9)

