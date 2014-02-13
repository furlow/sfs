#!/usr/bin/python

#Import various libaries
import cv2
import numpy as np
from datetime import datetime
from matplotlib import pyplot as plt
import os

img_file = 'images/mouse/out.jpg'

img = cv2.imread(img_file, 0)

img = cv2.equalizeHist(img)

img = cv2.blur(img, (3,3))

img[img >= 128]= 255
img[img < 128] = 0

cv2.imwrite('images/mouse/out_posterized.jpg', img)
