rem python Clean.py
SET CUSTOM_GCC=YES
rem call pythons.bat setup.py install
call pythons setup.py bdist_wheel upload
call pythons setup.py bdist_egg upload
call pythons setup.py bdist_wininst upload
call python setup.py sdist upload
