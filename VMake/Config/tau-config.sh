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

parsePackageConfigOptions $@

setValueWithDefault TAU_DIR '/usr/local/tau'
assertValidDirectory ${TAU_DIR} TAU TAU_DIR

if test "${TAU_CONFIG_ARCH}${TAU_MAKETYPE}x" = "x"; then
		setValueWithDefault TAU_CONFIG_ARCH \
			`tau-config | grep TAUARCH | awk -F'=' '{print $2}'`
		NUM_OPTIONS=`find ${TAU_DIR}/${TAU_CONFIG_ARCH} -type f -name "Makefile.tau*" | wc -l`
		if test "${NUM_OPTIONS}" -eq "1"; then
			setValueWithDefault TAU_MAKETYPE \
			`find ${TAU_DIR}/${TAU_CONFIG_ARCH} -type f -name "Makefile.tau*"  | xargs basename | sed 's/Makefile.tau//'`
		else
			echo "Error - TAU_MAKETYPE could not be found. Check that TAU has been installed properly and set TAU_MAKETYPE"
		fi
fi

setValueWithDefault TAU_INCDIR '${TAU_DIR}/include'
warnIfNotValidFile "${TAU_INCDIR}/TAU.h"    "TAU Libraries" TAU_INCDIR TAU_DIR

getVariableFromMakefile TAU_INCLUDE $TAU_DIR/$TAU_CONFIG_ARCH/lib/Makefile.tau${TAU_MAKETYPE}
getVariableFromMakefile TAU_DEFS $TAU_DIR/$TAU_CONFIG_ARCH/lib/Makefile.tau${TAU_MAKETYPE}
getVariableFromMakefile TAU_MPI_INCLUDE $TAU_DIR/$TAU_CONFIG_ARCH/lib/Makefile.tau${TAU_MAKETYPE}
TAU_CFLAGS="${TAU_INCLUDE} ${TAU_DEFS} ${TAU_MPI_INCLUDE}"

setValueWithDefault TAU_INCLUDES "${TAU_CFLAGS}"

setValueWithDefault TAU_LIBDIR '${TAU_DIR}/${TAU_CONFIG_ARCH}/lib'

getVariableFromMakefile TAU_MPI_LIBS $TAU_DIR/$TAU_CONFIG_ARCH/lib/Makefile.tau${TAU_MAKETYPE}
getVariableFromMakefile TAU_SHLIBS $TAU_DIR/$TAU_CONFIG_ARCH/lib/Makefile.tau${TAU_MAKETYPE}
TAU_LFLAGS="${TAU_MPI_LIBS} ${TAU_SHLIBS}"

setValueWithDefault TAU_LIBS "${TAU_LFLAGS}"

setValueWithDefault TAU_RPATH "${RPATH_FLAG}${TAU_LIBDIR}"


