#distutils: language = c++
#distutils: sources = depth_map_methods.cpp

#Image stack class definition
cdef extern from "depth_map_methods.h":
	cdef cppclass image_stack:
		image_stack(int, int, int, int, char*) except +
		void add(char*)
		void create_depth_map()
		void fuse_focus()
		void refocus(int, int)

cdef class Pyimage_stack:
	cdef image_stack *thisptr
	def __cinit__(self, int height, int width, int size, int threshold, char* output_img_dir):
		self.thisptr = new image_stack(height, width, size, threshold, output_img_dir)
	def __dealloc__(self):
		del self.thisptr
	def add(self, char* image_path):
		self.thisptr.add(image_path)
	def create_depth_map(self):
		self.thisptr.create_depth_map()
	def fuse_focus(self):
		self.thisptr.fuse_focus()
	def refocus(self, int depth_of_field, int depth_focus_point):
		self.thisptr.refocus(depth_of_field, depth_focus_point)
