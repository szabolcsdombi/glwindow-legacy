ifeq ($(OS), Windows_NT)
	PYTHON = python
else
	PYTHON = python3
endif

all: build

build:
	$(PYTHON) setup.py build_ext -b .

docs: build
	$(PYTHON) setup.py build_sphinx

wheel:
	$(PYTHON) setup.py bdist_wheel

test: build
	$(PYTHON) -m pytest

install:
	$(PYTHON) setup.py install

.PHONY: all build docs wheel install test
