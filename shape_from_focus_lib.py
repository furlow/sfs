#Import various libaries
import cv2
import numpy as np
from matplotlib import pyplot as plt
from matplotlib import cm
import sort
import os

#This is the image stack class
class image_stack:
	current_depth = 0

	#Initalization function
	def __init__(self, img_dir, CV_DTYPE, NP_DTYPE, k_size):
	
		self.CV_DTYPE = CV_DTYPE
		self.NP_DTYPE = NP_DTYPE
		self.k_size = k_size
	
		#Move this into a function which also checks if all the files have the same dimension sizes
		image_files = os.listdir(img_dir)
		sort.sort_nicely(image_files)
		
		#Get image size of first file and assume as dimensions of all files
		img = cv2.imread(img_dir + image_files.pop(0), 0)
		self.height, self.width = img.shape

		self.depth = len(image_files)
		self.stretch_factor = 255 / self.depth - 1
		
		#Create depth map and image stack arrays
		self.depth_map = np.zeros((self.height, self.width), dtype=NP_DTYPE)
		self.image = np.zeros((self.depth, self.height, self.width), dtype=NP_DTYPE)
		
		#Add the files
		for file in image_files:
				print('Adding ' + file)
				self.add(img_dir + file)

	#This function is used to add an image to the stack
	def add(self, img_file):

		#open the image file
		img = cv2.imread(img_file, 0)
		img = img.astype(self.NP_DTYPE)

		#find the focus using laplacian operator, this needs to be improved
		#sobelx = cv2.Sobel(img, ddepth=cv2.CV_64F, dx=1, dy=0, ksize=11)
		#sobely = cv2.Sobel(img, ddepth=cv2.CV_64F, dx=0, dy=1, ksize=11)
		#img = sobelx * 0.5 + sobely * 0.5
		
		#Laplacian
		#img = cv2.Laplacian(img, ddepth = self.CV_DTYPE, ksize = self.ksize)
		
		#Sum Modified Laplacian

		ML_x, ML_y = ML(self.k_size)

		lap_x = cv2.filter2D(img, ddepth = self.CV_DTYPE, kernel = ML_x) 
		lap_y = cv2.filter2D(img, ddepth = self.CV_DTYPE, kernel = ML_y)
		img = abs(lap_x) + abs(lap_y)

		img = cv2.boxFilter(img, ddepth=self.CV_DTYPE, ksize = (self.k_size, self.k_size) )

		#copy image to array
		self.image[self.current_depth] = img
		self.current_depth += 1
	
	def get_focus_data(self, y, x):
		return self.image[:, x, y].tolist()

def ML(size):
        ML_x = np.zeros((size,size))
        ML_y = np.zeros((size,size))

        ML_x[size/2, size/2] = 2
        ML_x[0,size/2] = -1
        ML_x[size -1, size/2] = -1

        ML_y[size/2, size/2] = 2
        ML_y[size/2,0] = -1
        ML_y[size/2, size - 1] = -1

        return ML_x, ML_y
