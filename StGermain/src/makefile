##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
##
## Victorian Partnership for Advanced Computing (VPAC) Ltd, Australia
## (C) 2003 All Rights Reserved
##
## <copyright-release-tag>
##
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


#Finds the Absolute path to the Project Root directory
SHELL := /bin/bash
PROJ_ROOT := $(shell until test -r ./Makefile.system ; do cd .. ; done ; echo `pwd`)
include ${PROJ_ROOT}/Makefile.system

bin = ${PROJECT}

SRCS = $(wildcard *.c)

# Necessary to load all my modules in one shebang!
EXTERNAL_LIBS =  -lStGermain -lStGermainDiscretisation -lStGermainBase ${EXPORT_DYNAMIC_LFLAGS}

EXTERNAL_INCLUDES = -DMODULE_EXT=\"${MODULE_EXT}\"

#packages = PETSC MPI XML MATH DL HYPRE
packages = MPI XML MATH DL HYPRE
		
ifdef HAVE_PYTHON
	packages += PYTHON
endif

include ${PROJ_ROOT}/Makefile.vmake
