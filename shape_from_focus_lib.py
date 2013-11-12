#Import various libaries
import cv2
import numpy as np
import sort
import shutil
import os

#This is the image stack class
class image_stack:
	current_depth = 0

	#Initalization function
	def __init__(self, img_dir):
		#Move this into a function which also checks if all the files have the same dimension sizes
		image_files = os.listdir(img_dir)
		sort.sort_nicely(image_files)
		
		#Get image size of first file and assume as dimensions of all files
		img = cv2.imread(img_dir + image_files.pop(0), 0)
		self.height, self.width = img.shape

		self.depth = len(image_files)
		self.stretch_factor = 255 / self.depth - 1
		
		#Create depth map and image stack arrays
		self.depth_map = np.zeros((self.height, self.width), dtype=np.float64)
		self.image = np.zeros((self.depth, self.height, self.width), dtype=np.float64)
		
		#Add the files
		for file in image_files:
				print('Adding ' + file)
				self.add(img_dir + file)

	#This function is used to add an image to the stack
	def add(self, img_file):

		#open the image file
		img = cv2.imread(img_file, 0)
		img = img.astype(dtype=np.float64)

		#find the focus using laplacian operator, this needs to be improved
		img = cv2.Laplacian(img, -1, ksize = 1)
		
		#copy image to array
		self.image[self.current_depth] = img
		self.current_depth += 1
		
#Function for determining the focus maximum of a pixel
def detect_focus_peak(stack, y, x):
#possible create a focus structure 
	focus_peak = 0
	focus_depth = 0
	focus_value = 0
	grad_prev = 0
	grad = 0
	
	for depth in xrange(0, stack.depth - 1):
		grad = stack.image[depth + 1, y, x] - stack.image[depth, y, x]
		#Check for peak focus
		if( grad == 0 or (grad < 0 and grad_prev > 0)):
			#Look for the peak with heighest focus
			if(stack.image[depth, y, x] > focus_value):
				focus_depth = depth
				focus_value = stack.image[depth, y, x]
	return focus_depth * stack.stretch_factor

#Function for generating a depth map from a a focus image stack class
def create_depth_map(stack):
	for x in xrange(0, stack.width):
		for y in xrange(0, stack.height):
			#print(str(x) + "." + str(y))
			stack.depth_map[y,x] = detect_focus_peak(stack, y, x)
