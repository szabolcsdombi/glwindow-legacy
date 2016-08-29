open('../Makefile', 'w').write('''
all: clean install

clean:
	python Clean.py

install:
	install.bat
'''.strip())

open('../Source/Makefile', 'w').write('''
all:
	cd .. && make
'''.strip())

open('../multi_install.bat', 'w').write('''
call python Clean.py
SET CUSTOM_GCC=YES
call pythons.bat setup.py install
'''.strip())

open('../multi_upload.bat', 'w').write('''
SET CUSTOM_GCC=YES
call python setup.py sdist upload 
call pythons setup.py bdist_wheel upload
call pythons setup.py bdist_egg upload
call pythons setup.py bdist_wininst upload
'''.strip())

open('../install.bat', 'w').write('call python setup.py install --record files.txt')

open('../Clean.py', 'w').write('''
import shutil, os

if os.path.isdir('build'):
	shutil.rmtree('build')

if os.path.isdir('dist'):
	shutil.rmtree('dist')

if os.path.isdir('__pycache__'):
	shutil.rmtree('__pycache__')

if os.path.isdir('GLWindow/__pycache__'):
	shutil.rmtree('GLWindow/__pycache__')

if os.path.isdir('Development/__pycache__'):
	shutil.rmtree('Development/__pycache__')

if os.path.isfile('Development/cccompiler.pyc'):
	os.remove('Development/cccompiler.pyc')

if os.path.isdir('GLWindow.egg-info'):
	shutil.rmtree('GLWindow.egg-info')

if os.path.isfile('files.txt'):
	os.remove('files.txt')

if os.path.isfile('log.txt'):
	os.remove('log.txt')
'''.strip())
