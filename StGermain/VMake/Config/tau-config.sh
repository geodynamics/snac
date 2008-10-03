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

if test "${TAU_CONFIG_ARCH}${TAU_OPTIONS}x" = "x"; then
	# Attempt to be smart and pickup the arch and options
	NUM_ARCH=`find ${TAU_DIR} -maxdepth 1 -mindepth 1 -type d | grep -v man | grep -v include | wc -l | sed 's/^[ ]*//'`
	case ${NUM_ARCH} in
		0)
			echo "Error - No TAU_CONFIG_ARCH found in ${TAU_DIR}. Check that TAU has been installed properly."
			exit 1
			;;
		1)
			setValueWithDefault TAU_CONFIG_ARCH \
				`find ${TAU_DIR} -maxdepth 1 -mindepth 1 -type d | grep -v man | grep -v include | xargs basename`
			NUM_OPTIONS=`find ${TAU_DIR}/${TAU_CONFIG_ARCH} -type f -name "Makefile.tau*" | wc -l`
			if test "${NUM_OPTIONS}" -eq "1"; then
				setValueWithDefault TAU_OPTIONS \
				`find ${TAU_DIR}/${TAU_CONFIG_ARCH} -type f -name "Makefile.tau*"  | xargs basename | sed 's/Makefile.tau//'`
			else
				echo "Error - TAU_OPTIONS could not be found. Check that TAU has been installed properly and set TAU_OPTIONS"
			fi
			;;
		*)
			echo "Error - More than one architecture found in ${TAU_DIR}. Please select and set TAU_CONFIG_ARCH and TAU_OPTIONS."
			exit 1
			;;
	esac
fi

assertValidExecutable "$TAU_DIR/$TAU_CONFIG_ARCH/bin/tau_instrumentor" "tau_instrumentor"

setValueWithDefault TAUINSTR "${TAU_DIR}/${TAU_CONFIG_ARCH}/bin/tau_instrumentor"

setValueWithDefault TAU_INCDIR '${TAU_DIR}/include'
warnIfNotValidFile "${TAU_INCDIR}/TAU.h"    "TAU Libraries" TAU_INCDIR TAU_DIR

getVariableFromMakefile TAU_INCLUDE $TAU_DIR/$TAU_CONFIG_ARCH/lib/Makefile.tau${TAU_OPTIONS}
getVariableFromMakefile TAU_DEFS $TAU_DIR/$TAU_CONFIG_ARCH/lib/Makefile.tau${TAU_OPTIONS}
getVariableFromMakefile TAU_MPI_INCLUDE $TAU_DIR/$TAU_CONFIG_ARCH/lib/Makefile.tau${TAU_OPTIONS}
TAU_CFLAGS="${TAU_INCLUDE} ${TAU_DEFS} ${TAU_MPI_INCLUDE}"

setValueWithDefault TAU_INCLUDES "${TAU_CFLAGS}"

setValueWithDefault TAU_LIBDIR '${TAU_DIR}/${TAU_CONFIG_ARCH}/lib'

getVariableFromMakefile TAU_MPI_LIBS $TAU_DIR/$TAU_CONFIG_ARCH/lib/Makefile.tau${TAU_OPTIONS}
getVariableFromMakefile TAU_SHLIBS $TAU_DIR/$TAU_CONFIG_ARCH/lib/Makefile.tau${TAU_OPTIONS}
TAU_LFLAGS="${TAU_MPI_LIBS} ${TAU_SHLIBS}"

setValueWithDefault TAU_LIBS "${TAU_LFLAGS}"

setValueWithDefault TAU_RPATH "${RPATH_FLAG}${TAU_LIBDIR}"


