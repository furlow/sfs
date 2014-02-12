from distutils.core import setup
from Cython.Build import cythonize

setup(
      name = "depth_map_methods_py",
      ext_modules = cythonize('depth_map_methods.pyx'),
)
