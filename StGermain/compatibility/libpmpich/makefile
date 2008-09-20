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
## $Id: Makefile.rules 2746 2005-03-06 00:29:00Z SteveQuenette $
##
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# obtain defaults for required variables according to system and project location, and then run the build.
ifndef PROJ_ROOT
	PROJ_ROOT=../..
endif
include ${PROJ_ROOT}/Makefile.system
include Makefile.def

# Only build this on Darwin systems
ifeq (Darwin,$(SYSTEM))
ifeq (Tiger,$(KERNEL_RELEASE)) # So far only on Tiger's we need this
	lib = ${def_lib}
	libdynamic = yes
	
	SRCS = ${def_srcs}
	
	EXTERNAL_LIBS = ${MPI_EXTERNAL_LIBS}
	
	# TODO: the following would be better if it dwas done in a way that the un-ar-ing of libpython2.2.a was not done every time this makefile is called, but rather as a dependancy
	createMPI_LibDir = ${TMP_DIR}/lib-lib${lib}.${SO_EXT}
	ifeq (nonexistent, $(shell if test "x$$(ls ${createMPI_LibDir} 2> /dev/null)" = "x"; then echo "nonexistent"; fi ))
		create := $(shell ${MKDIR} ${createMPI_LibDir}; cwd=`pwd`; for i in ${def_unar}; do curLib=`echo $$i | $(SED) 's/\///g'`; echo $$curLib; ${MKDIR} ${createMPI_LibDir}/$$curLib; ${CD} ${createMPI_LibDir}/$$curLib; ${AR} -x $$i; for j in `ls *.o`; do $(CP) $$j ${createMPI_LibDir}/$$curLib.$$j; done; done; ${CD} ${cwd} ) 
	endif

	newMPI_LibObjs := $(wildcard ${createMPI_LibDir}/*.o)
	EXTERNAL_OBJS += ${newMPI_LibObjs}
	PROJ_CLEAN += ${createMPI_LibDir}
else
	WARN := $(shell echo libpmpich \(proxy lib\) isn't being built as this system doesn't need it. 1>&2 )
endif
else
	WARN := $(shell echo libpmpich \(proxy lib\) isn't being built as this system doesn't need it. 1>&2 )
endif


include ${PROJ_ROOT}/Makefile.vmake
