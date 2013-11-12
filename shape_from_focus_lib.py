#Import various libaries
import cv2
import numpy as np

#This is the image stack class
class image_stack:
	current_depth = 0

	#Initalization function
	def __init__(self, depth, height, width):	
		self.height = height
		self.width = width
		self.depth = depth
		self.stretch_factor = 256 / depth
		self.depth_map = np.zeros( (height, width), dtype=np.float64)
		self.image = np.zeros((depth, height, width), dtype=np.float64)

	#This function is used to add an image to the stack
	def add(self, img_file):

		#open the image file
		img = cv2.imread(img_file, 0)
		img = img.astype(dtype=np.float64)

		#find the focus using laplacian operator, this needs to be improved
		img = cv2.Laplacian(img, -1)
		
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
	
	for depth in range(0, stack.depth):
		grad = stack.image[depth, y, x] - stack.image[depth - 1, y, x]
		#Check for peak focus
		if( grad == 0 or (grad < 0 and grad_prev > 0)):
			#Look for the peak with heighest focus
			if(stack.image[depth, y, x] > focus_value):
				focus_depth = depth
				focus_value = stack.image[depth, y, x]
	return focus_depth * stack.stretch_factor

#Function for generating a depth map from a a focus image stack class
def create_depth_map(stack):
	for x in range(0, stack.width):
		for y in range(0, stack.height):
			#print(str(x) + "." + str(y))
			stack.depth_map[y,x] = detect_focus_peak(stack, y, x)
