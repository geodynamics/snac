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

# Check if we're allowed to use PETSC
if test "${NOPETSC}" = "1" ; then
	return 0
fi

# Obtain PetSc information. 
setValueWithDefault PETSC_DIR '/usr/'
PETSC_DIR_CHECK=`warnIfNotValidFile ${PETSC_DIR} PetSc PETSC_DIR`

if test "${PETSC_DIR_CHECK}x" != "x"; then
	echo $PETSC_DIR_CHECK
else	
	setValueWithDefault BOPT 'O'

	setValueWithDefault PETSC_INCDIR '${PETSC_DIR}/include'
	assertValidFile ${PETSC_INCDIR}/petsc.h PETSc PETSC_INCDIR "PETSC_DIR"
	assertValidFile ${PETSC_INCDIR}/petscversion.h PETSc PETSC_INCDIR "PETSC_DIR"

	PETSC_VERSION_MAJOR=`grep "#define[ \t]*PETSC_VERSION_MAJOR[ \t]*" ${PETSC_INCDIR}/petscversion.h | sed 's/#define[ \t]*PETSC_VERSION_MAJOR[ \t]*//'`
	PETSC_VERSION_MINOR=`grep "#define[ \t]*PETSC_VERSION_MINOR[ \t]*" ${PETSC_INCDIR}/petscversion.h | sed 's/#define[ \t]*PETSC_VERSION_MINOR[ \t]*//'`
	PETSC_VERSION_SUBMINOR=`grep "#define[ \t]*PETSC_VERSION_SUBMINOR[ \t]*" ${PETSC_INCDIR}/petscversion.h | sed 's/#define[ \t]*PETSC_VERSION_SUBMINOR[ \t]*//'`

	# Equals 1 if release, 0 if nightly build
	PETSC_VERSION_RELEASE=`grep "#define[ \t]*PETSC_VERSION_RELEASE[ \t]*" ${PETSC_INCDIR}/petscversion.h | sed 's/#define[ \t]*PETSC_VERSION_RELEASE[ \t]*//'`
	setValueWithDefault PETSC_VERSION_RELEASE "${PETSC_VERSION_RELEASE}"

	echo PETSc version detected as $PETSC_VERSION_MAJOR.$PETSC_VERSION_MINOR.$PETSC_VERSION_SUBMINOR

	if test "$PETSC_VERSION_MINOR" -lt "3"; then
		ARCH_DIR=$PETSC_DIR/lib/lib${BOPT}
	else
		ARCH_DIR=$PETSC_DIR/lib
	fi

	if test "${PETSC_ARCH}x" = "x"; then
		NUM_ARCH=`find $ARCH_DIR -maxdepth 1 -mindepth 1 -type d | wc -l | sed 's/^[ ]*//'`
		case ${NUM_ARCH} in
			0)
				echo "Error: No PETSC_ARCH found in $PETSC_DIR/lib"
				exitWithErrorIfNotOptional
				;;
			1)
				setValueWithDefault PETSC_ARCH `find $ARCH_DIR -maxdepth 1 -mindepth 1 -type d | xargs basename`;;
			*)
				echo "Error: More than one PETSC_ARCH found in $PETSC_DIR/lib and PETSC_ARCH is unset. Please set a PETSC_ARCH."
				exitWithErrorIfNotOptional
				;;
		esac
	fi

	setValueWithDefault PETSC_BINDIR '${PETSC_DIR}/bin'

	case ${PETSC_VERSION_MINOR} in
		3)
			setValueWithDefault PETSC_LIBDIR '${PETSC_DIR}/lib/${PETSC_ARCH}';;
		*)
			setValueWithDefault PETSC_LIBDIR '${PETSC_DIR}/lib/lib${BOPT}/${PETSC_ARCH}';;
	esac

	assertValidFile ${PETSC_LIBDIR} PETSc PETSC_LIBDIR "PETSC_DIR & BOPT & PETSC_ARCH"

	setValueWithDefault PETSC_BMAKEDIR '${PETSC_DIR}/bmake/${PETSC_ARCH}'

	case ${PETSC_VERSION_MINOR} in
		3)
			setValueWithDefault PETSC_CONFIG_FILE '${PETSC_BMAKEDIR}/petscconf'
			setValueWithDefault PETSC_VARIABLES_FILE '${PETSC_BMAKEDIR}/petscconf'
			;;
		*)
			# Assume < 2.2
			setValueWithDefault PETSC_CONFIG_FILE '${PETSC_BMAKEDIR}/packages'
			setValueWithDefault PETSC_VARIABLES_FILE '${PETSC_BMAKEDIR}/variables'
			;;
	esac

	PETSC_BMAKEDIR_RESULT=`warnIfNotValidFile ${PETSC_CONFIG_FILE} PetSc PETSC_BMAKEDIR "PETSC_DIR & PETSC_ARCH"`

	if test "${USE_OPTIMISED}x" != "x"; then
		case ${PETSC_VERSION_MINOR} in
			3)
				getVariableFromMakefile CONFIGURE_OPTIONS $PETSC_CONFIG_FILE
				PETSC_USES_DEBUG=`echo $CONFIGURE_OPTIONS | grep '\-\-with\-debugging=0'`
				if test "${PETSC_USES_DEBUG}x" = "x"; then
					echo "Warning - Configuring for optimised build, but PETSC is in debug mode"
				fi
				;;
			*)
				# Don't know the check for it yet
				;;
		esac
	fi
	
	setValueWithDefault PETSC_INCLUDES '-I${PETSC_INCDIR} -I${PETSC_BMAKEDIR}'

	if test "${PETSC_BMAKEDIR_RESULT}x" != "x"; then
		echo $PETSC_BMAKEDIR_RESULT
	else	
		if test "${MPI_DIR}x" = "x"; then
			# MPI hasn't been set, try to get PETSc's
			
			if test "$PETSC_VERSION_MINOR" -lt "3"; then
				PETSC_MPI_INCLUDE=`grep '^MPI_INCLUDE' ${PETSC_CONFIG_FILE} | cut -f 2 -d = | sed 's/ //g'`
				setValueWithDefault MPI_INCLUDES "${PETSC_MPI_INCLUDE}"
		
				PETSC_MPI_LIB=`grep '^MPI_LIB' ${PETSC_CONFIG_FILE} | cut -f 2 -d = | sed 's/ //g'`
				setValueWithDefault MPI_LIBS "${PETSC_MPI_LIB}"
		
				PETSC_MPIRUN=`grep '^MPIRUN' ${PETSC_CONFIG_FILE} | cut -f 2 -d = | sed 's/ //g'`
				setValueWithDefault MPI_RUN "${PETSC_MPIRUN}"

				if test "${PETSC_MPI_INCLUDE}${PETSC_MPI_LIB}x" != "x"; then
					setValueWithDefault PETSC_MPI '1'
					setValueWithDefault MPI_DIR `echo $PETSC_MPIRUN | xargs dirname | xargs dirname`
				fi
			else
				TMP_CC=$CC
				getVariableFromMakefile CC $PETSC_CONFIG_FILE
				if echo $CC | grep mpicc > /dev/null 2>&1; then
					setValueWithDefault MPI_INCLUDES `$CC -show -c | sed 's/^.* //' | sed 's/ \-c//'`
					setValueWithDefault MPI_LIB `$CC -show | sed 's/^.*\-L/\-L/'`
				else
					TMP_MPI_LIB=$MPI_LIB
					TMP_MPI_INCLUDE=$MPI_INCLUDE
					getVariableFromMakefile MPI_INCLUDE $PETSC_CONFIG_FILE
					getVariableFromMakefile MPI_LIB $PETSC_CONFIG_FILE
					setValueWithDefault MPI_INCLUDES "$MPI_INCLUDE"
					setValueWithDefault MPI_LIBS "$MPI_LIB"
					MPI_LIB=$TMP_MPI_LIB
					MPI_INCLUDE=$TMP_MPI_INCLUDE
				fi
				getVariableFromMakefile MPIRUN $PETSC_CONFIG_FILE
				setValueWithDefault MPI_RUN "$MPIRUN"
				setValueWithDefault MPI_DIR `echo $MPIRUN | xargs dirname | xargs dirname`

				setValueWithDefault PETSC_MPI '1'
				CC=$TMP_CC
			fi
		fi
		
		BLAS_LIB__PACKAGES=`grep '^BLAS_LIB' ${PETSC_CONFIG_FILE} | cut -f 2 -d =`
		setValueWithDefault BLAS_LIB "${BLAS_LIB__PACKAGES}"

		LAPACK_LIB__PACKAGES=`grep '^LAPACK_LIB' ${PETSC_CONFIG_FILE} | cut -f 2 -d =`
		setValueWithDefault LAPACK_LIB "${LAPACK_LIB__PACKAGES}"

		LAPACK_BLAS_LIBS__PACKAGES=`grep '^BLASLAPACK_LIB' ${PETSC_CONFIG_FILE} | cut -f 2 -d =`
		setValueWithDefault LAPACK_BLAS_LIBS "${LAPACK_BLAS_LIBS__PACKAGES}"

		PETSC_X11_LIBS__PACKAGES=`grep '^X11_LIB' ${PETSC_CONFIG_FILE} | cut -f 2 -d =`
		setValueWithDefault PETSC_X11_LIBS "${PETSC_X11_LIBS__PACKAGES}"

		CLINKER_SLFLAG__FLAG=`grep '^C_CLINKER_SLFLAG' ${PETSC_VARIABLES_FILE} | cut -f 2 -d =`
		setValueWithDefault CLINKER_SLFLAG "${CLINKER_SLFLAG__FLAG}"

		MATLAB_LIB__PACKAGES=`grep '^MATLAB_LIB' ${PETSC_CONFIG_FILE} | cut -f 2 -d =`
		setValueWithDefault MATLAB_LIB "${MATLAB_LIB__PACKAGES}"

		MUMPS_LIB__PACKAGES=`grep '^MUMPS_LIB' ${PETSC_CONFIG_FILE} | cut -f 2 -d = | sed 's/ //g'`

		if test ! "${MUMPS_LIB__PACKAGES}x" = "x"; then
			# We are using MUMPS, so don't ignore white spaces 
			MUMPS_LIB__PACKAGES=`grep '^MUMPS_LIB' ${PETSC_CONFIG_FILE} | cut -f 2 -d =`

			if test "${BLACS_LIB}x" = "x"; then
				echo "Warning. You are using MUMPS and BLACS_LIB is unset." \
					"List the BLACS library files with full path in BLACS_LIB to correctly compile."
			fi
			if test "${SCALAPACK_LIB}x" = "x"; then
				echo "Warning. You are using MUMPS and SCALAPACK_LIB is unset." \
					"Set the SCALAPACK library file to correctly compile."
			fi
			eval "MUMPS_LIB__PACKAGES=\"${MUMPS_LIB__PACKAGES} ${SCALAPACK_LIB} ${BLACS_LIB}\""
			setValueWithDefault MUMPS_LIB "${MUMPS_LIB__PACKAGES}"
		fi
	fi

	#Here we need to test which solver to use. Base it off the changes file in the docs dir
	if test "2" -le "`grep '#define PETSC_VERSION_MINOR' ${PETSC_INCDIR}/petscversion.h | tr -s ' ' | cut -d ' ' -f 3`"; then
		setValueWithDefault PETSC_SOLVER_LIB 'petscksp'
	else	
		setValueWithDefault PETSC_SOLVER_LIB 'petscsles'
	fi

	setValueWithDefault PETSC_LIBFILES '-l${PETSC_SOLVER_LIB} -lpetscdm -lpetscmat -lpetscvec -lpetsc -lpetscsnes ${MUMPS_LIB} ${LAPACK_BLAS_LIBS} ${MATLAB_LIB} ${PETSC_X11_LIBS} ${CC_FORTRAN_LFLAGS}'
	
	case ${SYSTEM} in
		Darwin)
			if test "${STGERMAIN_LIBDIR}x" = "x"; then
				setValueWithDefault PETSC_LIBS '-L${LIB_DIR} -L${PETSC_LIBDIR} -lpetsc -lpetscsnes ${LAPACK_BLAS_LIBS} ${PETSC_X11_LIBS} ${CC_FORTRAN_LFLAGS}'
			else 
				setValueWithDefault PETSC_LIBS '-L${STGERMAIN_LIBDIR} -L${PETSC_LIBDIR} -lpetsc -lpetscsnes ${LAPACK_BLAS_LIBS} ${PETSC_X11_LIBS} ${CC_FORTRAN_LFLAGS}'
			fi;;
		*)
			setValueWithDefault PETSC_LIBS '-L${PETSC_LIBDIR} ${PETSC_LIBFILES}';;
	esac

	case $CC_TYPE in
		gnu)
			case $SYSTEM in
				Linux|CYGWIN)
					setValueWithDefault PETSC_RPATH '-Xlinker -rpath -Xlinker ${PETSC_LIBDIR}';;
				Darwin)
					# Darwin doesn't support RPATH... use INSTALL NAME instead
					setValueWithDefault PETSC_RPATH '';;
				OSF1)
					setValueWithDefault PETSC_RRPATH '-Wl,-R,${PETSC_LIBDIR}';;
				SunOS)
					setValueWithDefault PETSC_RPATH '';; # Alan not conveinced it does it 
				*)
					echo "Warning: PETSC_RPATH for C compiler \"${CC_TYPE}\" unknown for system \"${SYSTEM}\"";;
			esac ;;
		osf)
			# NOTE: This may not work... I think only ONE rpath arguement is accepted/read.
			setValueWithDefault PETSC_RPATH '-rpath ${PETSC_LIBDIR}';;
		intel)
			setValueWithDefault PETSC_RPATH '-Xlinker -rpath -Xlinker ${PETSC_LIBDIR}';;
		sparc)
			setValueWithDefault PETSC_RPATH '-R ${PETSC_LIBDIR}';;
		ibmxl)
			setValueWithDefault PETSC_RPATH '-R ${PETSC_LIBDIR}';;
		*)
			echo "Warning: PETSC_RPATH for C compiler \"${CC_TYPE}\" unknown";;
	esac
fi	


# Need to source this dependency to make proper check
. ./VMake/Config/mpi-config.sh

if ! DoCompileConftest ./VMake/Config/PETSc; then
	echo "*** Error: Configuration has failed because PETSc installation ${PETSC_DIR}, ${PETSC_ARCH} is invalid"
	exitWithErrorIfNotOptional
fi

setValue HAVE_PETSC '1'
