#Finds the Absolute path to the Project Root directory
SHELL := /bin/bash
PROJ_ROOT := $(shell until test -r ./Makefile.system ; do cd .. ; done ; echo `pwd`)
include ${PROJ_ROOT}/Makefile.system

EXTERNAL_INCLUDES = -I${INC_DIR}/StGermain

EXTERNAL_LIBS = -L${LIB_DIR} -lStGermainBaseExtensibility -lStGermainBaseAutomation -lStGermainBaseIO -lStGermainBaseFoundation -lStGermainBaseContainer

ifdef HAVE_SOAP
	EXTERNAL_LIBS += -lStGermainRegresstor
endif

packages = MPI XML MATH

mod=LocalPlugin
SRCS=LocalPlugin.c

include ${PROJ_ROOT}/Makefile.vmake
