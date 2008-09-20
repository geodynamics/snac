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

# Scalable Library for Eigenvalue Problem Computations
# http://www.grycap.upv.es/slepc

parsePackageConfigOptions $@

#Note: this is a hack, because the above function doesn't work, at least on Ubuntu, given that this file is
# currently "sourced" rather than "run". --PatrickSunter, 28 Jan 2007.

export PACKAGE_OPTIONAL=1

# Check if we're allowed to use SLEPC
if test "${NOSLEPC}" = "1" ; then
	return 0
fi

case ${SYSTEM} in
        Darwin)
		setValueWithDefault SLEPC_DIR       '/sw/';;
	*)
		setValueWithDefault SLEPC_DIR       '/usr/';;
esac

if test "${SLEPC_ARCH}x" = "x"; then
	NUM_ARCH=`find $SLEPC_DIR/lib -maxdepth 1 -mindepth 1 -type d | wc -l | sed 's/^[ ]*//'`
	case ${NUM_ARCH} in
		0)
			if test "${PACKAGE_OPTIONAL}x" = "x"; then
				echo "Error: No SLEPC_ARCH found in $SLEPC_DIR/lib"
			fi
			exitWithErrorIfNotOptional;;
		1)
			setValueWithDefault SLEPC_ARCH `find $SLEPC_DIR/lib -maxdepth 1 -mindepth 1 -type d | xargs basename`;;
		*)
			if test "${PACKAGE_OPTIONAL}x" = "x"; then
				echo "Warning: More than one SLEPC_ARCH found in $SLEPC_DIR/lib and SLEPC_ARCH is unset. Please set a SLEPC_ARCH if you need this package."
			fi
			exitWithErrorIfNotOptional;;
	esac
fi

setValueWithDefault SLEPC_INCDIR    '${SLEPC_DIR}/include'
if test -r "${SLEPC_INCDIR}/slepc.h" ; then
	setValueWithDefault SLEPC_INCLUDES   '-I${SLEPC_INCDIR} -DHAVE_SLEPC'
	setValueWithDefault SLEPC_LIBDIR     '${SLEPC_DIR}/lib/${SLEPC_ARCH}'
	setValueWithDefault SLEPC_LIBFILES   '-lslepc'
	setValueWithDefault SLEPC_LIBS       '-L${SLEPC_LIBDIR} ${SLEPC_LIBFILES}'
	if test "${RPATH_FLAG}x" != "x" ; then
		setValueWithDefault SLEPC_RPATH      '${RPATH_FLAG}${SLEPC_LIBDIR}'
	fi
	setValueWithDefault HAVE_SLEPC       '1'
fi

