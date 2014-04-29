#!/usr/bin/python
import sys
from depth_map_methods_py import Pyimage_stack

def main():
    img_dir = sys.argv[1]
    print img_dir
    stack = Pyimage_stack(img_dir, threshold)

if __name__ == '__main__':
    main()
