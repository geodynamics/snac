#!/bin/sh

##  Copyright (C) 2005 Victorian Partnership for Advanced Computing (VPAC) Ltd
##  110 Victoria Street, Melbourne, 3053, Australia.
##
##  This library is free software; you can redistribute it and/or
##  modify it under the terms of the GNU Lesser General Public
##  License as published by the Free Software Foundation; either
##  version 2.1 of the License, or (at your option) any later version.
##
##  This library is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
##  Lesser General Public License for more details.
##
##  You should have received a copy of the GNU Lesser General Public
##  License along with this library; if not, write to the Free Software
##  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

setValueWithDefault FIND_SUBDIRS '${FIND} * -type d -follow -prune ! -name .svn'
setValueWithDefault MAKEFILE_FIND_SUBDIRS '$(shell ${FIND_SUBDIRS})'

setValueWithDefault GET_MODNAME 'basename `pwd | ${SED} s/src//g | ${SED} s/tests//g`'
setValueWithDefault MAKEFILE_GET_MODNAME '$(shell ${GET_MODNAME})'

setValueWithDefault MAKEFILE_CHECK_DEPENDENCIES '$(foreach DEP,${DEPENDS},$(shell cd ${LIB_DIR} && if test -z `find . -name ${PROJECT}${DEP}*` ; then echo Compiling dependency: ${DEP} 1>&2 && cd ${PROJ_ROOT}/src && if test ! -z `find . -type d -name ${DEP}` ; then cd `find . -type d -name ${DEP}` && make 1>&2 ; else echo Cannot find dependency: ${DEP} 1>&2 ; fi ; fi ) )'

