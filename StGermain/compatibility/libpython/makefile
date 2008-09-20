##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
##
## Copyright (C), 2003, Victorian Partnership for Advanced Computing (VPAC) Ltd, 110 Victoria Street, Melbourne, 3053, Australia.
##
## Authors:
##	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
##	Patrick D. Sunter, Software Engineer, VPAC. (pds@vpac.org)
##	Luke J. Hodkinson, Computational Engineer, VPAC. (lhodkins@vpac.org)
##	Siew-Ching Tan, Software Engineer, VPAC. (siew@vpac.org)
##	Alan H. Lo, Computational Engineer, VPAC. (alan@vpac.org)
##	Raquibul Hassan, Computational Engineer, VPAC. (raq@vpac.org)
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
##
## $Id: makefile 3864 2006-10-16 09:21:03Z SteveQuenette $
##
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# obtain defaults for required variables according to system and project location, and then run the build.
ifndef PROJ_ROOT
	PROJ_ROOT=../..
endif
include ${PROJ_ROOT}/Makefile.system
include Makefile.def

ifdef HAVE_PYTHON
ifndef PYTHON_MAC_DEFAULT
ifndef PYTHON_HAVE_SO_LIB

	ifeq (true,$(shell if test "${PYTHON_STATIC_LIB}x" != "x" -a -r "${PYTHON_STATIC_LIB}"; then echo true; fi ))
		lib = ${def_lib}
		libdynamic = yes

		SRCS = ${def_srcs}

		EXTERNAL_LIBS = ${PYTHON_EXTERNAL_LIBS} ${MATH_LIBS}

		# TODO: the following would be better if it dwas done in a way that the un-ar-ing of libpython2.2.a was not done every time this makefile is called, but rather as a dependancy
		createPyLibDir = ${TMP_DIR}/lib-lib${lib}.${SO_EXT}
		testfile = ${createPyLibDir}/object.o
		ifeq (nonexistent, $(shell if test ! -r ${testfile}; then echo nonexistent; fi ))
			create := $(shell if test ! -d ${createPyLibDir}; then ${MKDIR} ${createPyLibDir}; fi; cwd=`pwd`; ${CD} ${createPyLibDir}; ${AR} -x ${def_unar}; ${CD} ${cwd} )
		endif

		newPyLibObjs := $(wildcard ${createPyLibDir}/*.o)
		EXTERNAL_OBJS += ${newPyLibObjs}
	else
		WARN := $(shell echo libpython \(proxy lib\) isn't being built as Python isn't detected. 1>&2 )
	endif
endif
endif
endif

include ${PROJ_ROOT}/Makefile.vmake
