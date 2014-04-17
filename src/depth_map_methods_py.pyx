#distutils: language = c++
#distutils: sources = depth_map_methods.cpp
import numpy as np
cimport numpy as np
# We now need to fix a datatype for our arrays. I've used the variable
# DTYPE for this, which is assigned to the usual NumPy runtime
# type info object.
DTYPE = np.uint8
# "ctypedef" assigns a corresponding compile-time type to DTYPE_t. For
# every type in the numpy module there's a corresponding compile-time
# type with a _t-suffix.
ctypedef np.uint8_t DTYPE_t

#Image stack class definition
cdef extern from "depth_map_methods.h":
	cdef cppclass image_stack:
		image_stack(int, int, int, int, char*) except +
		void add(char*)
		void create_depth_map()
		void fuse_focus(char*)
		void refocus(int, int, char*)

#Python image class definition
cdef class Pyimage_stack:
	cdef image_stack *thisptr
	def __cinit__(self, int height, int width, int size, int threshold, char* output_img_dir):
		self.thisptr = new image_stack(height, width, size, threshold, output_img_dir)
		self.height = height
		self.width = width
	def __dealloc__(self):
		del self.thisptr
	def add(self, char* image_path):
		self.thisptr.add(image_path)
	def create_depth_map(self):
		self.thisptr.create_depth_map()
	def fuse_focus(self): #, np.ndarray[char, mode="c"] fuse_focused_image):
		fuse_focused_image = np.zeros((self.height, self.width, 3), dtype = DTYPE, order = 'C')
		self.thisptr.fuse_focus(fuse_focused_image[0])
		return fuse_focused_image
	def refocus(self, int depth_of_field, int depth_focus_point): #, np.ndarray[char, mode="c"] refocused_image):
		refocused_image = np.zeros((self.height, self.width, 3), dtype = DTYPE, order = 'C')
		self.thisptr.refocus(depth_of_field, depth_focus_point, refocused_image[0])
		return refocused_image
