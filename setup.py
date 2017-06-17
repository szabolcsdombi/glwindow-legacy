from setuptools import setup, Extension

long_description = '''
`GLWindow on github <https://github.com/cprogrammer1994/GLWindow>`_

`Documentation <http://glwindow.readthedocs.io/en/latest/>`_
'''

GLWindow = Extension(
	'GLWindow.glwnd',
	libraries = ['User32', 'opengl32', 'gdi32'],
	sources = [
		'src/GLWindow.cpp',
		'src/ModernContext.cpp',
	]
)

setup(
	name = 'GLWindow',
	version = '2.1.2',
	description = 'GLWindow',
	long_description = long_description.strip(),
	url = 'https://github.com/cprogrammer1994/GLWindow',
	author = 'Szabolcs Dombi',
	author_email = 'cprogrammer1994@gmail.com',
	license = 'MIT',
	packages = ['GLWindow'],
	ext_modules = [GLWindow],
	install_requires = ['ModernGL'],
	platforms = ['win32', 'win64']
)
