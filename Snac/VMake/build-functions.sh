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
## $Id: Makefile.def 2338 2004-11-19 04:51:11Z SteveQuenette $
##
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


#Set up the which command (used for all tests)
if test "${WHICH}x" = "x"; then
	WHICH="/usr/bin/which"
	if test ! -x "${WHICH}"; then
		echo "Error: default \"which\" command/default of \"$WHICH\" not found/executable. Set the WHICH variable."
		exit 1
	fi
	export WHICH
fi


#Function to see if a "which" command failed to find a binary (needs to handle Os X where Stderr goes to Stdout
# for which command)
whichFailed() {
	if test "${1}x" = "x"; then return 0; fi
	#For Max OsX which implementation
	if test `echo $1 | cut -f 1 -d' '` = "no"; then return 0; fi
	if test `echo $1 | cut -f 3 -d' '` = "aliased"; then return 0; fi
	#For OSF1 which implementation at ANU
	if test `echo $1 | cut -f 2 -d' '` = "Cannot"; then return 0; fi
	return 1;
}

#Set up the tr command (used for check functions below)
if test "${TR}x" = "x"; then
	TR=`${WHICH} tr 2> /dev/null`
	if whichFailed "${TR}"; then
		TR="/bin/tr"
	fi
	if test ! -x "${TR}"; then
		echo "Error: default \"tr\" command of \"$TR\" not found/executable. Set the TR variable."
		exit 1
	fi
	export TR
else
	if whichFailed `${WHICH} ${TR} 2> /dev/null`; then
		echo "Error: given \"tr\" command of \"$TR\" not found/executable. Correct the TR variable."
		exit 1
	fi
fi


#Functions to check validity
assertValidExecutable ()
{
	if test ! -x "${1}"; then
		upperCommand=`echo ${2} | tr '[:lower:]' '[:upper:]'`
		echo "Error: $2 binary \"$1\" not found in path, and defaults were not found/executable."
		printf "Set the $upperCommand "
		if test "${3}x" != "x"; then
			printf "(or $3)"
		fi
		printf " environment variable/s, or modify your path appropriately.\n"
		exitWithErrorIfNotOptional
	fi
}


warnValidExecutable ()
{
	resolveVariable theExec ${1}
	if test ! -x "${theExec}"; then
		upperCommand=`echo ${2} | tr '[:lower:]' '[:upper:]'`
		echo "Warning: $2 binary \"$theExec\" not found in path, and defaults were not found/executable."
		printf "If you require this package, check it is installed, built, your path is correct, and that the $upperCommand "
		if test "${4}x" != "x"; then
			printf "(or $4)"
		fi
		printf " environment variable(s) are set.\n"
		return 1
	fi
	return 0
}

assertValidExecutableInput ()
{
	testResult=`${WHICH} "$1" 2> /dev/null`
	if whichFailed "$testResult"; then
		upperCommand=`echo ${2} | tr '[:lower:]' '[:upper:]'`
		echo "Error: Given name/path to $2 command of \"$1\" not found or not valid."
		printf "Correct the $upperCommand "
		if test "${3}x" != "x"; then
			printf "(or $3)"
		fi
		printf " environment variable/s, or modify your path appropriately.\n"
		exitWithErrorIfNotOptional
	fi
}

warnValidExecutableInput ()
{
	testResult=`${WHICH} "$1" 2> /dev/null`
	if whichFailed "$testResult"; then
		upperCommand=`echo ${2} | tr '[:lower:]' '[:upper:]'`
		echo "Warning: Given name/path to $2 command of \"$1\" not found or not valid."
		printf "If needed, correct the $upperCommand "
		if test "${3}x" != "x"; then
			printf "(or $3)"
		fi
		printf " environment variable/s, or modify your path appropriately.\n"
		return 1
	fi
	return 0
}

#Functions to check validity
assertValidFile ()
{
	if test ! -r "${1}"; then
		echo "Error: File/dir \"$1\" not found, meaning package $2 is not installed, not build, or some variables are wrong."
		printf "Check the package is installed, built, and that the $3 "
		if test "${4}x" != "x"; then
			printf "(or $4)"
		fi
		printf " environment variable/s are set correctly.\n"
		exitWithErrorIfNotOptional
	fi
}

#Functions to check validity
warnIfNotValidFile ()
{
	if test ! -r "${1}"; then
		echo "Warning: File/dir \"$1\" not found, meaning package $2 is not installed, not build, or some environment variables are incorrect."
		printf "If you require this package, check it is installed, built, and that the $3 "
		if test "${4}x" != "x"; then
			printf "(or $4)"
		fi
		printf " environment variable/s are set correctly.\n"
	fi
}

assertValidDirectory ()
{
	if test ! -d "${1}"; then
		echo "Error: Directory \"$1\" not found, meaning package $2 is not installed, not build, or some variables are wrong."
		printf "Check the package is installed, built, and that the $3 "
		if test "${4}x" != "x"; then
			printf "(or $4)"
		fi
		printf " environment variable/s are set correctly.\n"
		exitWithErrorIfNotOptional
	fi
}

# Sets value in Makefile.system with value from environment or from default
setValueWithDefault() {
	echo "ifndef $1" >> Makefile.system
	eval "INPUTVALUE=\"\$$1\""
	
	if test "${INPUTVALUE}x" = "x" ; then
		echo "	$1=$2" >> Makefile.system
		eval "$1=\"$2\"" 2> /dev/null &&
		export ${1} 
	else
		echo "	$1=${INPUTVALUE}" >> Makefile.system
	fi
	echo "endif" >> Makefile.system
}

# Sets value in Makefile.system
setValue() {
	echo "$1=$2" >> Makefile.system
	eval "$1=\"$2\"" 2> /dev/null &&
	export ${1}
}

makePathAbsolute () {
	eval "INPUTVALUE=\"\$$1\"" 
	if test "${INPUTVALUE}" = "x" ; then
		eval "${1}=`pwd`"
	else
		eval "${1}=`cd ${INPUTVALUE} ; pwd`"
	fi
	export ${1}
}


appendVariableIfReadable() {
	if test -r ${2} ; then
		echo "${1}+=${3}" >> Makefile.system
		eval "$1=\"\$$1\"\ \"$3\"" 2> /dev/null &&
		export ${1}
	fi
}

getValueFromMakefile_System() {
	MAKEFILE_SYSTEM=Makefile.system
	until test -r ${MAKEFILE_SYSTEM} ; do 
		MAKEFILE_SYSTEM=../${MAKEFILE_SYSTEM}
	done

	eval "INPUTVALUE=\"\$$1\"" 
	if test "${INPUTVALUE}x" = "x" ; then
		# The "head" filter is a bit of a hack due to a known problem of MPI info appearing in the Makefile.system
		#  files twice at the moment. Once that's fixed, it really should be changed into a firewall such that
		#  only one result comes back.
		FOUND_VALUE=`grep "${1}=" ${MAKEFILE_SYSTEM} | head -n 1 | cut -d'=' -f2`
		eval "${1}=\"${FOUND_VALUE}\"" 2> /dev/null
	fi
}

getVariableFromMakefile() {
	theVariable=$1
	theMakefile=$2
	echotemp="./echoMakefile"
cat <<EOF > ${echotemp}
ifdef TARGET_MAKEFILE
	include \$(TARGET_MAKEFILE)
endif
echo-var:
	@echo \$(\$(VAR))
EOF
	theValue=`make echo-var -f ${echotemp} TARGET_MAKEFILE=${theMakefile} VAR=${theVariable}`
	rm ${echotemp}
	eval "${theVariable}=\"${theValue}\"" 2> /dev/null
}

resolveVariable() {
	theVar=$1
	resolvedResult=$2
	while test "`echo $resolvedResult | grep '[\$]'`x" != "x"
	do
		eval resolvedResult=$resolvedResult
	done
	eval $theVar=$resolvedResult
}



### For compiling & running code tests in configure

cleanConftest() {
	CONFTEST_DIR=$1
	${MAKE} -C ${CONFTEST_DIR} clean > /dev/null 2>&1
	return $?
}
compileConftest() {
	CONFTEST_DIR=$1
	cleanConftest ${CONFTEST_DIR}
	echo "    Compiling config test ${CONFTEST_DIR} ..."
	${MAKE} -C ${CONFTEST_DIR} > /dev/null 2>&1
	if ! test -x ${CONFTEST_DIR}/conftest; then
		echo "        ...Compile failed"
		return 1
	fi
	return 0;
}
runConftest() {
	CONFTEST_DIR=$1
	RESULT_VAR=$2
	echo "    Running   config test ${CONFTEST_DIR} ..."
	if ! test -x ${CONFTEST_DIR}/conftest; then
		EXIT_VALUE=1
	else
		RESULT_TEXT=`${CONFTEST_DIR}/conftest`
		EXIT_VALUE=$?
		eval "${RESULT_VAR}=$RESULT_TEXT"
		return ${EXIT_VALUE}
	fi
	if ! test "${EXIT_VALUE}" = "0"; then
		echo "        ...Run failed"
		return ${EXIT_VALUE}
	fi
	return 0
}
DoCompileConftest() {
	CONFTEST_DIR=$1
	if ! compileConftest ${CONFTEST_DIR}; then
		return 1
	fi
	cleanConftest ${CONFTEST_DIR}
	return 0
}
DoCriticalCompileConftest() {
	if ! DoCompileConftest $1; then
		echo "Critical compile conftest failed, -> exiting configure process."
		exit 1
	fi
}
DoConftest() {
	CONFTEST_DIR=$1
	RESULT_VAR=$2
	if ! compileConftest ${CONFTEST_DIR}; then
		return 1
	fi
	if ! runConftest ${CONFTEST_DIR} ${RESULT_VAR}; then
		return 1
	fi
	cleanConftest ${CONFTEST_DIR}

	return 0
}

DoCritialConftest() {
	if ! DoConftest $1 $2; then
		echo "Critical compile conftest failed, -> exiting configure process."
		exit 1
	fi
}


parsePackageConfigOptions() {
	# Unset all possible options for package configs
	unset PACKAGE_OPTIONAL

	while test $# -gt 0; do
		case $1 in
			--optional)
				export PACKAGE_OPTIONAL=1;
				;;
#			*)
#				echo "Unknown argument $1 for package config"
#				;;
		esac
		shift
	done
}

exitWithErrorIfNotOptional() {
	if test "${PACKAGE_OPTIONAL}x" = "x"; then
		exit 1
	fi
}

exitWithErrorIfNotOptionalWithMessage() {
	if test "${PACKAGE_OPTIONAL}x" = "x"; then
		echo "    *** Error: Configuration has failed because $i installation $2 is invalid"
		exit 1
	else
		echo "     ($1 will be unavailable)"
	fi
}

dumpPkgConfigFile() {

	dumpdir=$1
	if ! test -d "${dumpdir}"; then
		mkdir -p ${dumpdir}
	fi

	INC_VAR=`packages="PETSC MPI XML MATH DL HYPRE" make INCLUDES-echo | cut -f 2 -d =`
	LIB_VAR=`packages="PETSC MPI XML MATH DL HYPRE" make EXTERNAL_LIBS-echo | cut -f 2 -d = | sed 's/\-Xlinker \-rpath \-Xlinker /\-Wl,\-rpath,/g'`

	echo "    Creating $dumpdir/stgermain.pc"
cat <<EOF > $dumpdir/stgermain.pc
prefix=${EXPORT_DIR}
exec_prefix=${EXPORT_DIR}/bin
libdir=${EXPORT_DIR}/lib
includedir=${EXPORT_DIR}/include

Name: StGermain
Description: The StGermain Framework
Version: 0
Requires:
Libs: -L${EXPORT_DIR}/lib -lStGermain  ${LIB_VAR}
Cflags: ${INC_VAR}
EOF
}

markAsBuilding() {
	echo "    $1 Marked as a project to be built"
cat <<EOF > ./create-env.tmp
BUILDING_$1=1
EOF
	. ./create-env.tmp
	rm create-env.tmp
}


executeProgramWithTimeOut() {
	# Get my process id
	myid=$$

	# remove leading '-'
	# on macs, bash is listed as '-bash' which grep does not like as an argument
	myname=`echo $0 | sed 's/^\-//'`

	if ! test $# -eq 2; then
		echo "Usage:"
		echo "./exec.sh (timeout) (program)"
		return
	fi

	timeout=$1
	if test $timeout -lt 0; then
		echo "timeout must be greater than 0 seconds"
		return
	fi

	commandline=$2
	program=`echo $commandline | cut -f 1 -d ' '`

	# Run the test as a background process
	echo "Process id $myid Running program '$commandline'"
	$commandline &

	elapsedtime="0"
	while [ $elapsedtime -lt $timeout ]
	do
		sleep 1

		# Check alive
		programid=`ps xo pid,ppid,command 2> /dev/null | grep $program | grep -v grep | grep -v $myname | grep $myid | sed 's/^[ ]*//' | cut -f 1 -d ' '`
		if test "${programid}x" != "x"; then
			elapsedtime=$[$elapsedtime+1]
		else
			# exit loop
			finishtime=$[$elapsedtime+1]
			elapsedtime=$timeout
		fi
	done

	# Search for whether the test program has finished
	# Explaination:
	#    - Get processes
	#    - Get matches of the of the program name
	#         -> excluding grep
	#         -> excluding this script (arguments)
	#    - Which has this script id in it (the parent id)
	#    - trim leading white space (mac ps adds this)
	#    - Take the id value
	programid=`ps xo pid,ppid,command 2> /dev/null | grep $program | grep -v grep | grep -v $myname | grep $myid | sed 's/^[ ]*//' | cut -f 1 -d ' '` 
	if test "${programid}x" != "x"; then
		echo "Process has not finished in $timeout seconds...killing"
		kill -9 $programid > /dev/null 2>&1
	else
		echo "Process finished on its own in $finishtime seconds"
	fi
}
