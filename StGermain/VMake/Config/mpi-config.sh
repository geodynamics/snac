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

#if ! test "${HAVE_MPI}x" = "x"; then
#	unset HAVE_MPI
#	return
#fi

# Obtain MPI information
case ${SYSTEM} in
	Linux|CYGWIN|Darwin|SunOS)
		setValueWithDefault MPI_DIR   "/usr/";;
	OSF1)
		setValueWithDefault MPI_DIR   "/usr/";;
	*)
		if test "${MPI_DIR}x" = "x" ; then
			echo "Warning: MPI_DIR for system \"${SYSTEM}\" unknown. Set the environment variable."
		fi;;
esac
setValueWithDefault MPI_BINDIR   '${MPI_DIR}/bin'
setValueWithDefault MPI_LIBDIR   '${MPI_DIR}/lib'
setValueWithDefault MPI_INCDIR   '${MPI_DIR}/include'
if test ! "${CC}x" = "mpiccx"; then
	setValueWithDefault MPI_INCLUDES '-I${MPI_INCDIR}'
else
	setValueWithDefault MPI_INCLUDES ' '
fi

if test "${PETSC_MPI}x" = "x"; then
	warnIfNotValidFile "${MPI_INCDIR}/mpi.h" MPI MPI_INCDIR MPI_DIR
else
	echo "Using PETSc's mpi configuration"
fi

if test -r "${MPI_INCDIR}/mpi.h"; then
	if `grep "SILICON GRAPHICS, INC" ${MPI_INCDIR}/mpi.h > /dev/null 2>&1`; then
		setValueWithDefault MPI_IMPLEMENTATION sgi
	elif `grep "OPEN_MPI" ${MPI_INCDIR}/mpi.h > /dev/null 2>&1`; then
		setValueWithDefault MPI_IMPLEMENTATION open_mpi
	elif `grep "MPICH2" ${MPI_INCDIR}/mpi.h > /dev/null 2>&1`; then
		setValueWithDefault MPI_IMPLEMENTATION mpich2
#	elif `grep "Argonne National Laboratory" ${MPI_INCDIR}/mpi.h > /dev/null 2>&1`; then
#		setValueWithDefault MPI_IMPLEMENTATION mpich
	else
		setValueWithDefault MPI_IMPLEMENTATION mpich
	fi
fi

# set depending on system. on most systems, just default to mpich
if test "${MPI_LIBFILES}x" = "x"; then
	case ${SYSTEM} in
		Linux|CYGWIN|Darwin|SunOS)
			if test "${MPI_IMPLEMENTATION}" = "mpich2"; then
				MPI_LIBFILES='-lmpich'
				setValueWithDefault MPI_LIBRARY 'mpich'
			elif test "${MPI_IMPLEMENTATION}" = "mpich"; then
				MPI_LIBFILES='-lmpich -lpmpich'
				setValueWithDefault MPI_LIBRARY 'mpich'
			else
				MPI_LIBFILES='-lmpi'
				setValueWithDefault MPI_LIBRARY 'mpi'
			fi;
			;;
		OSF1)
			MPI_LIBFILES='-lmpi -lelan';
			setValueWithDefault MPI_LIBRARY 'mpi'
			;;
		*)
			if test "${MPI_DIR}x" = "x" ; then
				echo "Warning: MPI_LIBFILES for system \"${SYSTEM}\" unknown. Set the environment variable."
			fi;;
		esac
		resolveVariable MPI_LIBRARY_FILE "${MPI_LIBDIR}/lib${MPI_LIBRARY}.a"
	if ! test -r ${MPI_LIBRARY_FILE}; then
		resolveVariable MPI_LIBRARY_FILE "${MPI_LIBDIR}/lib${MPI_LIBRARY}.${SO_EXT}"
	fi
	if test -r ${MPI_LIBRARY_FILE}; then
		if test "`${NM} ${MPI_LIBRARY_FILE} 2> /dev/null | ${GREP} -v SGI | ${GREP} -v sgi | ${GREP} -v gmon | ${GREP} _gm`x" != "x"; then
			GM_MPI=1
		fi
	fi
	if test "${GM_MPI}x" != "x"; then
		# GM MPI stuff
		if test "${GM_DIR}x" = "x"; then
			setValueWithDefault GM_DIR '/usr/gm'
		fi
		if test "${USE_64BITS}x" = "x"; then
			setValueWithDefault GM_LIBDIR '${GM_DIR}/lib'
		else
			setValueWithDefault GM_LIBDIR '${GM_DIR}/lib64'
		fi
		resolveVariable GM_LIBDIR "${GM_LIBDIR}"
		if test -d "${GM_LIBDIR}"; then
			MPI_LIBFILES="${MPI_LIBFILES} -L${GM_LIBDIR} -lgm"
		else
			echo "Warning: Using GM MPI but GM_DIR not set. Please set GM_DIR or GM_LIBDIR variable"
		echo "         Try looking for a directory named /usr/GMxxxx (x is a number)"
		fi
			export MPI_LIBFILES
	fi
	if test "${MPI_IMPLEMENTATION}" = "mpich2"; then
		MPI_BASE_LIB_LIST=`grep "^MPI_OTHERLIBS" ${MPI_DIR}/bin/mpicc 2> /dev/null | cut -f 2 -d = | sed 's/\"//g'`;
		MPI_LIBFILES="${MPI_LIBFILES} ${MPI_BASE_LIB_LIST}"
	elif test "${MPI_IMPLEMENTATION}" = "mpich"; then
		MPI_BASE_LIB_LIST=`grep "^BASE_LIB_LIST" ${MPI_DIR}/bin/mpicc 2> /dev/null | cut -f 2 -d = | sed 's/\"//g'`;
		MPI_LIBFILES="${MPI_LIBFILES} ${MPI_BASE_LIB_LIST}"
	fi
fi
setValueWithDefault MPI_LIBFILES   "${MPI_LIBFILES}"

case $CC_TYPE in
	gnu)
		case $SYSTEM in
			Linux|CYGWIN)
				setValueWithDefault MPI_RPATH '-Xlinker -rpath -Xlinker ${MPI_LIBDIR}';;
			Darwin)
				# Darwin doesn't support RPATH... use INSTALL NAME instead
				setValueWithDefault MPI_RPATH '';;
			OSF1)
				setValueWithDefault MPI_RPATH '-Wl,-rpath,${MPI_LIBDIR}';;
			SunOS)
				setValueWithDefault MPI_RPATH '';; # Alan not conveienced it does it
			*)
				echo "Warning: MPI_RPATH for C compiler \"${CC_TYPE}\" unknown for system \"${SYSTEM}\"";;
		esac ;;
	osf)
		# NOTE: This may not work... I think only ONE rpath arguement is accepted/read.
		setValueWithDefault MPI_RPATH '-rpath ${MPI_LIBDIR}';;
	intel)
		setValueWithDefault MPI_RPATH '-Xlinker -rpath -Xlinker ${MPI_LIBDIR}';;
	sparc)
		setValueWithDefault MPI_RPATH '-R ${MPI_LIBDIR}';;
	ibmxl)
		setValueWithDefault MPI_RPATH '-R ${MPI_LIBDIR}';;
	*)
		echo "Warning: MPI_RPATH for C compiler \"${CC_TYPE}\" unknown";;
esac

case ${SYSTEM} in
	Darwin)
		if test "${STGERMAIN_LIBDIR}x" = "x"; then
			setValueWithDefault MPI_LIBS   '-L${LIB_DIR} -l${MPI_LIBRARY} -L${MPI_LIBDIR} ${MPI_LIBFILES} -L/sw/lib ${CC_FORTRAN_LFLAGS}'
		else
			setValueWithDefault MPI_LIBS   '-L${STGERMAIN_LIBDIR} -l${MPI_LIBRARY} -L${MPI_LIBDIR} ${MPI_LIBFILES} -L/sw/lib ${CC_FORTRAN_LFLAGS}'
		fi;;
	*)
		setValueWithDefault MPI_LIBS   '-L${MPI_LIBDIR} ${MPI_LIBFILES}';;
esac

if test "${MPI_EXTERNAL_LIBS}x" = "x"; then
	case ${SYSTEM} in
		Darwin)
			setValueWithDefault MPI_EXTERNAL_LIBS '${CC_FORTRAN_LFLAGS}'
	esac
fi

case ${SYSTEM} in
	Linux|CYGWIN|Darwin|SunOS)
		setValueWithDefault MPI_RUN_COMMAND   'mpirun';;
	OSF1)
		setValueWithDefault MPI_RUN_COMMAND   'prun';;
	*)
		if test "${MPI_RUN_COMMAND}x" = "x" ; then
			echo "Warning: MPI_RUN_COMMAND for system \"${SYSTEM}\" unknown. Set the environment variable."
		fi;;
esac

if test "${PETSC_MPI}x" = "x"; then
	if test "${MPI_RUN}x" = "x"; then
		warnValidExecutable "${MPI_BINDIR}/${MPI_RUN_COMMAND}" mpi_run MPI_RUN "MPI_DIR & MPI_RUN_COMMAND"
	else
		warnValidExecutable "${MPI_RUN}" mpi_run MPI_RUN "The MPI_RUN you have set does not exists or is not executable"
	fi
else
	warnValidExecutable "${MPI_RUN}" mpi_run MPI_RUN "PETSc's MPIRUN"
fi

setValueWithDefault MPI_RUN   '${MPI_BINDIR}/${MPI_RUN_COMMAND}'

case ${SYSTEM} in
	Linux|CYGWIN|Darwin|SunOS)
		setValueWithDefault MPI_NPROC   '-np';;
	OSF1)
		setValueWithDefault MPI_NPROC   '-n';;
	*)
		if test "${MPI_NPROC}x" = "x" ; then
			echo "Warning: MPI_NPROC for system \"${SYSTEM}\" unknown. Set the environment variable."
		fi;;
esac

# Only add MPI_MACHINES to Makefile.system if specified, otherwise leave omitted
if test "${MPI_MACHINES}x" != "x"; then
	setValueWithDefault MPI_MACHINES "${MPI_MACHINES}"
fi

if ! DoCompileConftest ./VMake/Config/mpi; then
	echo "*** Error: Configuration has failed because mpi installation ${MPI_DIR} is invalid"
	exit 1
fi

setValue HAVE_MPI '1'
