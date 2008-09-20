#Finds the Absolute path to the Project Root directory
SHELL := /bin/bash
PROJ_ROOT := $(shell until test -r ./Makefile.system ; do cd .. ; done ; echo `pwd`)
include ${PROJ_ROOT}/Makefile.system

subdirs := LocalPlugin LocalPluginDep

include ${PROJ_ROOT}/Makefile.vmake



#REMOTE_PLUGIN_LIST = $(filter ${PLUGINS_DIR}/Remote%,$(PLUGINS))

#submit_plugins:
# submit them -> where to get their urls?

#remove_remote:
#	@rm -rf $(REMOTE_PLUGIN_LIST)

