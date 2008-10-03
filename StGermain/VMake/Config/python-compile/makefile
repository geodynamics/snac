SHELL := /bin/bash
PROJ_ROOT := $(shell until test -d ./VMake ; do cd .. ; done ; pwd)

ifeq ($(shell if test -r ${PROJ_ROOT}/Makefile.system; then echo true; fi),true)
	include ${PROJ_ROOT}/Makefile.system
endif

override RM_F=rm -f

override USE_PDT=0

override PYC_DIR := $(shell pwd)

PYTHON_PACKAGE = conftest
pyc = conftest.py

packages = PYTHON

include ${PROJ_ROOT}/Makefile.vmake
