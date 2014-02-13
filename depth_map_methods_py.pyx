#distutils: language = c++
#distutils: sources = depth_map_methods.cpp

cdef extern from "depth_map_methods.h":
    cdef cppclass image_stack:
        image_stack(int, int, int) except +
        void add(char*)
        void create_depth_map()

cdef class Pyimage_stack:
    cdef image_stack *thisptr
    def __cinit__(self, int height, int width, int size):
        self.thisptr = new image_stack(height, width, size)
    def __dealloc__(self):
        del self.thisptr
    def add(self, char* image_path):
        self.thisptr.add(image_path)
    def create_depth_map(self):
        self.thisptr.create_depth_map()