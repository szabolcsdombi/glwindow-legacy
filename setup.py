import sys
from setuptools import Extension, setup

long_description = open('README.md').read()

if sys.platform == 'win32':
    glwindow = Extension(
        name='glwindow',
        sources=['./glwindow.cpp'],
        libraries=['user32'],
    )

if sys.platform == 'linux':
    glwindow = Extension(
        name='glwindow',
        sources=['./glwindow_linux.cpp'],
        libraries=['X11'],
        extra_compile_args=['-fpermissive'],
    )

setup(
    name='glwindow',
    version='4.1.0',
    description='glwindow',
    long_description=long_description,
    long_description_content_type='text/markdown',
    url='https://github.com/cprogrammer1994/glwindow',
    author='Szabolcs Dombi',
    author_email='cprogrammer1994@gmail.com',
    license='MIT',
    ext_modules=[glwindow],
)
