import sys

from setuptools import Extension, setup

long_description = open('README.md').read()

if sys.platform == 'win32':
    glwindow = Extension(
        name='glwindow',
        sources=['src/glwindow.cpp'],
        libraries=['User32'],
    )

if sys.platform == 'linux':
    glwindow = Extension(
        name='glwindow',
        sources=['src/glwindow_linux.cpp'],
        libraries=['X11'],
        extra_compile_args=['-fpermissive'],
    )

setup(
    name='glwindow',
    version='4.2.0',
    description='A simple window with your own main loop.',
    long_description=long_description,
    long_description_content_type='text/markdown',
    url='https://github.com/glnext/glwindow/',
    author='Szabolcs Dombi',
    author_email='cprogrammer1994@gmail.com',
    license='MIT',
    ext_modules=[glwindow],
)
