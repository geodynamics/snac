#!/bin/sh
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
## Role:
##	Obtain the system configuration.
##
## Comments:
##	The X_SH variables are used when the value of a variable is needed within
##	the shell script.
##
## $Id: configure.sh 2586 2005-01-28 00:08:55Z AlanLo $
##
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# Be Bourne compatible
if test -n "${ZSH_VERSION+set}" && (emulate sh) >/dev/null 2>&1; then
	emulate sh
	NULLCMD=:
elif test -n "${BASH_VERSION+set}" && (set -o posix) >/dev/null 2>&1; then
	set -o posix
fi

if test "${PROJ_ROOT}x" = "x"; then
	PROJ_ROOT=`pwd`
	export PROJ_ROOT
fi

# Store the configure command in the start of the configure log
# Named Config.log (capital C) for tab-complete nice-ness
echo "Configure command: " $0 $* > Config.log

if test "${RUN_TEE}x" = "x"; then
	echo "Doing general configuration in dir ${PROJ_ROOT}"
else
	echo "Doing general configuration in dir ${PROJ_ROOT}" >> Config.log
fi

#Get rid of all aliases (which screw up which's output)

# for csh
unalias * 2> /dev/null
# for sh and bash
unalias -a 2> /dev/null

. './build-functions.sh'

# Test for tee first so we can use it to create the Config.log
if test "${TEE}x" = "x"; then
	TEE=`${WHICH} tee 2> /dev/null`
	if whichFailed "${TEE}"; then
		TEE="usr/bin/tee"
	fi
	assertValidExecutable "${TEE}" tee 
	export TEE
else
	assertValidExecutableInput "${TEE}" tee
fi

if test "${RUN_TEE}x" = "x"; then
	export RUN_TEE=1
	$0 $* 2>&1 | ${TEE} -a Config.log
	exit
fi

# Obtain system information
if test "${UNAME}x" = "x"; then
	UNAME=`${WHICH} uname 2> /dev/null`
	if whichFailed "${UNAME}"; then
		UNAME="/bin/uname"
		if test ! -x ${UNAME}; then
			UNAME="/usr/bin/uname"
		fi	
	fi
	export UNAME
	assertValidExecutable "${UNAME}" uname
else
	assertValidExecutableInput "${UNAME}" uname
fi

# machine
if test "${UNAME_M}x" = "x"; then
	UNAME_M="${UNAME} -m"
	export UNAME_M
fi
# kernel release
if test "${UNAME_R}x" = "x"; then
	UNAME_R="${UNAME} -r"
	export UNAME_R
fi
if test "${MACHINE}x" = "x"; then
	case `$UNAME_M` in 
		*i686*) 
			MACHINE=i686;; 
		*x86_64*) # optron
			MACHINE=x86_64;; 
		*ia64*) # itanium
			MACHINE=ia64;; 
		*alpha*)
			MACHINE=alpha;;
		*Power*Macintosh*)
			MACHINE=power;;
		*ppc64*) # Power5
			MACHINE=ppc64;;
		*) 
			MACHINE=Unknown;; 
	esac
	export MACHINE
fi
if test "${UNAME_S}x" = "x"; then
	UNAME_S="${UNAME} -s"
	export UNAME_S
fi
if test "${SYSTEM}x" = "x"; then
	case `$UNAME_S` in 
		*Linux*) 
			SYSTEM=Linux;; 
		*CYGWIN*) 
			SYSTEM=CYGWIN;; 
		*OSF1*) 
			SYSTEM=OSF1;; 
		*Darwin*)
			SYSTEM=Darwin;;
		*SunOS*)
			SYSTEM=SunOS;;
		*) 
			SYSTEM=Unknown;; 
	esac
	export SYSTEM
fi
if test "${KERNEL_RELEASE}x" = "x"; then
	KERNEL_RELEASE=`${UNAME_R}`
	# The kernel release is general more important for macs so give meaningful name
	if test "${SYSTEM}" = "Darwin"; then
		case ${KERNEL_RELEASE} in
			7.*)
				KERNEL_RELEASE=Panther;;
			8.*)
				KERNEL_RELEASE=Tiger;;
			*)
				KERNEL_RELEASE=OtherCats;;
		esac
	fi
fi

# Obtain system tool information
if test "${MAKE}x" = "x"; then
	MAKE=`${WHICH} make 2> /dev/null`
	if whichFailed "$MAKE"; then
		MAKE="/usr/bin/make"
	fi	
	assertValidExecutable "${MAKE}" make
	if ! ${MAKE} --version | grep GNU > /dev/null 2>&1; then
		echo "Warning: GNU make not found, this is the recommended make implementation"
	fi	
	export MAKE
else
	assertValidExecutableInput "${MAKE}" make
fi

if test "${SH}x" = "x"; then
	SH=`${WHICH} sh 2> /dev/null`
	if whichFailed "${SH}"; then
		SH="/bin/sh"
	fi
	assertValidExecutable "${SH}" sh
	export SH
else
	assertValidExecutableInput "${SH}" sh
fi

if test "${FIND}x" = "x"; then
	FIND=`${WHICH} find 2> /dev/null`
	if whichFailed "${FIND}"; then
		FIND="/usr/bin/find"
	fi	
	assertValidExecutable "${FIND}" find
	export FIND
else
	assertValidExecutableInput "${FIND}" find
fi

if test "${RM}x" = "x"; then
	RM=`${WHICH} rm 2> /dev/null`
	if whichFailed "${RM}"; then
		RM="/bin/rm"
	fi	
	assertValidExecutable "${RM}" rm
	export RM
else
	assertValidExecutableInput "${RM}" rm
fi

if test "${RM_F}x" = "x"; then
	RM_F="${RM} -rf"
	export RM_F
fi

if test "${CD}x" = "x"; then
	CD="cd"
	export CD
fi

if test "${CP}x" = "x"; then
	CP=`${WHICH} cp 2> /dev/null`
	if whichFailed "${CP}"; then
		CP="/bin/cp"
	fi
	assertValidExecutable "${CP}" cp
	export CP
else
	assertValidExecutableInput "${CP}" cp
fi

if test "${CP_F}x" = "x"; then
	CP_F="${CP} -f"
	export CP_F
fi

if test "${MV}x" = "x"; then
	MV="/bin/mv"
	if test -x ${MV}; then
		export MV
	else
		echo "Error: mv not found. Set the MV environment variable."
		exit
	fi
fi

if test "${MV_F}x" = "x"; then
	MV_F="${MV} -f"
	export MV_F
fi

if test "${MKDIR}x" = "x"; then
	MKDIR="mkdir -p"
	export MKDIR
fi
if test "${SED}x" = "x"; then
	SED="/bin/sed"
	if test -x ${SED}; then
		export SED
	else
		SED="/usr/bin/sed"
		if test -x ${SED}; then
			export SED
		else
			echo "Error: sed not found. Set the SED environment variable."
			exit
		fi
	fi
fi
if test "${LN}x" = "x"; then
	LN="/bin/ln"
	if test -x ${LN}; then
		export LN
	else
		echo "Error: ln not found. Set the LN environment variable."
		exit
	fi
fi
if test "${LN_S}x" = "x"; then
	LN_S="${LN} -fs"
	export LN_S
fi

if test "${NM}x" = "x"; then
	NM=`${WHICH} nm 2> /dev/null`
	if whichFailed "${NM}"; then
		NM="/usr/bin/nm"
	fi
	assertValidExecutable "${NM}" nm
	export NM
else
	assertValidExecutable "${NM}" nm
fi

if test "${LS}x" = "x"; then
	LS=`${WHICH} ls 2> /dev/null`
	if whichFailed "${LS}"; then
		LS="/bin/ls"
	fi
	assertValidExecutable "${LS}" ls
	export LS
else
	assertValidExecutableInput "${LS}" ls
fi

# If this is a mac, having colours for 'ls' can mess up comparisons between strings given from the output of 'ls'
# e.g. if test `ls $file` == /expected/filename ; then ...
# If we unset these environment variables this will turn off colours for the mac
unset CLICOLOR
unset CLICOLOR_FORCE

parseOptions() {
	while test $# -gt 0; do
		case $1 in
			tau)
				USE_TAU=1;
				export USE_TAU=1;;
			pdt)
				USE_PDT=1;
				export USE_PDT;;
			optimised)
				USE_OPTIMISED=1;
				export USE_OPTIMISED;;
			debug)
				USE_DEBUG=1;
				export USE_DEBUG;;
			profile)
				USE_PROFILE=1;
				export USE_PROFILE;;
			macro_as_func)
				USE_MACRO_AS_FUNC=1;
				export USE_MACRO_AS_FUNC;;
			cautious)
				USE_CAUTIOUS=1;
				export USE_CAUTIOUS;;
			memory_stats)
				USE_MEMORY_STATS=1;
				export USE_MEMORY_STATS;;
			separate_build)
				USE_SEPARATE_BUILD=1;
				export USE_SEPARATE_BUILD;;
			version)
				USE_VERSION=1;
				export USE_VERSION;;
			nolog)
				USE_NOLOG=1;
				export USE_NOLOG;;
			64bits)
				USE_64BITS=1
				export USE_64BITS;;
			mpirecord)
				USE_MPIRECORD=1
				export USE_MPIRECORD;;
			run_long_tests)
				USE_RUN_LONG_TESTS=1
				export USE_RUN_LONG_TESTS;;
			optional_math)
				USE_OPTIONAL_MATH=1
				export USE_OPTIONAL_MATH;;
			*)
				echo "Unknown option \"$1\". Exiting."
				exit;;
		esac
		
		shift
	done
}

parseArguements() {
	OPTIONS=""
	while test $# -gt 0; do
		# Does the command line arguement take the right form (i.e --argumentName=value)?
		case "$1" in
			--*=*)
				optarg="$1";;
			--*)
				optarg="$1";;
			*)
				optarg= ;;
		esac
		
		# If the above is true, then check if it any of the valid arguements
		case $optarg in
			--options=*)
				options=`echo $1 | $SED 's/[-_a-zA-Z0-9]*=//' | $SED 's/,/ /g'`
				
				# Save the "," delimited list of options
				optionsTmp=${options}            
				
				# Rebuild the options into a nice string to append to dir

				# First remove options which should not appear on dir string
				optionsTmp="`echo ${optionsTmp} | $SED 's/separate_build//g'`" # Do not include this option string
				optionsTmp="`echo ${optionsTmp} | $SED 's/version//g'`"        # Do not include this option string
				optionsTmp="`echo ${optionsTmp} | $SED 's/nolog//g'`"        # Do not include this option string

				# Reduce multi-spaces to one created by the removal of the above option text
				optionsTmp="`echo ${optionsTmp} | $SED 's/ [ ]*/ /g'`"
				OPTIONS="`echo ${optionsTmp} | $SED 's/ /-/g'`"

				parseOptions $options;;
			--prefix=*)
				EXPORT_DIR=`echo $1 | $SED 's/[-_a-zA-Z0-9]*=//'`;;
			--blddir=*)
				BLD_DIR=`echo $1 | $SED 's/[-_a-zA-Z0-9]*=//'`;;
			--without-*)
				PACKAGE_NAME=`echo $1 | $SED 's/--without-//' | ${TR} '[:lower:]' '[:upper:]'`
				ENVIRONMENT_VAR=NO${PACKAGE_NAME}
				eval "$ENVIRONMENT_VAR=1" 2> /dev/null &&
				export $ENVIRONMENT_VAR;;
			--disable-shared)
				# Hardcode without rule for now. Make generic disable in VMake2
				NOSHARED=1
				export NOSHARED;;
			*)
				echo "Warning: Unknown command line option \"$1\".";;
		esac
		
		shift
	done
	export OPTIONS
}

# Obtain options
parseArguements $@

determineProjectVersion() {
	if test "${USE_VERSION}x" != "x"; then
		# Retrive version value
		if test -d .svn; then
			# This implies we in a checkout copy, and not in an export tarball
			VERSION=`svn info | grep Revision | cut -f 2 -d : | $SED 's/ //g'`
			echo ${VERSION} > VERSION
		else
			if test -r VERSION; then
				VERSION=`cat VERSION`
			fi
		fi
	fi
}

determineProjectVersion

if test "${BLD_DIR}x" = "x"; then
	# Add version number iff version option given and a value was retrived
	if test "${USE_VERSION}x" = "x"; then
		BLD_DIR=${PROJ_ROOT}/build
	else
		if test "${VERSION}x" != "x"; then
			BLD_DIR=${PROJ_ROOT}/build-v${VERSION}
		fi
	fi
	
	# Add argumented options
	if test "${OPTIONS}x" != "x"; then
		BLD_DIR=${BLD_DIR}-${OPTIONS}
	fi
	
	export BLD_DIR
	BLD_BASE=${PROJ_ROOT}	# The build base dir for separate_build option
else
	BLD_BASE=${BLD_DIR}	# The build base dir for separate_build option
fi

if test "${EXPORT_DIR}x" = "x"; then
	EXPORT_DIR=${BLD_DIR}
	EXPORT_BASE=${BLD_DIR}	# The export base dir for separate_build option
	export EXPORT_DIR EXPORT_BASE
else
	EXPORT_BASE=${EXPORT_DIR}
	export EXPORT_BASE
fi
if test "${TMP_DIR}x" = "x"; then
	TMP_DIR=${BLD_DIR}/tmp
	export TMP_DIR
fi
if test "${BIN_DIR}x" = "x"; then
	BIN_DIR=${EXPORT_DIR}/bin
	export BIN_DIR
fi
if test "${LIB_DIR}x" = "x"; then
	LIB_DIR=${EXPORT_DIR}/lib
	export LIB_DIR
fi
if test "${INC_DIR}x" = "x"; then
	INC_DIR=${EXPORT_DIR}/include
	export INC_DIR
fi
if test "${TST_DIR}x" = "x"; then
	TST_DIR=${BLD_DIR}/tests
	export TST_DIR
fi
if test "${PYB_DIR}x" = "x"; then
	PYB_DIR=${EXPORT_DIR}/Python
	export PYB_DIR
fi
if test "${PYC_DIR}x" = "x"; then
	PYC_DIR=${EXPORT_DIR}/Python
	export PYC_DIR
fi
if test "${SHARE_DIR}x" = "x"; then
	SHARE_DIR=${EXPORT_DIR}/share
	export SHARE_DIR
fi

if test "${GREP}x" = "x"; then
	GREP=`${WHICH} grep 2> /dev/null`
	if whichFailed "${GREP}"; then
		GREP="/bin/grep"
	fi	
	assertValidExecutable "${GREP}" grep
	export GREP
else
	assertValidExecutableInput "${GREP}" grep
fi

if test "${AR}x" = "x"; then
	AR="/usr/bin/ar"
	if test -x ${AR}; then
		export AR
	else
		echo "Error: ar not found. Set the AR environment variable."
		exit
	fi
fi
if test "${AR_C}x" = "x"; then
	AR_C='${AR} -cru'
	export AR_C
fi
if test "${RANLIB}x" = "x"; then
	RANLIB="/usr/bin/ranlib"
	if test -x ${RANLIB}; then
		export RANLIB
	else
		echo "Error: ranlib not found. Set the RANLIB environment variable."
		exit
	fi
fi

if test "${CC}x" = "x"; then
	CC=`${WHICH} cc 2> /dev/null`
	if whichFailed "${CC}"; then
		CC="/usr/bin/cc"
		if test ! -x ${CC}; then
			echo "Warning: cc not found, using gcc instead."
			CC="/usr/bin/gcc"
		fi	
	fi
	assertValidExecutable "${CC}" cc
	export CC
else
	assertValidExecutableInput "${CC}" cc
fi

DoCritialConftest ./VMake/SystemTests/C-CompilerType CC_TYPE
if test "${CC_TYPE}x" = "x"; then
	echo "Warning: Unknown C compiler type \"${CC_TYPE}\"."
	unset CC_TYPE
else
	export CC_TYPE
fi

if test "${CC_PIPE}x" = "x"; then
	case ${CC_TYPE} in
		gnu)
			CC_PIPE="-pipe";;
		osf)
			CC_PIPE="";;
		intel)
			CC_PIPE="";;
		sparc)
			CC_PIPE="";;
		ibmxl)
			CC_PIPE="";;
		*)
			echo "Warning: CC_PIPE for C compiler \"${CC_TYPE}\" unknown. Please set.";;  
	esac
	export CC_PIPE
fi

# The additional flags (if any) for 64 bit compilation
if test "${CC_64}x" = "x"; then
	case ${CC_TYPE} in
		gnu)
			CC_64="-m64";;
		osf)
			CC_64="";; 
		intel)
			CC_64="";;
		sparc)
			CC_64="";;
		ibmxl)
			CC_64="-q64";;
		*)
			echo "Warning: CC_64 for C compiler \"${CC_TYPE}\" unknown. Please set if needed.";;  
	esac
	export CC_64
fi

DoCritialConftest ./VMake/SystemTests/LongSize SYSTEM_SIZEOF_LONG
if test "${SYSTEM_SIZEOF_LONG}" = "8"; then
	SYSTEM_64=1
else
	SYSTEM_64=0
fi
export SYSTEM_64

if ! DoConftest ./VMake/SystemTests/Endian SYSTEM_BIG_ENDIAN; then
	# Assume little (x86)
	SYSTEM_BIG_ENDIAN=0
fi
export SYSTEM_BIG_ENDIAN

if test "${OBJCC}x" = "x"; then
	case ${CC_TYPE} in
		gnu)
			OBJCC="${CC} -x objective-c";;
		*)
			echo "Warning: OBJCC for C compiler \"${CC_TYPE}\" unknown. Please set if needed.";;  
	esac
	export OBJCC
fi

if test "${OBJECTIVEC_LIBS}x" = "x"; then
	case ${CC_TYPE} in
		gnu)
			OBJECTIVEC_LIBS="-lobjc";;
		*)
			echo "Warning: OBJECTIVEC_LIBS for C compiler \"${CC_TYPE}\" unknown. Please set if needed.";;  
	esac
	export OBJCECTIVEC_LIBS
fi

	

if test "${CXX_EXT}x" = "x"; then
	CXX_EXT="cxx"
	export CXX_EXT
fi

if test "${CXX}x" = "x"; then
	CXX=`${WHICH} c++ 2> /dev/null`
	if whichFailed "${CXX}"; then
		case ${SYSTEM} in
			Linux|Darwin|CYGWIN|SunOS)
				CXX=`${WHICH} g++ 2> /dev/null`
				if whichFailed "${CXX}"; then
					CXX="/usr/bin/g++";
				fi;;
			OSF1)
				CXX="/usr/bin/cxx";;
			*)
				CXX="";;
		esac
	fi	
	export CXX
	CXX_TEST=`warnValidExecutable ${CXX} cxx`
else
	CXX_TEST=`warnValidExecutableInput ${CXX} cxx`
fi

if test "${CXX_TEST}x" != "x"; then
	echo ${CXX_TEST}
else
	DoCritialConftest ./VMake/SystemTests/CXX-CompilerType CXX_TYPE
	if test "${CXX_TYPE}x" = "x"; then
		echo "Warning: Unknown C compiler type \"${CXX_TYPE}\"."
		unset CXX_TYPE
	else
		export CXX_TYPE
	fi
	
	case $CC_TYPE in
		gnu)
			case $CXX_TYPE in
				gnu)
					CC_CXX_LFLAGS="-lstdc++";;
				*)
					echo "Warning: CC_CXX_LFLAGS for C compiler \"${CC_TYPE}\" unknown for C++ compiler \"${CXX_TYPE}\".";;
			esac ;;
		osf)
			case $CXX_TYPE in
				osf)
					CC_CXX_LFLAGS="-lcxx";;
				*)
					echo "Warning: CC_CXX_LFLAGS for C compiler \"${CC_TYPE}\" unknown for C++ compiler \"${CXX_TYPE}\"";;
			esac ;;
		intel)
			case $CXX_TYPE in
				intel)
					CC_CXX_LFLAGS="";;
				*)
					echo "Warning: CC_CXX_LFLAGS for C compiler \"${CC_TYPE}\" unknown for C++ compiler \"${CXX_TYPE}\".";;
			esac ;;
			
		sparc)
			case $CXX_TYPE in
				gnu)
					CC_CXX_LFLAGS="";;
				sparc)
					CC_CXX_LFLAGS="";;
				*)
					echo "Warning: CC_CXX_LFLAGS for C compiler \"${CC_TYPE}\" unknown for C++ compiler \"${CXX_TYPE}\".";;
			esac ;;
		ibmxl)
			CC_CXX_LFLAGS="-L /usr/local/IBM_compilers/vacpp/7.0/lib64 -lxlopt -lxl -libmc++ -L /usr/lib64 -lstdc++ -R /usr/local/IBM_compilers/vacpp/7.0/lib64";;
		*)
			echo "Error: CC_CXX_LFLAGS for C compiler \"${CC_TYPE}\" unknown";
			exit ;;
	esac
	export CC_CXX_LFLAGS
fi	

if test "${CXX_PIPE}x" = "x"; then
	case ${CXX_TYPE} in
		gnu)
			CXX_PIPE="-pipe";;
		osf)
			CXX_PIPE="";;
		intel)
			CXX_PIPE="";;
		sparc)
			CXX_PIPE="";;
		ibmxl)
			CXX_PIPE="";;
		*)
			echo "Warning: CXX_PIPE for C compiler \"${CXX_TYPE}\" unknown. Please set.";;  
	esac
	export CXX_PIPE
fi

if test "${F77}x" = "x"; then
	F77=`${WHICH} f77 2> /dev/null`
	if whichFailed "${F77}"; then
		F77="/usr/bin/f77"
		if test ! -x ${F77}; then
			echo "Warning: f77 not found, using g77 instead."
			F77=`${WHICH} g77 2> /dev/null`
			if whichFailed "${F77}"; then
				if test "${SYSTEM}" = "Darwin"; then
					if test -x "/usr/bin/g77"; then
						F77="/usr/bin/g77"
					else
						F77="/sw/bin/g77"
					fi
				else
					F77="/usr/bin/g77"
				fi
			fi	
		fi
	fi
	F77_TEST=`warnValidExecutable ${F77} f77`
	export F77
else
	F77_TEST=`warnValidExecutableInput ${F77} f77`
fi

if test "${F77_TEST}x" != "x"; then
	echo ${F77_TEST}
else
	DoCritialConftest ./VMake/SystemTests/Fortran-CompilerType F77_TYPE
	if test "${F77_TYPE}x" = "x"; then
		echo "Warning: Unknown F77 compiler type \"${F77_TYPE}\"."
		unset F77_TYPE
	else
		export F77_TYPE
	fi

	if test "${FORTRAN_COMPILER_LIBDIR}x" = "x"; then
		case $F77_TYPE in
			gnu)
				case ${SYSTEM} in
					Darwin)
						FORTRAN_COMPILER_LIBDIR="/sw/lib";;
					*)
						#gcc can usually find it for systems...
						FORTRAN_COMPILER_LIBDIR="/usr/lib";;
				esac;;		
			osf)
				case ${SYSTEM} in
					OSF1)
						FORTRAN_COMPILER_LIBDIR="/usr/lib";;
					*)
						echo "Error: FORTRAN_COMPILER_LIBDIR for F77 compiler \"${F77_TYPE}\" unknown for system \"${SYSTEM}\"";
						exit ;;
				esac;;
			intel)
				IFORT_EXE=`which ifort`;
				FORTRAN_COMPILER_LIBDIR=`${GREP} 'LD_LIBRARY_PATH=.*;' ${IFORT_EXE} | ${GREP} -v '\$LD_LIBRARY_PATH' | cut -f 2 -d = | sed 's/;//g'`
				;;
			sparc)
				echo "Warning: FORTRAN_COMPILER_LIBDIR for F77 compiler \"${F77_TYPE}\" not able be determined automatically, please set as an environment variable.";;
			esac	
		export FORTRAN_COMPILER_LIBDIR
	fi

	if test "${EXTRA_FORTRAN_LIBS}x" = "x"; then
		case $CC_TYPE in
			gnu)
				case $F77_TYPE in
					gnu)
						EXTRA_FORTRAN_LIBS="-lg2c";;
					*)
						echo "Warning: EXTRA_FORTRAN_LIBS for C compiler \"${CC_TYPE}\" unknown for F77 compiler \"${F77_TYPE}\".";
				esac ;;
			osf)
				case $F77_TYPE in
					osf)
						EXTRA_FORTRAN_LIBS="-lfor";;
					*)
						echo "Error: EXTRA_FORTRAN_LIBS for C compiler \"${CC_TYPE}\" unknown for F77 compiler \"${F77_TYPE}\"";
						exit ;;
				esac ;;
			intel)
				case $F77_TYPE in
					intel)
						EXTRA_FORTRAN_LIBS="-lifport -lifcore";;
					*)
						echo "Warning: EXTRA_FORTRAN_LIBS for C compiler \"${CC_TYPE}\" unknown for F77 compiler \"${F77_TYPE}\".";
				esac ;;
			sparc)
				case $F77_TYPE in
					sparc)
						EXTRA_FORTRAN_LIBS="";;
					*)
						echo "Warning: EXTRA_FORTRAN_LIBS for C compiler \"${CC_TYPE}\" unknown for F77 compiler \"${F77_TYPE}\".";
				esac ;;
			ibmxl)
				EXTRA_FORTRAN_LIBS="";;	
			*)
				echo "Error: EXTRA_FORTRAN_LIBS for C compiler \"${CC_TYPE}\" unknown";
				exit ;;
		esac
		export EXTRA_FORTRAN_LIBS
	fi	

	if test "${CC_FORTRAN_LFLAGS}x" = "x"; then
		if test "${FORTRAN_COMPILER_LIBDIR}x" != "x"; then
			CC_FORTRAN_LFLAGS='-L${FORTRAN_COMPILER_LIBDIR} ${EXTRA_FORTRAN_LIBS}'
		else
			CC_FORTRAN_LFLAGS='${EXTRA_FORTRAN_LIBS}'
		fi
		export CC_FORTRAN_LFLAGS
	fi	

	DoCritialConftest ./VMake/SystemTests/Fortran-Symbol F77_SYMBOL_TRANSLATION
	export F77_SYMBOL_TRANSLATION
fi	

	
if test "${LINKER}x" = "x"; then
	LINKER=${CC}
	export LINKER;
fi

# Note: pic capitalised to PIC below 17 August 2004 to work on Sun Sparc machines
if test "${SO_CFLAGS}x" = "x"; then
	case $CC_TYPE in
		gnu)
			case $SYSTEM in
				Linux)
					SO_CFLAGS="-fPIC";;
				CYGWIN|Darwin|OSF1)
					SO_CFLAGS="";;
				SunOS)
					SO_CFLAGS="-fPIC";;
				*)
					echo "SO_CFLAGS for C compiler \"${CC_TYPE}\" unknown for system \"${SYSTEM}\"";
					exit ;;
			esac ;;
		osf)
			SO_CFLAGS="-shared";;
		intel)
			SO_CFLAGS="-fPIC";;
		sparc)
			SO_CFLAGS="-shared";;
		ibmxl)
			SO_CFLAGS="-qpic";;
		pgi)
			SO_CFLAGS="-fPIC";;
		*)
			echo "SO_CFLAGS for C compiler \"${CC_TYPE}\" unknown";
			exit ;;
	esac
	export SO_CFLAGS
fi

if test "${SO_EXT}x" = "x"; then
	# SO_EXT... extension of a dynamically linked library
	# MODULE_EXT... extension a dynamucally loadable module (e.g. plugin). Necessary because this is different on 
	#	Darwin to a dll.
	case $CC_TYPE in
		gnu)
			case $SYSTEM in
				Linux|OSF1|SunOS)
					SO_EXT="so";
					MODULE_EXT='${SO_EXT}';;
				Darwin)
					SO_EXT="dylib";
					MODULE_EXT="so";;
				CYGWIN)
					SO_EXT="dll";
					MODULE_EXT='${SO_EXT}';;
				*)
					echo "Error: SO_EXT/MODULE_EXT for C compiler \"${CC_TYPE}\" unknown for system \"${SYSTEM}\"";
					exit ;;
			esac ;;
		osf)
			SO_EXT="so";
			MODULE_EXT='${SO_EXT}';;
		intel)
			SO_EXT="so";
			MODULE_EXT='${SO_EXT}';;
		sparc)
			SO_EXT="so";
			MODULE_EXT='${SO_EXT}';;
		ibmxl)
			SO_EXT="so";
			MODULE_EXT='${SO_EXT}';;
		pgi)
			SO_EXT="so";
			MODULE_EXT='${SO_EXT}';;
		*)
			echo "Error: SO_EXT/MODULE_EXT for C compiler \"${CC_TYPE}\" unknown";
			exit ;;
	esac
	export SO_EXT
	export MODULE_EXT
fi

if test "${SO_LFLAGS}x" = "x"; then
	# SO_LFLAGS... what to pass to the compiler to tell it to create a dynamically linked library
	case $CC_TYPE in
		gnu)
			case $SYSTEM in
				Linux|CYGWIN|SunOS)
					SO_LFLAGS="-shared";;
				Darwin)
					SO_LFLAGS="-dynamiclib -flat_namespace -undefined suppress -single_module";;
				OSF1)
					SO_LFLAGS="-Wl,-shared";;
				*)
					echo "Error: SO_LFLAGS for C compiler \"${CC_TYPE}\" unknown for system \"${SYSTEM}\"";
					exit ;;
			esac ;;
		osf)
			SO_LFLAGS='-shared -check_registry ${LIB_DIR}/so_locations -update_registry ${LIB_DIR}/so_locations';;
		intel)
			SO_LFLAGS="-shared";;
		sparc)
			SO_LFLAGS="-G";;
		ibmxl)
			SO_LFLAGS="-qmkshrobj";;
		pgi)
			SO_LFLAGS="-shared";;
		*)
			echo "Error: SO_LFLAGS for C compiler \"${CC_TYPE}\" unknown";
			exit ;;
	esac
	export SO_LFLAGS
fi

if test "${MODULE_LFLAGS}x" = "x"; then
	# MODULE_LFLAGS... what to pass to the compiler to tell is to create a dynamucally loadable module (e.g. plugin). Necessary
	#	because this is built differently on Darwin to a dll.
	case $CC_TYPE in
		gnu)
			case $SYSTEM in
				Linux|CYGWIN|SunOS)
					MODULE_LFLAGS='${SO_LFLAGS}';;
				OSF1)
					MODULE_LFLAGS="";;
				Darwin)
					MODULE_LFLAGS="-bundle -flat_namespace -undefined suppress";;
				*)
					echo "Error: MODULE_LFLAGS for C compiler \"${CC_TYPE}\" unknown for system \"${SYSTEM}\"";
					exit ;;
			esac ;;
		osf)
			MODULE_LFLAGS='${SO_LFLAGS}';;
		intel)
			MODULE_LFLAGS='${SO_LFLAGS}';;
		sparc)
			MODULE_LFLAGS='${SO_LFLAGS}';;
		ibmxl)
			MODULE_LFLAGS='${SO_LFLAGS}';;
		pgi)
			MODULE_LFLAGS='${SO_LFLAGS}';;
		*)
			echo "Error: MODULE_LFLAGS for C compiler \"${CC_TYPE}\" unknown";
			exit ;;
	esac
	export MODULE_LFLAGS
fi
export MODULE_LFLAGS

if test "${RPATH_FLAG}x" = "x"; then
	case $CC_TYPE in
		gnu)
			case $SYSTEM in
				Linux|CYGWIN)
					RPATH_FLAG='-Xlinker -rpath -Xlinker '
					;;
				Darwin)
					RPATH_FLAG="";;
				SunOS)
					RPATH_FLAG="";; # don't do it, because Alan doesn't thinks it works on rice anyway
				OSF1)
					RPATH_FLAG="-Wl,-rpath,";;
				*)
					echo "Error: RPATH_FLAG for C compiler \"${CC_TYPE}\" unknown for system \"${SYSTEM}\"";
					exit ;;
			esac ;;
		osf)
			RPATH_FLAG='-rpath '
			;;
		intel)
			RPATH_FLAG='-Xlinker -rpath -Xlinker '
			;;
		sparc)
			RPATH_FLAG='-R'
			;;
		ibmxl)
			RPATH_FLAG='-R '
			;;
		pgi)
			RPATH_FLAG="-Wl,-rpath,";;
		*)
			echo "Error: RPATH_FLAG for C compiler \"${CC_TYPE}\" unknown";
			exit ;;
	esac
fi

if test "${RPATH_FLAG}x" != "x"; then
	if test "${RPATH_LFLAGS}x" = "x"; then
		RPATH_LFLAGS='${RPATH_FLAG}${LIB_DIR}'
	else
		case $CC_TYPE in
			osf|sparc)
				RPATH_FLAGS='${RPATH_FLAG}${LIB_DIR}:'"`echo ${RPATH} | ${SED} 's/-rpath //'`"
				;;
			*)
				RPATH_LFLAGS='${RPATH_FLAG}${LIB_DIR} ${RPATH_LFLAGS}'
				;;
		esac
	fi
fi
export RPATH_FLAG
export RPATH_LFLAGS

if test "${EXPORT_DYNAMIC_LFLAGS}x" = "x"; then
	case $CC_TYPE in
		gnu)
			case $SYSTEM in
				Linux|CYGWIN)
					EXPORT_DYNAMIC_LFLAGS="-Xlinker -export-dynamic";;
				Darwin)
					EXPORT_DYNAMIC_LFLAGS="-framework System -framework CoreServices -framework Foundation";;
				SunOS)
					EXPORT_DYNAMIC_LFLAGS="-B dynamic";;
				OSF1)
					EXPORT_DYNAMIC_LFLAGS="";;
				*)
					echo "Error: EXPORT_DYNAMIC_LFLAGS for C compiler \"${CC_TYPE}\" unknown for system \"${SYSTEM}\"";
					exit ;;
			esac ;;
		osf)
			EXPORT_DYNAMIC_LFLAGS="";;
		intel)
			EXPORT_DYNAMIC_LFLAGS="";;
		sparc)
			EXPORT_DYNAMIC_LFLAGS="-B dynamic";;
		ibmxl)
			EXPORT_DYNAMIC_LFLAGS="-Wl,--export-dynamic";;
		pgi)
			EXPORT_DYNAMIC_LFLAGS="-Bdynamic";;
		*)
			echo "Error: EXPORT_DYNAMIC_LFLAGS for C compiler \"${CC_TYPE}\" unknown";
			exit ;;
	esac
	export EXPORT_DYNAMIC_LFLAGS
fi


# Set warning level
if test "${CC_WARNINGLEVEL}x" = "x"; then
	case ${CC_TYPE} in
		gnu)
			CC_WARNINGLEVEL="-Wall";;
		osf)
			CC_WARNINGLEVEL="-msg_enable level3 -msg_disable unstructmem -msg_disable undefenum -msg_disable nonstandcast -msg_disable mixfuncvoid";;
		intel)
			CC_WARNINGLEVEL="-w1";;
		sparc)
			CC_WARNINGLEVEL="";;
		ibmxl)
			# theres no good equivalient to -Wall, turn on a few warnings at least
			CC_WARNINGLEVEL="-qformat=all -qwarn64";; 
		pgi)
			CC_WARNINGLEVEL="-Minform=inform";;
		*)
			echo "Warning: CC_WARNINGLEVEL for C compiler \"${CC_TYPE}\" unknown. Please set.";;  
	esac
	export CC_WARNINGLEVEL
fi


# Set symbol level
if test "${CC_SYMBOLLEVEL}x" = "x"; then
	if test "${USE_OPTIMISED}x" = "x"; then
		case ${CC_TYPE} in
			gnu)
				CC_SYMBOLLEVEL="-g";;
			osf)
				CC_SYMBOLLEVEL="-g3";;
			intel)
				CC_SYMBOLLEVEL="-g";;
			sparc)
				CC_SYMBOLLEVEL="-g";;
			ibmxl)
				CC_SYMBOLLEVEL="-g";;
			pgi)
				CC_SYMBOLLEVEL="-g";;
			*)
				echo "Warning: CC_SYMBOLLEVEL for C compiler \"${CC_TYPE}\" unknown. Please set.";;  
		esac
	else
		CC_SYMBOLLEVEL=" "
	fi
	export CC_SYMBOLLEVEL
fi

# Set optimisation level
if test "${CC_OPTIMISATIONLEVEL}x" = "x"; then
	if test "${USE_OPTIMISED}x" = "x"; then
		CC_OPTIMISATIONLEVEL=""
	else
		case ${MACHINE} in
			i686)
				CC_OPTIMISATIONLEVEL="-O3 -march=i686 -fomit-frame-pointer -funroll-loops";; #-fmove-all-movables";;
			x86_64)
				if test "${CC_TYPE}x" = "pgix"; then
					CC_OPTIMISATIONLEVEL="-O3 -tp amd64e" #-fomit-frame-pointer -funroll-loops";;
				else
					CC_OPTIMISATIONLEVEL="-O3 -march=athlon64" #-fomit-frame-pointer -funroll-loops";;
				fi;;
			ia64)
				CC_OPTIMISATIONLEVEL=" ";;
			alpha)
				CC_OPTIMISATIONLEVEL=" ";;
			power)
				CC_OPTIMISATIONLEVEL="-O2 -faltivec";;
			ppc64)
				if test "${CC_TYPE}" = "ibmxl"; then
					CC_OPTIMISATIONLEVEL="-O5 -qunroll"
				else
					CC_OPTIMISATIONLEVEL=" "
				fi	;;
			*)
				echo "Warning: CC_OPTIMISATIONLEVEL for C compiler \"${CC_TYPE}\" unknown with USE_OPTIMISED enabled. Please set.";;  
		esac
		CC_OPTIMISATIONLEVEL="${CC_OPTIMISATIONLEVEL} -DNDEBUG"
	fi
	export CC_OPTIMISATIONLEVEL
fi



# Obtain dl information
if test "${DL_DIR}x" = "x"; then
	case ${SYSTEM} in
		Linux|CYGWIN|OSF1|SunOS|ranger)
			DL_DIR="/usr";;
		Darwin)
			DL_DIR="/sw";;
		*)
			echo "Warning: DL_DIR for system \"${SYSTEM}\" unknown. Set the environment variable.";;
	esac
	export DL_DIR
fi
if test "${DL_LIBDIR}x" = "x"; then
	DL_LIBDIR=${DL_DIR}/lib
	export DL_LIBDIR
fi
if test "${DL_LIBFILES}x" = "x"; then
	case $SYSTEM in
		Linux|Darwin|SunOS|ranger)
			DL_LIBFILES="-ldl";;
			# dlcompat package is required for darwin
		CYGWIN|OSF1)
			DL_LIBFILES="";;
		*)
			echo "Warning: DL_LIBFILES for system \"${SYSTEM}\" unknown. Set the DL_LIBFILES environment variable.";;
	esac
	export DL_LIBFILES
fi
if test "${DL_LIBS}x" = "x"; then
	DL_LIBS='-L${DL_LIBDIR} ${DL_LIBFILES}'
	export DL_LIBS
fi
case ${SYSTEM} in
	CYGWIN|OSF1)
		;;
	*)
		if test -r ${DL_LIBDIR}/libdl.a; then
			:
		elif test -r ${DL_LIBDIR}/libdl.so; then
			:
		elif test -r ${DL_LIBDIR}/libdl.dylib; then
		        :
		else
			echo "Warning: ${DL_LIBDIR}/libdl.[a|so]  not found. Set the DL_DIR environment variables. Darwin requires the dlcompat package installed."
		fi;;
esac
if test "${DL_INCDIR}x" = "x"; then
	DL_INCDIR=${DL_DIR}/include
	export DL_INCDIR
fi
if test -r ${DL_INCDIR}/dlfcn.h; then
	:
else
	echo "Warning: ${DL_INCDIR}/dlfcn.h not found. Set the environment variables."
fi
if test "${DL_INCLUDES}x" = "x"; then
	DL_INCLUDES="-I${DL_INCDIR}"
	export DL_INCLUDES
fi

if test "${DOXYGEN}x" = "x"; then
	DOXYGEN=`${WHICH} doxygen 2> /dev/null`
	if whichFailed "${DOXYGEN}"; then
		DOXYGEN="/usr/bin/doxygen"
	fi
	warnValidExecutable "${DOXYGEN}" doxygen
	export DOXYGEN
else
	warnValidExecutableInput "${DOXYGEN}" doxygen
fi


if test "${SVN}x" = "x"; then
	SVN=`${WHICH} svn 2> /dev/null`
	if whichFailed "${SVN}"; then
		SVN="/usr/bin/svn"
	fi

	warnValidExecutable "${SVN}" svn
	resolveVariable SVN_EXEC ${SVN}
	if test -x "${SVN_EXEC}"; then
		HAVE_SVN=1
	fi
else
	warnValidExecutableInput "${SVN}" svn
	if test -x "${SVN}"; then
		HAVE_SVN=1
	fi
fi

if test "${CSOAP_DIR}x" = "x"; then
	CSOAP_DIR="/usr/lib/libcsoap"
	export CSOAP_DIR
fi
if test "${CSOAP_INCDIR}x" = "x"; then
	CSOAP_INCDIR='${CSOAP_DIR}/include/libcsoap-1.0'
	export CSOAP_INCDIR
fi
if test "${CSOAP_INCLUDES}x" = "x"; then
	CSOAP_INCLUDES='-I${CSOAP_INCDIR}'
	export CSOAP_INCLUDES
fi
if test "${CSOAP_LIBDIR}x" = "x"; then
	CSOAP_LIBDIR='${CSOAP_DIR}/lib'
	export CSOAP_LIBDIR
fi
if test "${CSOAP_LIBFILES}x" = "x"; then
	CSOAP_LIBFILES="-lcsoap-1.0"
	export CSOAP_LIBFILES
fi
if test "${CSOAP_LIBS}x" = "x"; then
	CSOAP_LIBS='-L${CSOAP_LIBDIR} ${CSOAP_LIBFILES}'
	export CSOAP_LIBS
fi

if test "${NANOHTTP_DIR}x" = "x"; then
	NANOHTTP_DIR='${CSOAP_DIR}'		# Use the same nanohttp bundled with csoap
	export NANOHTTP_DIR
fi
if test "${NANOHTTP_INCDIR}x" = "x"; then
	NANOHTTP_INCDIR='${NANOHTTP_DIR}/include/nanohttp-1.0'
	export NANOHTTP_INCDIR
fi
if test "${NANOHTTP_INCLUDES}x" = "x"; then
	NANOHTTP_INCLUDES='-I${NANOHTTP_INCDIR}'
	export NANOHTTP_INCLUDES
fi
if test "${NANOHTTP_LIBDIR}x" = "x"; then
	NANOHTTP_LIBDIR='${NANOHTTP_DIR}/lib'
	export NANOHTTP_LIBDIR
fi
if test "${NANOHTTP_LIBFILES}x" = "x"; then
	NANOHTTP_LIBFILES="-lnanohttp-1.0"
	export NANOHTTP_LIBFILES
fi
if test "${NANOHTTP_LIBS}x" = "x"; then
	NANOHTTP_LIBS='-L${NANOHTTP_LIBDIR} ${NANOHTTP_LIBFILES}'
	export NANOHTTP_LIBS
fi

if test -r `eval echo "${CSOAP_INCDIR}/libcsoap/soap-client.h"`; then
	HAVE_SOAP=1;
	export HAVE_SOAP
fi

if test "${REGRESSTOR_URL}x" = "x"; then
	REGRESSTOR_URL="http://localhost/~alan/cgi-bin/RegresstorServices/Regresstor.cgi";
	export REGRESSTOR_URL
fi

# Write out to Makefile.system
cat << EOF > Makefile.system
ifndef WHICH
	WHICH=${WHICH}
endif
ifndef UNAME
	UNAME=${UNAME}
endif
ifndef UNAME_M
	UNAME_M=${UNAME_M}
endif
ifndef UNAME_R
	UNAME_R=${UNAME_R}
endif
ifndef MACHINE
	MACHINE=${MACHINE}
endif
ifndef KERNEL_RELEASE
	KERNEL_RELEASE=${KERNEL_RELEASE}
endif
ifndef UNAME_S
	UNAME_S=${UNAME_S}
endif
ifndef SYSTEM
	SYSTEM=${SYSTEM}
endif
ifndef MAKE
	MAKE=${MAKE}
endif
ifndef SH
	SH=${SH}
endif
ifeq (default,\$(origin RM))
	RM=${RM}
endif
ifndef RM
	RM=${RM}
endif
ifndef RM_F
	RM_F=${RM_F}
endif
ifndef CD
	CD=${CD}
endif
ifndef CP
	CP=${CP}
endif
ifndef CP_F
	CP_F=${CP_F}
endif
ifndef MV
	MV=${MV}
endif
ifndef MV_F
	MV_F=${MV_F}
endif
ifndef MKDIR
	MKDIR=${MKDIR}
endif
ifndef SED
	SED=${SED}
endif
ifndef FIND
	FIND=${FIND}
endif
ifndef LN
	LN=${LN}
endif
ifndef LN_S
	LN_S=${LN_S}
endif
ifndef TEE
	TEE=${TEE}
endif
ifndef NM
	NM=${NM}
endif
ifndef LS
	LS=${LS}
endif
ifndef GREP
	GREP=${GREP}
endif
ifndef AR
	AR=${AR}
endif
ifndef AR_C
	AR_C=${AR_C}
endif
ifndef RANLIB
	RANLIB=${RANLIB}
endif
ifeq (default,\$(origin CC))
	CC=${CC}
endif
ifndef CC
	CC=${CC}
endif
ifndef CC_TYPE
	CC_TYPE=${CC_TYPE}
endif
ifeq (default,\$(origin CXX))
	CXX=${CXX}
endif
ifndef CC_PIPE
	CC_PIPE=${CC_PIPE}
endif
ifndef CC_64
	CC_64=${CC_64}
endif
ifndef SYSTEM_SIZEOF_LONG
	SYSTEM_SIZEOF_LONG=${SYSTEM_SIZEOF_LONG}
endif
ifndef SYSTEM_64
	SYSTEM_64=${SYSTEM_64}
endif
ifndef SYSTEM_BIG_ENDIAN
	SYSTEM_BIG_ENDIAN=${SYSTEM_BIG_ENDIAN}
endif
ifndef OBJCC
	OBJCC=${OBJCC}
endif
ifndef OBJECTIVEC_LIBS
	OBJECTIVEC_LIBS=${OBJECTIVEC_LIBS}
endif
ifndef CXX_EXT
	CXX_EXT=${CXX_EXT}
endif
ifndef CXX
	CXX=${CXX}
endif
ifndef CXX_TYPE
	CXX_TYPE=${CXX_TYPE}
endif
ifndef CXX_PIPE
	CXX_PIPE=${CXX_PIPE}
endif
ifeq (default,\$(origin F77))
	F77=${F77}
endif
ifndef F77
	F77=${F77}
endif
ifndef F77_TYPE
	F77_TYPE=${F77_TYPE}
endif
ifndef LINKER
	LINKER=${LINKER}
endif
ifndef SO_EXT
	SO_EXT=${SO_EXT}
endif
ifndef MODULE_EXT
	MODULE_EXT=${MODULE_EXT}
endif
ifndef SO_CFLAGS
	SO_CFLAGS=${SO_CFLAGS}
endif
ifndef SO_LFLAGS
	SO_LFLAGS=${SO_LFLAGS}
endif
ifndef MODULE_LFLAGS
	MODULE_LFLAGS=${MODULE_LFLAGS}
endif
ifndef RPATH_FLAG
	RPATH_FLAG=${RPATH_FLAG}
endif
ifndef RPATH
	RPATH=${RPATH}
endif
ifndef RPATH_LFLAGS
	RPATH_LFLAGS=${RPATH_LFLAGS}
endif
ifndef EXPORT_DYNAMIC_LFLAGS
	EXPORT_DYNAMIC_LFLAGS=${EXPORT_DYNAMIC_LFLAGS}
endif
ifndef CC_CXX_LFLAGS
	CC_CXX_LFLAGS=${CC_CXX_LFLAGS}
endif
ifndef FORTRAN_COMPILER_LIBDIR
	FORTRAN_COMPILER_LIBDIR=${FORTRAN_COMPILER_LIBDIR}
endif
ifndef EXTRA_FORTRAN_LIBS
	EXTRA_FORTRAN_LIBS=${EXTRA_FORTRAN_LIBS}
endif
ifndef CC_FORTRAN_LFLAGS
	CC_FORTRAN_LFLAGS=${CC_FORTRAN_LFLAGS}
endif
ifndef F77_LIBS
	F77_LIBS=${F77_LIBS}
endif
ifndef F77_SYMBOL_TRANSLATION
	F77_SYMBOL_TRANSLATION=${F77_SYMBOL_TRANSLATION}
endif
ifndef CC_WARNINGLEVEL
	CC_WARNINGLEVEL=${CC_WARNINGLEVEL}
endif
ifndef CC_SYMBOLLEVEL
	CC_SYMBOLLEVEL=${CC_SYMBOLLEVEL}
endif
ifndef CC_OPTIMISATIONLEVEL
	CC_OPTIMISATIONLEVEL=${CC_OPTIMISATIONLEVEL}
endif
ifndef NOSHARED
	NOSHARED=${NOSHARED}
endif
ifndef DL_DIR
	DL_DIR=${DL_DIR}
endif
ifndef DL_LIBDIR
	DL_LIBDIR=${DL_LIBDIR}
endif
ifndef DL_LIBFILES
	DL_LIBFILES=${DL_LIBFILES}
endif
ifndef DL_LIBS
	DL_LIBS=${DL_LIBS}
endif
ifndef DL_INCDIR
	DL_INCDIR=${DL_INCDIR}
endif
ifndef DL_INCLUDES
	DL_INCLUDES=${DL_INCLUDES}
endif
ifndef NOPYTHON
	NOPYTHON=${NOPYTHON}
endif
ifndef DOXYGEN
	DOXYGEN=${DOXYGEN}
endif
ifndef SVN
	SVN=${SVN}
endif
ifndef HAVE_SVN
	HAVE_SVN=${HAVE_SVN}
endif
ifndef CSOAP_DIR
	CSOAP_DIR=${CSOAP_DIR}
endif
ifndef CSOAP_INCDIR
	CSOAP_INCDIR=${CSOAP_INCDIR}
endif
ifndef CSOAP_INCLUDES
	CSOAP_INCLUDES=${CSOAP_INCLUDES}
endif
ifndef CSOAP_LIBDIR
	CSOAP_LIBDIR=${CSOAP_LIBDIR}
endif
ifndef CSOAP_LIBFILES
	CSOAP_LIBFILES=${CSOAP_LIBFILES}
endif
ifndef CSOAP_LIBS
	CSOAP_LIBS=${CSOAP_LIBS}
endif
ifndef NANOHTTP_DIR
	NANOHTTP_DIR=${NANOHTTP_DIR}
endif
ifndef NANOHTTP_INCDIR
	NANOHTTP_INCDIR=${NANOHTTP_INCDIR}
endif
ifndef NANOHTTP_INCLUDES
	NANOHTTP_INCLUDES=${NANOHTTP_INCLUDES}
endif
ifndef NANOHTTP_LIBDIR
	NANOHTTP_LIBDIR=${NANOHTTP_LIBDIR}
endif
ifndef NANOHTTP_LIBFILES
	NANOHTTP_LIBFILES=${NANOHTTP_LIBFILES}
endif
ifndef NANOHTTP_LIBS
	NANOHTTP_LIBS=${NANOHTTP_LIBS}
endif
ifndef HAVE_SOAP
	HAVE_SOAP=${HAVE_SOAP}
endif
ifndef REGRESSTOR_URL
	REGRESSTOR_URL=${REGRESSTOR_URL}
endif
EOF


if test ! "${USE_OPTIMISED}x" = "x"; then
cat << EOF >> Makefile.system
USE_OPTIMISED=${USE_OPTIMISED}
EOF
fi

if test ! "${USE_DEBUG}x" = "x"; then
cat << EOF >> Makefile.system
USE_DEBUG=${USE_DEBUG}
EOF
fi

if test ! "${USE_PROFILE}x" = "x"; then
cat << EOF >> Makefile.system
USE_PROFILE=${USE_PROFILE}
CFLAGS += -DUSE_PROFILE=1
EOF
fi

if test ! "${USE_TAU}x" = "x"; then
cat << EOF >> Makefile.system
USE_TAU=${USE_TAU}
EOF
fi


if test ! "${USE_PDT}x" = "x"; then
cat << EOF >> Makefile.system
USE_PDT=${USE_PDT}
EOF
fi

if test ! "${USE_MACRO_AS_FUNC}x" = "x"; then
cat << EOF >> Makefile.system
USE_MACRO_AS_FUNC=${USE_MACRO_AS_FUNC}
EOF
fi

if test ! "${USE_CAUTIOUS}x" = "x"; then
cat << EOF >> Makefile.system
USE_CAUTIOUS=${USE_CAUTIOUS}
EOF
fi

if test ! "${USE_MEMORY_STATS}x" = "x"; then
cat << EOF >> Makefile.system
USE_MEMORY_STATS=${USE_MEMORY_STATS}
EOF
fi

if test ! "${USE_NOLOG}x" = "x"; then
cat << EOF >> Makefile.system
USE_NOLOG=${USE_NOLOG}
EOF
fi

if test ! "${USE_64BITS}x" = "x"; then
cat << EOF >> Makefile.system
USE_64BITS=${USE_64BITS}
EOF
fi

if test ! "${USE_MPIRECORD}x" = "x"; then
cat << EOF >> Makefile.system
USE_MPIRECORD=${USE_MPIRECORD}
EOF
fi

# PDT
if test ! "${USE_PDT}x" = "x"; then
	. ./VMake/Config/pdt-config.sh
fi

# TAU
if test ! "${USE_TAU}x" = "x"; then
	. ./VMake/Config/tau-config.sh
fi

addLocationInfoTo() {
cat << EOF >> $1
ifndef OPTIONS
	OPTIONS=${OPTIONS}
endif
ifndef USE_RUN_LONG_TESTS
	USE_RUN_LONG_TESTS=${USE_RUN_LONG_TESTS}
endif
ifndef PROJ_ROOT
	PROJ_ROOT=${PROJ_ROOT}/${dir}
endif
ifndef BLD_DIR
	BLD_DIR=${BLD_DIR}
endif
ifndef EXPORT_DIR
	EXPORT_DIR=${EXPORT_DIR}
endif
ifndef TMP_DIR
	TMP_DIR=${TMP_DIR}
endif
ifndef BIN_DIR
	BIN_DIR=${BIN_DIR}
endif
ifndef LIB_DIR
	LIB_DIR=${LIB_DIR}
endif
ifndef INC_DIR
	INC_DIR=${INC_DIR}
endif
ifndef TST_DIR
	TST_DIR=${TST_DIR}
endif
ifndef PYB_DIR
	PYB_DIR=${PYB_DIR}
endif
ifndef PYC_DIR
	PYC_DIR=${PYC_DIR}
endif
ifndef SHARE_DIR
	SHARE_DIR=${SHARE_DIR}
endif
EOF
}

addProjectExportTo() {
cat << EOF >> $1
ifndef ${projectName}_DIR
	${projectName}_DIR=${EXPORT_DIR}
endif
EOF
}

addToMakefileThisVariableAndValue() {
cat << EOF >> $1
ifndef $2
	$2=$3
endif
EOF
}

addToMakefileThisVariableAndValueNoIf() {
cat << EOF >> $1
	$2=$3
EOF
}

# Obtain project specific configurations - need to do subdirectories first so the project-config.sh in the
#	current directory doesn't add its stuff to Makefile.system too early
currdir=${PROJ_ROOT}

# Do the config in the order they are listed in Makefile.def
if test -r makefile; then
	getVariableFromMakefile subproj makefile
	makeDirectories="$subproj ."
else
	makeDirectories="."
fi

for dir in $makeDirectories; do
	# Check if the directory is a project
	if test "`ls ${dir}/configure.sh 2> /dev/null`X" = "${dir}/configure.sh""X" ; then
		if test ${dir} != "."; then
			${CP_F} Makefile.system ${dir}
		fi
		cd ${dir};
		if test "${USE_SEPARATE_BUILD}x" != "x"; then
			determineProjectVersion
			# If using separate_build, just set the variables, ignoring environment set ones for now
				
			# Add version number iff version option given and a value was retrived
			if test "${USE_VERSION}x" = "x"; then
				BLD_DIR=${BLD_BASE}/${dir}
				EXPORT_DIR=${EXPORT_BASE}/${dir}
			else
				if test "${VERSION}x" != "x"; then
					BLD_DIR=${BLD_BASE}/${dir}-v${VERSION}
					EXPORT_DIR=${EXPORT_BASE}/${dir}-v${VERSION}
				fi
			fi

			# Add argumented options
			if test "${OPTIONS}x" != "x"; then
				BLD_DIR=${BLD_DIR}-${OPTIONS}
				EXPORT_DIR=${EXPORT_DIR}-${OPTIONS}
			fi
			
			export BLD_DIR
			TMP_DIR=${BLD_DIR}/tmp
			export TMP_DIR
			BIN_DIR=${EXPORT_DIR}/bin
			export BIN_DIR
			LIB_DIR=${EXPORT_DIR}/lib
			export LIB_DIR
			INC_DIR=${EXPORT_DIR}/include
			export INC_DIR
			TST_DIR=${BLD_DIR}/tests
			export TST_DIR
			PYB_DIR=${EXPORT_DIR}/Python
			export PYB_DIR
			PYC_DIR=${EXPORT_DIR}/Python
			export PYC_DIR
			SHARE_DIR=${EXPORT_DIR}/share
			export SHARE_DIR
			EXAMPLES_DIR=${SHARE_DIR}/examples
			export EXAMPLES_DIR

		fi	# End separate_build

		addLocationInfoTo Makefile.system

		if test -x project-config.sh; then
			echo "Doing project configuration in $dir"
			# Source the project config to retrive values it obtains from config scripts
			. ./project-config.sh
			if ! test $? = "0" ; then
				# project-config.sh for this project has failed
				exit 1
			fi
			# Fetch the project name back from Makefile.system and make it uppercase
			projectName=`grep 'PROJECT\=' Makefile.system | cut -f 2 -d'='`
			if test "${projectName}x" = "x"; then
				projectName=${dir}
			fi
			projectNameLowerCase=${projectName}
			projectName=`echo ${projectName} | <&0 tr abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ >&1`

			# add env variables
			eval "${projectName}_DIR=${EXPORT_DIR}"
			export "${projectName}_DIR"
			
			# Set these new dirs. TODO: move to more organised location
			APPS_XML_DIR=${EXPORT_DIR}/lib/${projectName}
			export APPS_XML_DIR
			addToMakefileThisVariableAndValue Makefile.system APPS_XML_DIR ${APPS_XML_DIR}

			EXPORT_INCLUDE_DIR=${STG_INCLUDE_PATH}/${projectNameLowerCase}
			export EXPORT_INCLUDE_DIR
			addToMakefileThisVariableAndValueNoIf Makefile.system EXPORT_INCLUDE_DIR ${EXPORT_INCLUDE_DIR}

			EXAMPLES_DIR=${SHARE_DIR}/${projectNameLowerCase}
			export EXAMPLES_DIR
			addToMakefileThisVariableAndValueNoIf Makefile.system EXAMPLES_DIR ${EXAMPLES_DIR}

			if test "${USE_VERSION}" = "1"; then
				if test "${STGERMAIN_VERSION}x" = "x"; then
					getVariableFromMakefile VERSION makefile
					MODEL_PATH_PREFIX=${projectNameLowerCase}-v${VERSION}
				else 
					MODEL_PATH_PREFIX=StGermain-v${STGERMAIN_VERSION}
				fi
			else
				if test "${STGERMAIN_DIR}x" = "x"; then
					MODEL_PATH_PREFIX=${projectNameLowerCase}
				else
					MODEL_PATH_PREFIX=StGermain
				fi
			fi

			STG_INCLUDE_PATH=${EXPORT_DIR}/lib/${MODEL_PATH_PREFIX}
			export STG_INCLUDE_PATH
			addToMakefileThisVariableAndValueNoIf Makefile.system STG_INCLUDE_PATH ${STG_INCLUDE_PATH}

			EXPORT_INCLUDE_DIR=${STG_INCLUDE_PATH}/${projectNameLowerCase}
			export EXPORT_INCLUDE_DIR
			addToMakefileThisVariableAndValueNoIf Makefile.system EXPORT_INCLUDE_DIR ${EXPORT_INCLUDE_DIR}

			if ! test -d ${EXPORT_DIR}; then
				mkdir -p ${EXPORT_DIR}
			fi
			# export the Makefile.system
			${CP_F} Makefile.system ${EXPORT_DIR}/${projectName}.system

			
		elif ! test "${dir}" = "."; then
			echo "Project $dir does not contain a project-config.sh"
			echo "Please create a project-config.sh or remove this directory from the 'subproj' variable in makefile"
			exit 1
		fi

		# Mark this this sub project is being built as part of this project. This variable wont be exported. Can be used to relax validification checks of files.
		markAsBuilding ${projectName}

		cd ${currdir};
		if test ${dir} != "."; then
			addProjectExportTo Makefile.system
		fi
	fi
done

echo "Configure complete."
echo "Record of configuration in 'Config.log'"
