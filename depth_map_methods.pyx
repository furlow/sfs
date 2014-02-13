#Import various neccessary libraries
cimport numpy as np
cimport cython

#Define sometypes
#DTYPE = np.float64
ctypedef np.float32_t DTYPE_t

#Function for generating a depth map from a focus image stack class
@cython.boundscheck(False)
@cython.wraparound(False)
@cython.nonecheck(False)
def create_depth_map(stack):
    cdef int x, y
    cdef int focus_depth = 0
    cdef float focus_value = 0
    cdef int depth = 0

    cdef np.ndarray[DTYPE_t, ndim=3] image = stack.image
    cdef np.ndarray[DTYPE_t, ndim=2] depth_map = stack.depth_map
    cdef int stretch_factor = stack.stretch_factor
    

    for x in xrange(stack.width):
        for y in xrange(stack.height):
            focus_depth = 0
            focus_value = 0
            depth = 0
            
            for depth in xrange(stack.depth - 1):
                if(image[depth, y, x] > focus_value):
                    focus_depth = depth
                    focus_value = image[depth, y, x]
                
            depth_map[y,x] = focus_depth
            
    depth_map = depth_map * stack.stretch_factor
