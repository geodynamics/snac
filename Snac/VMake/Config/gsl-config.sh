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

# Obtain GNU Scientific Library information
if test "${GSL_DIR}x" = "x"; then
	case ${SYSTEM} in
		Linux|CYGWIN|Darwin|SunOS|ranger)
			setValueWithDefault GSL_DIR   "/usr/local";;
		OSF1)
			setValueWithDefault GSL_DIR   "/usr/local";;
		*)
			if test "${GSL_DIR}x" = "x" ; then
				echo "Error: GSL_DIR for system \"${SYSTEM}\" unknown. Set the environment variable."
				exit
			fi;;
	esac
fi
if test "${GSL_LIBDIR}x" = "x"; then
	setValueWithDefault GSL_LIBDIR   '${GSL_DIR}/lib'
fi
if test "${GSL_INCDIR}x" = "x"; then
	setValueWithDefault GSL_INCDIR   '${GSL_DIR}/include'
fi

# set depending on system. on most systems, just default to mpich
if test "${GSL_LIBFILES}x" = "x"; then
	GSL_LIBFILES='-lgsl -lgslcblas'
	resolveVariable GSL_LIBRARY_FILE "${GSL_LIBDIR}/libgsl.a"
	resolveVariable GSLCBLAS_LIBRARY_FILE "${GSL_LIBDIR}/libgslcblas.a"
	if ! test -r ${GSL_LIBRARY_FILE}; then
		resolveVariable GSL_LIBRARY_FILE "${GSL_LIBDIR}/libgsl.${SO_EXT}"
		resolveVariable GSLCBLAS_LIBRARY_FILE "${GSL_LIBDIR}/libgslcblas.${SO_EXT}"
	fi
fi
setValueWithDefault GSL_LIBFILES   "${GSL_LIBFILES}"

case $CC_TYPE in
	gnu)
		case $SYSTEM in
			Linux|CYGWIN)
				setValueWithDefault GSL_RPATH '-Xlinker -rpath -Xlinker ${GSL_LIBDIR}';;
			Darwin)
				# Darwin doesn't support RPATH... use INSTALL NAME instead
				setValueWithDefault GSL_RPATH '';;
			OSF1)
				setValueWithDefault GSL_RPATH '-Wl,-rpath,${GSL_LIBDIR}';;
			SunOS)
				setValueWithDefault GSL_RPATH '';; # Alan not conveienced it does it
			*)
				echo "Warning: GSL_RPATH for C compiler \"${CC_TYPE}\" unknown for system \"${SYSTEM}\"";;
		esac ;;
	osf)
		# NOTE: This may not work... I think only ONE rpath arguement is accepted/read.
		setValueWithDefault GSL_RPATH '-rpath ${GSL_LIBDIR}';;
	intel)
		setValueWithDefault GSL_RPATH '-Xlinker -rpath -Xlinker ${GSL_LIBDIR}';;
	sparc)
		setValueWithDefault GSL_RPATH '-R ${GSL_LIBDIR}';;
	ibmxl)
		setValueWithDefault GSL_RPATH '-R ${GSL_LIBDIR}';;
	pgi)
		setValueWithDefault GSL_RPATH '-Wl,-rpath,${GSL_LIBDIR}';;
	*)
		echo "Warning: GSL_RPATH for C compiler \"${CC_TYPE}\" unknown";;
esac

case ${SYSTEM} in
	*)
		setValueWithDefault GSL_LIBS   '-L${GSL_LIBDIR} ${GSL_LIBFILES}'
		setValueWithDefault GSL_INCLUDES '-I${GSL_INCDIR} -DHAVE_GSL=1';;
esac

warnIfNotValidFile ${GSL_DIR} gsl "GNU Scientific Library (GSL)" GSL_DIR
warnIfNotValidFile ${GSL_INCDIR}/gsl/gsl_math.h "GNU Scientific Library (GSL)" GSL_INCDIR "GSL_DIR"
warnIfNotValidFile ${GSL_LIBDIR}/libgsl.a "GNU Scientific Library (GSL)" GSL_LIBDIR "GSL_DIR" 
if ! DoCompileConftest ./VMake/Config/gsl; then
	exitWithErrorIfNotOptionalWithMessage "GNU Scientific Library (GSL)" ${GSL_DIR}
fi

setValue HAVE_GSL '1'
