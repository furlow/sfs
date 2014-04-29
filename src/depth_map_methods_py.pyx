#distutils: language = c++
#distutils: sources = depth_map_methods.cpp
import numpy as np
cimport numpy as np
import os
import sort
import shutil
import subprocess
import matplotlib.pylab as plt
# We now need to fix a datatype for our arrays. I've used the variable
# DTYPE for this, which is assigned to the usual NumPy runtime
# type info object.
DTYPE = np.uint8
# "ctypedef" assigns a corresponding compile-time type to DTYPE_t. For
# every type in the numpy module there's a corresponding compile-time
# type with a _t-suffix.
ctypedef np.uint8_t DTYPE_t

tmp_cropped = "tmp_cropped/"
output = "output/"
tmp_copy = "tmp_copy/"

#Image stack class definition
cdef extern from "depth_map_methods.h":
	cdef cppclass image_stack:
		image_stack(char*, int, int, int) except +
		void load()
		void add(char*)
		void create_depth_map()
		void fuse_focus()
		void refocus(int, int)
		void resize(int, int)
		void allocate(char*, char*, char*, int, int)

#Python image class definition
cdef class Pyimage_stack:
	cdef image_stack *thisptr
	cdef public int scaled_height
	cdef public int scaled_width
	cdef public int depth
	cdef public np.ndarray focused_image
	cdef public np.ndarray refocused_image
	cdef public np.ndarray depth_map
	cdef public img_dir

	def __cinit__(self,
				  char* img_dir,
				  int threshold,
				  int scaled_width,
				  int scaled_height):

		self.scaled_height = scaled_height
		self.scaled_width = scaled_width
		self.img_dir = img_dir

		self.thisptr = new image_stack(self.img_dir,
										threshold,
										scaled_width,
										scaled_height)

		if not os.path.exists(self.img_dir + tmp_copy) \
		and not os.path.exists(self.img_dir + tmp_cropped) \
		and not os.path.exists(self.img_dir + output):
			self.align_images(self.img_dir)

		self.allocate()

		if os.path.exists(self.img_dir + output + 'depth_map.png') \
		and os.path.exists(self.img_dir + output + 'fused_focus.png'):
			# Load pre computed files
			self.thisptr.load()
		else:
			self.compute()

	def __dealloc__(self):
		del self.thisptr

	def add(self, char* image_path):
		self.thisptr.add(image_path)

	def compute(self):
		image_files = [image_file for image_file in os.listdir(self.img_dir + tmp_cropped) if image_file.endswith('.tif')]
		sort.sort_nicely(image_files)

		#Add the files to stack
		for image_file in image_files:
			print "Adding " + image_file
			self.add(self.img_dir + tmp_cropped + image_file)

		#Process the stack and create the depth map and fused images
		self.thisptr.create_depth_map()
		self.thisptr.fuse_focus()

	def refocus(self, int depth_of_field, int focus_depth):
		self.depth = focus_depth
		self.thisptr.refocus(1, self.depth)

	def refocus_by_point(self, int y, int x):
		depth = self.depth_map[y, x]
		print "Depth: ", depth
		self.refocus(1, depth)
		return depth

	def resize(self, int in_scaled_width , int in_scaled_height):
		self.scaled_height = in_scaled_height
		self.scaled_width = in_scaled_width
		self.allocate()
		self.thisptr.resize(self.scaled_width, self.scaled_height)

	def allocate(self):
		cdef np.ndarray[char, ndim=2, mode="c"] depth_map_local
		cdef np.ndarray[char, ndim=3, mode="c"] focused_local
		cdef np.ndarray[char, ndim=3, mode="c"] refocused_local
		depth_map_local = np.empty((self.scaled_height, self.scaled_width), dtype = DTYPE, order = 'c')
		focused_local = np.empty((self.scaled_height, self.scaled_width, 3), dtype = DTYPE, order = 'c')
		refocused_local = np.empty((self.scaled_height, self.scaled_width, 3), dtype = DTYPE, order = 'c')
		self.thisptr.allocate(&depth_map_local.data[0], &focused_local.data[0], &refocused_local.data[0], self.scaled_width, self.scaled_height)
		self.focused_image = focused_local
		self.refocused_image = refocused_local
		self.depth_map = depth_map_local

	def align_images(self):
		image_files = [image_file for image_file in os.listdir(self.img_dir) if image_file.endswith('.JPG')]
		sort.sort_nicely(image_files)

		print('Creating temporary directories...')
		os.mkdir(self.img_dir + tmp_copy)
		os.mkdir(self.img_dir + tmp_cropped)
		os.mkdir(self.img_dir + output)

		align_image_stack_cmd = ["/Applications/Hugin.app/Contents/MacOS/align_image_stack",
								"-m",
								"-C",
								"-c",
								"4",
								"-a",
								self.img_dir + tmp_cropped]
		#align_image_stack_cmd = ["align_image_stack", "-m", "-l", "-C", "-c", "4", "-a", self.img_dir + tmp_cropped]

		for file in image_files:
			shutil.copyfile(self.img_dir + file, self.img_dir + tmp_copy + file)
			align_image_stack_cmd.append(self.img_dir + tmp_copy + file)

		#Align image files to remove focus magnification
		print('Aligning images...')
		subprocess.call(align_image_stack_cmd)
