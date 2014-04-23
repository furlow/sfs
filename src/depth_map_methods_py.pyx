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
		void create_depth_map(char*)
		void fuse_focus(char*)
		void refocus(int, int, char*)

#Python image class definition
cdef class Pyimage_stack:
	cdef image_stack *thisptr
	cpdef public int height
	cpdef public int width
	cdef public np.ndarray focused_image
	cdef public np.ndarray refocused_image
	cdef public np.ndarray depth_map
	def __cinit__(self, int height, int width, int size, int threshold, char* output_img_dir):
		self.thisptr = new image_stack(height, width, size, threshold, output_img_dir)
		self.height = height
		self.width = width
	def __dealloc__(self):
		del self.thisptr
	def add(self, char* image_path):
		self.thisptr.add(image_path)
	def create_depth_map(self):
		cdef np.ndarray[char, ndim=2, mode="c"] depth_map_local
		depth_map_local = np.zeros((self.height, self.width), dtype = DTYPE, order = 'c')
		self.thisptr.create_depth_map( &depth_map_local.data[0] )
		self.depth_map = depth_map_local
		return depth_map_local
	def fuse_focus(self):
		cdef np.ndarray[char, ndim=3, mode="c"] focused_image_local
		focused_image_local = np.zeros((self.height, self.width, 3), dtype = DTYPE, order = 'c')
		self.thisptr.fuse_focus( &focused_image_local.data[0] )
		self.focused_image = focused_image_local
		return focused_image_local
	def refocus(self, int depth_of_field, int depth_focus_point):
		cdef np.ndarray[char, ndim=3, mode="c"] refocused_image_local
		refocused_image_local = np.zeros((self.height, self.width, 3), dtype = DTYPE, order = 'c')
		self.thisptr.refocus(depth_of_field, depth_focus_point, &refocused_image_local.data[0] )
		self.refocused_image = refocused_image_local
		return refocused_image_local
	def refocus_by_point(self, int x, int y):
		depth = self.depth_map[x, y]
		return self.refocus(1, depth)
