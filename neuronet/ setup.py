# setup.py
from setuptools import setup, Extension
from Cython.Build import cythonize
import sys
import os

here = os.path.abspath(os.path.dirname(__file__))

cpp_src = [
    os.path.join("cpp", "src", "sparse_graph.cpp"),
]

ext = Extension(
    "neuronet_core",
    sources=[os.path.join("cython", "graph_wrapper.pyx")] + cpp_src,
    language="c++",
    include_dirs=[os.path.join(here, "cpp", "include")],
    extra_compile_args=["-O3", "-std=c++17", "-march=native"],
    extra_link_args=[],
)

setup(
    name="neuronet_core",
    ext_modules=cythonize([ext], language_level=3),
    zip_safe=False,
)
