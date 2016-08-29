from setuptools import setup, Extension

GLWindow = Extension(
	'GLWindow.GLWindow',
	libraries = ['User32'],
	sources = [
		'Source/WindowsGL.cpp',
		'Source/Window.cpp',
		'Source/Python-Window.cpp',
	]
)

setup(
	name = 'GLWindow',
	version = '0.3.0',
	description = 'GLWindow',
	url = 'https://github.com/cprogrammer1994/GLWindow',
	author = 'Szabolcs Dombi',
	author_email = 'cprogrammer1994@gmail.com',
	license = 'MIT',
	packages = ['GLWindow'],
	ext_modules = [GLWindow],
	platforms = ['win32', 'win64']
)
