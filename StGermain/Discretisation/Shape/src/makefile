#Finds the Absolute path to the Project Root directory
SHELL := /bin/bash
PROJ_ROOT := $(shell until test -r ./Makefile.system ; do cd .. ; done ; echo `pwd`)
include ${PROJ_ROOT}/Makefile.system

# Source Code and Header files
SRCS := $(wildcard *.c)
HDRS := $(wildcard *.h)

# External Libraries and Headers 
packages = MPI XML 

ifdef HAVE_PYTHON
	packages += PYTHON
endif

EXTERNAL_INCLUDES += -I${INC_DIR}/StGermain
EXTERNAL_LIBS = -L${LIB_DIR} -lStGermainBase -lStGermainDiscretisationGeometry

# Work out name to call library
lib = ${PROJECT}Discretisation$(shell basename `pwd | sed s/src//g`)
libdynamic = yes
libstatic = yes

# Name of directory to place header files
includes := ${PROJECT}/Discretisation/$(shell basename `pwd | sed s/src//g`)

include ${PROJ_ROOT}/Makefile.vmake
