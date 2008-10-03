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
## $Id: makefile 3529 2006-04-20 05:25:22Z AlanLo $
##
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# obtain defaults for required variables according to system and project location, and then run the build.
ifndef PROJ_ROOT
	PROJ_ROOT=../..
endif
include ${PROJ_ROOT}/Makefile.system
include Makefile.def

# Only build this on Darwin systems
ifeq (Darwin-mpich,${SYSTEM}-${MPI_IMPLEMENTATION})
	lib = ${def_lib}
	libdynamic = yes
	
	SRCS = ${def_srcs}
	
	EXTERNAL_LIBS = ${MPI_EXTERNAL_LIBS}
	
	# TODO: the following would be better if it dwas done in a way that the un-ar-ing of libpython2.2.a was not done every time this makefile is called, but rather as a dependancy
	createMPI_LibDir = ${TMP_DIR}/lib-lib${lib}.${SO_EXT}
	
	# Create Directory if it hasn't already been made
	create := $(shell if test ! -d ${createMPI_LibDir}; then ${MKDIR} ${createMPI_LibDir} ; fi ))

	# Store current working directory
	cwd := $(shell echo `pwd`)

	newMPI_LibObjs := $(shell \
		for i in ${def_unar}; do \
			curLib=`echo $$i | $(SED) 's/\///g'`; \
			if test ! -d ${createMPI_LibDir}/$$curLib ; then \
				${MKDIR} ${createMPI_LibDir}/$$curLib; \
				${CD} ${createMPI_LibDir}/$$curLib; \
				${AR} -x $$i; \
				for j in `ls *.o`; do \
					$(CP) $$j ${createMPI_LibDir}/$$curLib.$$j; \
				done; \
				${CD} ${createMPI_LibDir}; \
			fi; \
			for j in `ls ${createMPI_LibDir}/$$curLib.*.o`; do \
				echo $$j; \
			done; \
		done; \
		${CD} ${cwd} ) 

	EXTERNAL_OBJS += ${newMPI_LibObjs}
	PROJ_CLEAN += ${createMPI_LibDir}

	RM_LIST_TEMPS += ${createMPI_LibDir}/*.o 
	RM_LIST_TEMPS += $(shell \
		for i in ${def_unar}; do \
			curLib=`echo $$i | $(SED) 's/\///g'`; \
			echo ${createMPI_LibDir}/$$curLib; \
		done; )

else
	WARN := $(shell echo libmpi \(proxy lib\) isn't being built as this system doesn't need it. 1>&2 )
endif


include ${PROJ_ROOT}/Makefile.vmake
