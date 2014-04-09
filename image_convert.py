from wand.image import Image
import os
import sys

img_dir = sys.argv[1]

images = [image for image in os.listdir(img_dir) if image.endswith('.JPG')]

for image in images:
    with Image(filename=img_dir + image) as img:
        name, ext = image.split('.')
        print('Resizing ' + name)
        img.resize(int(img.width * 0.6), int(img.height * 0.6) )
        print('Converting ' + name)
        img.save(filename=img_dir + name + '.pgm')
