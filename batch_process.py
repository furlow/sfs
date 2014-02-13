import os
from shape_from_focus import shape_from_focus
import sys

photo_batch_dir = sys.argv[1]

#photo_batch = os.listdir(photo_batch_dir)

#for scene in photo_batch:
for k_size in range(13,21,2):
	print(photo_batch_dir + " with k_size of " + str(k_size))
	shape_from_focus(photo_batch_dir, k_size)
