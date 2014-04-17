#!/usr/bin/python

#import library of functions
import cv2
from matplotlib import pyplot as plt
from matplotlib import cm
import os
import time
import thread
import sort
import sys
import shutil
import subprocess
from depth_map_methods_py import Pyimage_stack

tmp_copy = 'tmp_copy/'
tmp_cropped = 'tmp_cropped/'
output = 'output/'

#Check if the folders ave been created and images cropped
#Need to improve this
#TODO use seperate temporary folders rather than in the current directories
def align_images(img_dir):
    image_files = [image_file for image_file in os.listdir(img_dir) if image_file.endswith('.JPG')]
    sort.sort_nicely(image_files)

    print('Creating temporary directories...')
    os.mkdir(img_dir + tmp_copy)
    os.mkdir(img_dir + tmp_cropped)
    os.mkdir(img_dir + output)

    align_image_stack_cmd = ["/Applications/Hugin.app/Contents/MacOS/align_image_stack", "-m", "-C", "-c", "4",
                             "-a",
                             img_dir + tmp_cropped]
    #align_image_stack_cmd = ["align_image_stack", "-m", "-l", "-C", "-c", "4", "-a", img_dir + tmp_cropped]

    for file in image_files:
        shutil.copyfile(img_dir + file, img_dir + tmp_copy + file)
        align_image_stack_cmd.append(img_dir + tmp_copy + file)

    #Align image files to remove focus magnification
    print('Aligning images...')
    subprocess.call(align_image_stack_cmd)

def process_stack(img_dir):
    if not os.path.exists(img_dir + tmp_copy) \
    and not os.path.exists(img_dir + tmp_cropped) \
    and not os.path.exists(img_dir + output):
        align_images(img_dir) #A more robust check should be performed

    #List the files in the image directory and sort them in numerical
    #and alphabetical order
    image_files = [image_file for image_file in os.listdir(img_dir + tmp_cropped) if image_file.endswith('.tif')]
    sort.sort_nicely(image_files)
    img = cv2.imread(img_dir + tmp_cropped + image_files[0], 0)
    height, width = img.shape

    #Create a new image stack
    #TODO move the opening of the first image to get
    #the height and width of the image stack from within the Pyimage_stack
    #place the check in the constructor in the c++ or the cythong code
    print('Creating image stack with dimensions', height, width)
    stack = Pyimage_stack(height, width, len(image_files), 1000, img_dir + output)

    # Add image files
    # TODO Move this into the c++ code
    start = time.time()
    for image_file in image_files:
        print "Adding " + image_file
        stack.add(img_dir + tmp_cropped + image_file)
    finish = time.time()
    print 'Execution time =', finish - start

    # Generate Depth map
    # TODO Move this into the c++ code
    start = time.time()
    print('Generating Depth map...')
    stack.create_depth_map()
    finish = time.time()
    print 'Execution time =', finish - start

    # Generate an all in focus image
    # TODO Move this into the c++ code
    start = time.time()
    print('Generating fuse focus image')
    fused_image = stack.fuse_focus()
    plt.imshow(fused_image)
    finish = time.time()
    print 'Execution time =', finish - start
    return stack

def main():
    img_dir = sys.argv[1]
    print img_dir
    process_stack(img_dir)

if __name__ == '__main__':
    main()
