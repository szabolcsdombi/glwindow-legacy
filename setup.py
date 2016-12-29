from setuptools import setup, Extension

try:
	from Development import cccompiler
except ImportError:
	pass

GLWindow = Extension(
	'GLWindow.GLWindow',
	libraries = ['User32', 'opengl32', 'gdi32'],
	sources = [
		'Source/GLWindow.cpp',
		'Source/ModernContext.cpp',
	]
)

setup(
	name = 'GLWindow',
	version = '1.1.2',
	description = 'GLWindow',
	url = 'https://github.com/cprogrammer1994/GLWindow',
	author = 'Szabolcs Dombi',
	author_email = 'cprogrammer1994@gmail.com',
	license = 'MIT',
	packages = ['GLWindow'],
	ext_modules = [GLWindow],
	platforms = ['win32', 'win64']
)
