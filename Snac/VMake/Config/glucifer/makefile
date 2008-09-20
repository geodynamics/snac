SHELL := /bin/bash
PROJ_ROOT := $(shell until test -d ./VMake ; do cd .. ; done ; pwd)

ifeq ($(shell if test -r ${PROJ_ROOT}/Makefile.system; then echo true; fi),true)
	include ${PROJ_ROOT}/Makefile.system
endif

override RM_F=rm -f

override USE_PDT=0

bin = conftest 
override BIN_DIR := $(shell pwd)
override INC_DIR := $(shell pwd)
override LIB_DIR := $(shell pwd)
override TMP_DIR := $(shell pwd)

SRCS = conftest.c

packages = GLUCIFER STGFEM STGDOMAIN STGERMAIN MPI XML MATH DL

include ${PROJ_ROOT}/Makefile.vmake
