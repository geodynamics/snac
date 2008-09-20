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

if test "${USE_OPTIONAL_MATH}" = "1" ; then
	echo "Using --options=optional_math. Complex full tensor math is enabled."
	setValue USE_OPTIONAL_MATH 1
	return 0
fi
	

# Obtain math library information
if test ! "${PETSC_CONFIG_FILE}x" = "x"; then 
	LAPACK_BLAS_LIBS__PACKAGES=`grep '^BLASLAPACK_LIB' ${PETSC_CONFIG_FILE} | cut -f 2 -d =`
	setValueWithDefault LAPACK_BLAS_LIBS "${LAPACK_BLAS_LIBS__PACKAGES}"
	
elif test ! "${CLAPACKHEADER_DIR}x" = "x"; then
		setValueWithDefault BLASLAPACK_DIR "${CLAPACKHEADER_DIR}"
		setValueWithDefault BLASLAPACK_INCDIR "${BLASLAPACK_DIR}"

		warnIfNotValidFile "${BLASLAPACK_INCDIR}/clapack.h"    "Blas-Lapack Libraries" BLASLAPACK_INCDIR BLASLAPACK_DIR

		setValueWithDefault BLASLAPACK_INCLUDES '-I${BLASLAPACK_INCDIR}'
		setValueWithDefault BLASLAPACK_LIBDIR '${BLASLAPACK_DIR}'

		case ${SYSTEM} in
			Darwin)
				setValueWithDefault BLASLAPACK_LIBS '-L${BLASLAPACK_LIBDIR} -lm -lmx';;
			*)
				setValueWithDefault BLASLAPACK_LIBS '-L${BLASLAPACK_LIBDIR} -lm';;
		esac

elif test ! "${CLAPACK_DIR}x" = "x"; then
		setValueWithDefault F2C_DIR "${CLAPACK_DIR}/F2CLIBS"
		setValueWithDefault F2C_INCDIR "${F2C_DIR}"
	
		warnIfNotValidFile "${F2C_INCDIR}/f2c.h"    "Blas-Lapack F2C Libraries" F2C_INCDIR F2C_DIR
	
		setValueWithDefault F2C_INCLUDES '-I${F2C_INCDIR}'
		setValueWithDefault F2C_LIBDIR '${F2C_DIR}/lib'
	
		case ${SYSTEM} in
			Darwin)
				setValueWithDefault F2C_LIBS '-L${F2C_LIBDIR} -lm -lmx';;
			*)
				setValueWithDefault F2C_LIBS '-L${F2C_LIBDIR} -lm';;
		esac
else
## Include a 4rd option - straight out Blas-lapack installation

	setValueWithDefault BLASLAPACK_DIR '/usr'
	setValueWithDefault BLASLAPACK_LIBDIR '${BLASLAPACK_DIR}/lib'
	case ${SYSTEM} in
		Darwin)
			setValueWithDefault LAPACK_BLAS_LIBS '-framework Accelerate';;
		*)
			setValueWithDefault LAPACK_BLAS_LIBS '-L${BLASLAPACK_LIBDIR} -llapack -lblas';;
	esac
fi
