
#Finds the Absolute path to the Project Root directory
SHELL := /bin/bash
PROJ_ROOT := $(shell until test -r ./Makefile.system ; do cd .. ; done ; echo `pwd`)
include ${PROJ_ROOT}/Makefile.system

# Subdirectories
#subdirs := tests

# Source Code and Header files
SRCS := $(wildcard *.c)
HDRS := $(wildcard *.h)

# What to call the plugin
modName := $(shell basename `pwd`)
mod = ${PROJECT}_${modName}

# Where to put header files
includes = ${PROJECT}/${modName}

# External Libraries and Headers
packages = MPI XML MATH

EXTERNAL_INCLUDES = -I${INC_DIR}/StGermain

include ${PROJ_ROOT}/Makefile.vmake
