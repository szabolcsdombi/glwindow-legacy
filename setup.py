from setuptools import Extension, setup

long_description = open('README.md').read()

glwindow = Extension(
    name='glwindow',
    sources=['./glwindow.cpp'],
    libraries=['user32'],
)

setup(
    name='glwindow',
    version='4.0.0',
    description='glwindow',
    long_description=long_description,
    long_description_content_type='text/markdown',
    url='https://github.com/cprogrammer1994/glwindow',
    author='Szabolcs Dombi',
    author_email='cprogrammer1994@gmail.com',
    license='MIT',
    ext_modules=[glwindow],
)
