import os
import subprocess
import sys

image_dir = sys.argv[1]

list_of_images = os.listdir(image_dir)

#os.makedir(image_dir + "converted")

for image in list_of_images:
	file_name, file_ext = os.path.splitext(image)
	if( file_ext == ".tif" ):
		print("converting" + str(file_name))
		subprocess.call(["convert",image_dir + image, "-resize", "30%", image_dir + "converted/" + file_name + ".JPG"])

