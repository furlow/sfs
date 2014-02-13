import numpy as np

def SML(size):
	ML_x = np.zeros((size,size))
	ML_y = np.zeros((size,size))
	
	ML_x[size/2, size/2] = 2 
	ML_x[0,size/2] = -1
	ML_x[size -1, size/2] = -1

	ML_y[size/2, size/2] = 2 
	ML_y[size/2,0] = -1
	ML_y[size/2, size - 1] = -1

	return ML_x, ML_y




print (SML(7))
