import platform

from setuptools import setup, Extension

PLATFORMS = {'windows', 'linux'}

libraries = {
    'windows': ['gdi32', 'opengl32', 'user32'],
    'linux': ['GL', 'X11'],
    # 'darwin': [],
}

extra_compile_args = {
    'windows': [],
    'linux': ['-fpermissive'],
    # 'darwin': ['-Wno-deprecated-declarations'],
}

extra_linker_args = {
    'windows': [],
    'linux': [],
    # 'darwin': ['-framework', 'OpenGL', '-Wno-deprecated'],
}

sources = {
    'windows': [
        'glwindow/glwindow.cpp',
        'glwindow/core_windows.cpp',
        'res/glwindow.rc',
    ],
    'linux': [
        'glwindow/glwindow.cpp',
        'glwindow/core_linux.cpp',
    ],
    # 'darwin': [],
}

target = platform.system().lower()

if target not in PLATFORMS:
    target = 'linux'

long_description = open('README.md').read()

glwindow = Extension(
    name='glwindow',
    sources=sources[target],
    extra_compile_args=extra_compile_args[target],
    extra_linker_args=extra_linker_args[target],
    libraries=libraries[target],
)

setup(
    name='glwindow',
    version='3.0.0',
    description='glwindow',
    long_description=long_description,
    long_description_content_type='text/markdown',
    url='https://github.com/cprogrammer1994/glwindow',
    author='Szabolcs Dombi',
    author_email='cprogrammer1994@gmail.com',
    license='MIT',
    ext_modules=[glwindow],
    install_requires=['moderngl'],
)
