from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext

setup(
      name = 'depth_map_methods_py',
      version = '0.1',
      author = 'Joshua Furlow',
      # The ext modules interface the cpp code with the python one:
                ext_modules=[
                              Extension('depth_map_methods_py',
                              sources = ["depth_map_methods_py.pyx", "depth_map_methods.cpp"],
                              include_dirs=[".","/usr/local/include/opencv2", "/usr/local/include"],
                              language="c++",
                              library_dirs=['/usr/local/lib'],
                              libraries=['opencv_core', 'opencv_highgui'])
                             ],
      cmdclass = {'build_ext': build_ext},
)
