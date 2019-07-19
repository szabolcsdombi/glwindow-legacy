FROM quay.io/pypa/manylinux1_i686:latest as build_i686
COPY . .
RUN /opt/python/cp35-cp35m/bin/python setup.py bdist_wheel && auditwheel repair dist/*.whl && rm dist/*.whl
RUN /opt/python/cp36-cp36m/bin/python setup.py bdist_wheel && auditwheel repair dist/*.whl && rm dist/*.whl
RUN /opt/python/cp37-cp37m/bin/python setup.py bdist_wheel && auditwheel repair dist/*.whl && rm dist/*.whl

FROM quay.io/pypa/manylinux1_x86_64:latest as build_x86_64
COPY . .
RUN /opt/python/cp35-cp35m/bin/python setup.py bdist_wheel && auditwheel repair dist/*.whl && rm dist/*.whl
RUN /opt/python/cp36-cp36m/bin/python setup.py bdist_wheel && auditwheel repair dist/*.whl && rm dist/*.whl
RUN /opt/python/cp37-cp37m/bin/python setup.py bdist_wheel && auditwheel repair dist/*.whl && rm dist/*.whl

FROM python:3.7.3
RUN pip install -U twine
COPY --from=build_i686 /wheelhouse/* dist/
COPY --from=build_x86_64 /wheelhouse/* dist/
CMD python -m twine upload dist/*
