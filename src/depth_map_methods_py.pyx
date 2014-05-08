#distutils: language = c++
#distutils: sources = depth_map_methods.cpp
import numpy as np
cimport numpy as np
import os
import sys
import sort
import shutil
import subprocess
import json
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

RELOAD = 1
RECREATE = 0
ALIGN_ON = 1
ALIGN_OFF = 0

#Image stack class definition
cdef extern from "depth_map_methods.h":
	cdef cppclass image_stack:
		image_stack(char*, int, int, int, int, int) except +
		void load(int)
		void allocate(char*, char*, char*, int, int)
		void resize()
		void add(char*)
		void create_depth_map()
		void fuse_focus()
		void setDefocus(int)
		void refocus(int, int)
		void refocus_multiple(int, int)


#Python image class definition
cdef class Pyimage_stack:
	cdef image_stack *thisptr
	cdef public int scaled_height
	cdef public int scaled_width
	cdef public int depth
	cdef public int depth_of_field
	cdef public int size
	cdef public np.ndarray focused_image
	cdef public np.ndarray refocused_image
	cdef public np.ndarray depth_map
	cdef img_dir

	def __cinit__(self,
				  char* img_dir,
				  int threshold,
				  int quantization,
				  int scaled_width,
				  int scaled_height,
				  int align_toggle,
				  int depth_map_toggle):

		self.scaled_height = scaled_height
		self.scaled_width = scaled_width
		self.img_dir = img_dir
		self.depth_of_field = 0
		self.depth = 0

		if os.path.exists(self.img_dir + output + 'attributes.txt'):
			with open(img_dir + output + 'attributes.txt', 'r') as infile:
				self.size = json.load(infile)
				self.thisptr.load(self.size)
		else:
			if not os.path.exists(self.img_dir + output):
				os.mkdir(self.img_dir + output)
			files = [file for file in os.listdir(self.img_dir) if file.endswith('.JPG')]
			self.size = len(files)
			with open(self.img_dir + output + 'attribtues.txt', 'w') as outfile:
				json.dump(self.size, outfile)

		if not os.path.exists(self.img_dir + tmp_copy) \
		and not os.path.exists(self.img_dir + tmp_cropped) \
		and align_toggle == ALIGN_ON:
			self.align_images()

		self.thisptr = new image_stack(self.img_dir,
										threshold,
										self.size,
										quantization,
										scaled_width,
										scaled_height)


		self.allocate()

		if os.path.exists(self.img_dir + output + 'depth_map.png') \
		and os.path.exists(self.img_dir + output + 'fused_focus.png') \
		and depth_map_toggle == RELOAD:
			self.thisptr.load(self.size)
		elif align_toggle == ALIGN_ON:
			self.compute(self.img_dir + tmp_cropped)
		elif align_toggle == ALIGN_OFF:
			self.compute(self.img_dir)
		else:
			raise("Error couldn't determine how to load or compute images")

	def __dealloc__(self):
		del self.thisptr

	def add(self, char* image_path):
		self.thisptr.add(image_path)

	def compute(self, img_dir):
		image_files = [image_file for image_file in os.listdir(img_dir) if image_file.endswith('.tif') or image_file.endswith('.JPG')]
		sort.sort_nicely(image_files)

		#Add the files to stack
		for image_file in image_files:
			print "Adding " + image_file
			self.add(img_dir + image_file)

		#Process the stack and create the depth map and fused images
		self.thisptr.create_depth_map()
		self.thisptr.fuse_focus()

	def setDof(self, int depth_of_field):
		self.depth_of_field = depth_of_field
		self.thisptr.refocus(self.depth_of_field, self.depth)

	def setDefocus(self, int defocus):
		self.thisptr.setDefocus(defocus)
		self.thisptr.refocus(self.depth_of_field, self.depth)

	def refocus(self, int focus_depth):
		self.depth = focus_depth
		self.thisptr.refocus(self.depth_of_field, self.depth)

	def refocus_by_point(self, int y, int x):
		self.depth = self.depth_map[y, x]
		print "Depth: ", self.depth
		self.thisptr.refocus(self.depth_of_field, self.depth)
		return self.depth

	def refocus_between_points(self, int y1, int x1, int y2, int x2):
		cdef char focus_depth_1 = self.depth_map[y1, x1]
		cdef char focus_depth_2 = self.depth_map[y2, x2]
		print "focus depth 1:", focus_depth_1
		print "focus depth 2:", focus_depth_2
		self.depth_of_field = abs(focus_depth_1 - focus_depth_2) / 2
		self.depth = (focus_depth_1 + focus_depth_2) / 2
		print "depth: ", self.depth
		print "depth of field", self.depth_of_field
		self.thisptr.refocus(self.depth_of_field, self.depth)
		return self.depth, self.depth_of_field

	def refocus_multiple(self, int y1, int x1, int y2, int x2):
		focus_depth_1 = self.depth_map[y1, x1]
		focus_depth_2 = self.depth_map[y2, x2]
		self.thisptr.refocus_multiple(focus_depth_1, focus_depth_2)

	def resize(self, int in_scaled_width , int in_scaled_height):
		self.scaled_height = in_scaled_height
		self.scaled_width = in_scaled_width
		self.allocate()
		self.thisptr.resize(self.scaled_width, self.scaled_height)

	def allocate(self):
		print "Re allocating scaled images"
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

		if not os.path.exists(self.img_dir + tmp_copy):
			os.mkdir(self.img_dir + tmp_copy)
		if not os.path.exists(self.img_dir + tmp_cropped):
			os.mkdir(self.img_dir + tmp_cropped)

		os_type = sys.platform
		if os_type == 'darwin':
			align_image_stack_cmd = ["/Applications/Hugin.app/Contents/MacOS/align_image_stack"]
		elif os_type == 'linux2':
			align_image_stack_cmd = ["align_image_stack"]
		elif os_type == 'win32':
			align_image_stack_cmd = ["align_image_stack"]
		else:
			raise('OS Type was not recognised!')

		align_image_stack_cmd +=["-m",
								 "-C",
								 "-c",
								 "4",
								 "-l",
								 "-a",
								 self.img_dir + tmp_cropped]

		for file in image_files:
			shutil.copyfile(self.img_dir + file, self.img_dir + tmp_copy + file)
			align_image_stack_cmd.append(self.img_dir + tmp_copy + file)

		#Align image files to remove focus magnification
		print('Aligning images...')
		subprocess.call(align_image_stack_cmd)
