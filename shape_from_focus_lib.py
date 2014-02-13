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
	def __init__(self, img_dir, CV_DTYPE, NP_DTYPE):
	
		
		self.depth = len(image_files)
		self.stretch_factor = 255 / self.depth - 1
		
		#Create depth map and image stack arrays
		self.depth_map = np.zeros((self.height, self.width), dtype=np.uint8)
		self.image = np.zeros((self.depth, self.height, self.width), dtype=NP_DTYPE)
		
		#Add the files
		for file in image_files:
				print('Adding ' + file)
				self.add(img_dir + file)

	#This function is used to add an image to the stack
	def add(self, img_file):

		#open the image file
		img = cv2.imread(img_file, 0)
		img = img.astype(dtype=self.NP_DTYPE)

		#find the focus using laplacian operator, this needs to be improved
		#sobelx = cv2.Sobel(img, ddepth=cv2.CV_64F, dx=1, dy=0, ksize=11)
		#sobely = cv2.Sobel(img, ddepth=cv2.CV_64F, dx=0, dy=1, ksize=11)
		#img = sobelx * 0.5 + sobely * 0.5
		
		img = cv2.Laplacian(img, ddepth=self.CV_DTYPE, ksize=15)

		img = abs(img)

		#copy image to array
		self.image[self.current_depth] = img
		self.current_depth += 1
	
	def get_focus_data(self, y, x):
		return self.image[:, x, y].tolist()
