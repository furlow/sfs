#Import various neccessary libraries
#import numpy as np
cimport numpy as np

#Define sometypes
#DTYPE = np.float64
ctypedef np.float32_t DTYPE_t

#Function for determining the focus maximum of a pixel
def detect_focus_peak(np.ndarray[DTYPE_t, ndim=3] image, int stack_depth, int y, int x):
	cdef int focus_depth = 0
	cdef float focus_value = 0
	cdef int depth = 0
	
	for depth in xrange(stack_depth - 1):
		if(image[depth, y, x] > focus_value):
			focus_depth = depth
			focus_value = image[depth, y, x]
			
	return focus_depth

#Function for generating a depth map from a focus image stack class
def create_depth_map(stack):
	cdef int x, y
	
	for x in xrange(stack.width):
		for y in xrange(stack.height):
			#print(str(x) + "." + str(y))
			stack.depth_map[y,x] = detect_focus_peak(stack.image, stack.depth, y, x)
	
	stack.depth_map = stack.depth_map * stack.stretch_factor
