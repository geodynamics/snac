SHELL := /bin/bash
PROJ_ROOT := $(shell until test -d ./VMake ; do cd .. ; done ; pwd)

ifeq ($(shell if test -r ${PROJ_ROOT}/Makefile.system; then echo true; fi),true)
	include ${PROJ_ROOT}/Makefile.system
endif

override RM_F=rm -f
override CC=${F77}

# older g77 compiler dislikes having pedantic (because it uses -traditional in the preprocessor)
CFLAGS := $(filter-out --pedantic,${CFLAGS})
CFLAGS := $(filter-out -pedantic,${CFLAGS})

bin = conftest 
override BIN_DIR := $(shell pwd)
override INC_DIR := $(shell pwd)
override LIB_DIR := $(shell pwd)
override TMP_DIR := $(shell pwd)

override USE_PDT := 0

SRCS = conftest.F

include ${PROJ_ROOT}/Makefile.vmake
