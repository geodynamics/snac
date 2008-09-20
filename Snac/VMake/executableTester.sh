#!/bin/sh
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
##
## Victorian Partnership for Advanced Computing (VPAC) Ltd, Australia
## (C) 2003 All Rights Reserved
##
## California Institute of Technology (Caltech), USA
## (C) 2003 All Rights Reserved
##
## Authors:
## 	Stevan M. Quenette, Senior Software Engineer, VPAC.
##	Stevan M. Quenette, Visitor in Geophysics, Caltech.
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
## 	Functions used in testing
##
## Assumptions:
##	MPI_DIR environment variable is set, else defaults to "/usr/local/mpich"
##	MPI_BINDIR environment variable is set, else defaults to "${MPI_DIR}/bin"
##	MPI_RUN environment variable is set, else defaults to "mpirun"
##	MPI_NPROC environment variable is set, else defaults to "-np"
##
## Comments:
##	None as yet.
##
## $Id: executableTester.sh 390 2008-02-19 01:12:00Z RobertTurnbull $
##
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# system info
if test -r './tmp.sh'; then
	. './tmp.sh'
fi

if test "${LOG_DIR}x" = "x"; then
	LOG_DIR="."
	export LOG_DIR
fi

cleanFileOfNonDeterministics() {
	file="$1"
	fileCleaned="$2"
	getValueFromMakefile_System GREP
	getValueFromMakefile_System SED
	cat ${file} | ${GREP} -v "TODO" | ${GREP} -v "(ptr)" | ${GREP} -v "(func ptr)" | ${GREP} -v "(context ptr)" | ${GREP} -v "Process" |  ${GREP} -v "sizeOfSelf" | ${GREP} -v "(version)" | ${GREP} -v "(path)" | ${GREP} -v "(secs)" | ${GREP} -v "(mins)" | ${GREP} -v "(hrs)" | ${GREP} -v "(time)" | ${SED} 's/ revision [0-9]*//g' > ${fileCleaned}

	# Test if grep thought that the file was binary, if so, then don't worry about cleaning the file
	if test "`cat ${fileCleaned}`" = "Binary file (standard input) matches" ; then
		cp ${file} ${fileCleaned}
	fi
}

runTest() {
	testname=$1
	shift;
	command="$@"

	# set soft time limit to 10 minutes
	ulimit -t 600

	# Set mpi dir. This is required on SGI implementations
	MPI_DIR=`pwd`
	# run the test
	if ! $command > ${output} 2> ${error}; then
		if test "${UPDATE_MODE}x" = "onx"; then
			printf "`$testname`: ";
		fi	
		echo "*Failed to run*"
		echo "  command was $command";
		echo "  Output stored in $output";
		echo "  Stderr stored in ${error}";
		return 1;
	fi
	rm -f ${error}
}

handleTestResult() {
	result=$1

	if test ! "${UPDATE_MODE}x" = "onx"; then
		printf "\tChecking output file $result: "
	fi
	if test ! -r $result; then
		echo "*Error*:- file doesn't exist:- abandoning test.";
		return 1;
	fi
	
	expected=$2
	singletest=`basename ${expected} .expected`
	diffs="${LOG_DIR}/${singletest}.diffs"
	savedResult="${LOG_DIR}/${singletest}.actual"
	mv -f $result $savedResult

	if test ! -r $expected; then
		if test "${UPDATE_MODE}x" = "onx"; then
			# update the expected value
			echo "No expected result -> creating $expected"
			mv $savedResult $expected
			return 0
		else
			echo "*Error*:- expected file doesn't exist:- abandoning test.";
			return 1;
		fi	
	fi

	# Clean Expected file
	expectedCleaned=${expected}.clean
	cleanFileOfNonDeterministics $expected $expectedCleaned
	
	# Clean Result File 
	savedResultCleaned=${savedResult}.clean
	cleanFileOfNonDeterministics $savedResult $savedResultCleaned

	# Check if the diff environment variable has been set
	if test "${DIFF}x" = "x" ; then
		# THIS SHOULD REALLY BE SET BY THE MAKEFILE.SYSTEM
		DIFF=diff
		export DIFF
	fi
	
	# check the outputs
	if ${DIFF} ${expectedCleaned} ${savedResultCleaned} > ${diffs}; then
		rm -f ${diffs} ${savedResult} ${expectedCleaned} ${savedResultCleaned};
		if test ! "${UPDATE_MODE}x" = "onx"; then
			echo "OK."
		fi
		return 0;
	else
		if test "${UPDATE_MODE}x" = "onx"; then
			# update the expected value
			echo "*Failed* - Patching expected file: $expected"
			mv $savedResult $expected
			rm -f $diffs ${savedResult} ${expectedCleaned} ${savedResultCleaned}
		else
			echo "*Failed*";
			printf "\t\tOutput stored in $savedResult\n";
			printf "\t\tDiffs with expected stored in ${diffs}\n";
			rm -f ${expectedCleaned} ${savedResultCleaned}
	                if test ! "${REGRESSION_RUN}x" = "x"; then
	                        if test -x ${BLD_DIR}/bin/regresstorCheckDiff; then
        	                        ${BLD_DIR}/bin/regresstorCheckDiff \
                	                        RunID=${RUNID} \
                        	                TestName=${singletest} \
                                	        URL=${REGRESSTOR_URL} \
                                        	Passed=0 \
	                                        OutputLocation=${diffs} \
        	                                UnitName=Unknown
                	        fi
	                fi
			return 1;
		fi
	fi
}


handleStdOutTest() {
	stdOutFile=$1

	if test ! "${UPDATE_MODE}x" = "onx"; then
		printf "\tChecking standard output: "
	fi
	if test ! -r $stdOutFile; then
		echo "*Error*:- file doesn't exist:- abandoning test.";
		return 1;
	fi
	
	expected=$2
	singletest=`basename ${expected} .expected`
	diffs="${LOG_DIR}/${singletest}.diffs"

	if test ! -r $expected; then
		if test "${UPDATE_MODE}x" = "onx"; then
			# update the expected value
			echo "No expected stdOutFile -> creating $expected"
			mv $stdOutFile $expected
			return 0
		else
			echo "*Error*:- expected file doesn't exist:- abandoning test.";
			return 1;
		fi	
	fi

	# Clean Expected file
	expectedCleaned=${expected}.clean
	cleanFileOfNonDeterministics $expected $expectedCleaned
	# Clean Result File 
	stdOutFileCleaned=${stdOutFile}.clean
	cleanFileOfNonDeterministics $stdOutFile $stdOutFileCleaned

	# Check if the diff environment variable has been set
	if test "${DIFF}x" = "x" ; then
		# THIS SHOULD REALLY BE SET BY THE MAKEFILE.SYSTEM
		DIFF=diff
		export DIFF
	fi

	# check the outputs
	if ${DIFF} ${expectedCleaned} ${stdOutFileCleaned} > ${diffs}; then
		rm -f ${diffs} ${stdOutFile} ${expectedCleaned} ${stdOutFileCleaned};
		if test ! "${UPDATE_MODE}x" = "onx"; then
			echo "OK."
		fi
		return 0;
	else
		if test "${UPDATE_MODE}x" = "onx"; then
			# update the expected value
			echo "*Failed* - Patching expected file: $expected"
			mv $stdOutFile $expected
			rm -f $diffs  ${expectedCleaned} ${stdOutFileCleaned}
		else
			echo "*Failed*";
			printf "\t\tOutput stored in $stdOutFile\n";
			printf "\t\tDiffs with expected stored in ${diffs}\n";
			rm -f ${expectedCleaned} ${stdOutFileCleaned}
	                if test ! "${REGRESSION_RUN}x" = "x"; then
	                        if test -x ${BLD_DIR}/bin/regresstorCheckDiff; then
        	                        ${BLD_DIR}/bin/regresstorCheckDiff \
                	                        RunID=${RUNID} \
                        	                TestName=${singletest} \
                                	        URL=${REGRESSTOR_URL} \
                                        	Passed=0 \
	                                        OutputLocation=${diffs} \
        	                                UnitName=Unknown
                	        fi
	                fi
			return 1;
		fi
	fi
}

setUpdateModeFromArg() {
	if test "${UPDATE_MODE}" = "on" || test "`echo ${1} | cut -c -5`" = "patch" ; then
		UPDATE_MODE="on"
	else
		UPDATE_MODE="off"
	fi
	export UPDATE_MODE
}


# Note: this function assumes several Makefile.system variables are available
runAndHandleSystemTest() {
	testname=`basename $2 .sh`
	executable=`echo $1 | cut -f 1 -d ' '`
	#if there are more than one arguments (i.e. there is a space) everything after the executable goes into execInput
	if test "`echo $1 | grep ' '`" = "`echo $1`" ; then
		execInput=`echo $1 | cut -f 2- -d ' '`
	fi 

	# Set UPDATE_MODE flag if this is supposed to patch the tests
	setUpdateModeFromArg ${3}

	nproc=`echo ${testname} | cut -d . -f 2 | cut -c 4`
	procToWatch=`echo ${testname} | cut -d . -f 2 | cut -f 1 -d 'o'`

	execInput="$procToWatch $execInput"

	# Finds Base Functions Script 
	BUILD_FUNCTIONS=build-functions.sh
	until test -r ${BUILD_FUNCTIONS} ; do 
		BUILD_FUNCTIONS=../${BUILD_FUNCTIONS}
	done
	. ${BUILD_FUNCTIONS}

	if test "${OUTPUT_DIR}x" = "x"; then
		 OUTPUT_DIR="./"
		 export OUTPUT_DIR
	fi
	if test "${EXPECTED_DIR}x" = "x"; then
		 EXPECTED_DIR="./"
		 export EXPECTED_DIR
	fi
	if test "${LOG_DIR}x" = "x"; then
		LOG_DIR="./"
		export LOG_DIR
	fi

	makePathAbsolute OUTPUT_DIR
	makePathAbsolute EXPECTED_DIR
	makePathAbsolute LOG_DIR

	# Gets Values from Makefile.system
	getValueFromMakefile_System EXPORT_DIR
	getValueFromMakefile_System BIN_DIR
	getValueFromMakefile_System TST_DIR
	getValueFromMakefile_System BLD_DIR
	getValueFromMakefile_System MPI_DIR
	getValueFromMakefile_System MPI_BINDIR
	getValueFromMakefile_System MPI_RUN_COMMAND
	getValueFromMakefile_System MPI_RUN
	getValueFromMakefile_System MPI_MACHINES
	getValueFromMakefile_System MPI_NPROC

	if test "${nproc}x" = "x"; then
		nproc=1
	fi

	error=${LOG_DIR}/${testname}.error
	output=${LOG_DIR}/${testname}.out
	export error
	export output

	if test ! "${UPDATE_MODE}x" = "onx"; then
		printf "$testname: running using ${nproc} procs...\n";
	fi
	
	# put call to softlink builder here
	
	# need to do this softlink so that MPI can find the executable...it doesn't

	if ! createSoftlinkToExecutable ${executable}; then
		echo 'create Softlink Failed'
		if test ! "${REGRESSION_RUN}x" = "x"; then
			if test -x ${BLD_DIR}/bin/regresstorCheckExecution; then
				echo "Error: requested test executable \"${executable}\" not found in neither BIN_DIR ${BIN_DIR} nor TST_DIR ${TST_DIR}" > ${error}
				${BLD_DIR}/bin/regresstorCheckExecution \
					RunID=${RUNID} \
					TestName=${testname} \
					URL=${REGRESSTOR_URL} \
					Passed=0 \
					OutputLocation=${error} \
					UnitName=Unknown
				rm ${error}
			fi
		fi;
		exit 1
	fi
		# To run on SGI machines:
	getValueFromMakefile_System MPI_IMPLEMENTATION
	if test ! "`echo $MPI_IMPLEMENTATION | grep 'sgi'`x" = "x"; then
			MPI_SGIIMPLEMENT=" -d `pwd` "
	else
			MPI_SGIIMPLEMENT=""		
	fi	
	
		MPI_EXEC="${MPI_RUN} ${MPI_SGIIMPLEMENT} ${MPI_MACHINES} ${MPI_NPROC} ${nproc}"

	if runTest $testname "${MPI_EXEC} $softlink" $execInput; then
		retValue=0
		passed="true"
		expectedFileFound="false"
		
		# Try the normal .expected (stdout) to test - only compares if file is found
		if test -r ${EXPECTED_DIR}/${testname}.expected ; then
			if ! handleStdOutTest ${LOG_DIR}/${testname}.out ${EXPECTED_DIR}/${testname}.expected ; then
				passed="false";
				if test ! "${UPDATE_MODE}x" = "onx"; then
					printf "\tResult: *Failed*\n";
				fi	
				rm -f ${softlink}
				exit 1
			fi
			expectedFileFound="true"
		fi
		
		# Try all other .expected files
		for fullCheckFile in `ls -1 ${EXPECTED_DIR}/${testname}.*.expected 2> /dev/null` ; do
			intermediateCheckFile=`basename $fullCheckFile .expected`
			checkFile=`echo $intermediateCheckFile | sed "s/${testname}.//"`
			
			if ! handleTestResult ${OUTPUT_DIR}/${checkFile} $fullCheckFile ; then
				passed="false";
				if test ! "${UPDATE_MODE}x" = "onx"; then
					printf "\tResult: *Failed*\n";
				fi	
				rm -f ${softlink}
				exit 1
			fi
			expectedFileFound="true"
		done

		# Check if no expected files found
		if test $expectedFileFound = "false"; then
			passed="false"
			#if update mode, then ask user for each of the output files
			if test "${UPDATE_MODE}" = "on"; then
				echo "No expected files found for ${testname}: patching"
				echo "Do you want stdout to be made an expected file? [y/n]"
				read -e MAKE_EXPECTED
				if test "$MAKE_EXPECTED" = "y" ; then
					expectedFile=${EXPECTED_DIR}/${testname}.expected
					echo "Creating ${expectedFile}"
					cp ${LOG_DIR}/${testname}.out ${expectedFile}
				fi				
			
				for outputFile in `ls -1 ${OUTPUT_DIR}/ 2> /dev/null` ; do
					echo "Do you want \"$outputFile\" to be made an expected file? [y/n]"
					read -e MAKE_EXPECTED
					if test "$MAKE_EXPECTED" = "y" ; then
						expectedFile=${EXPECTED_DIR}/${testname}.${outputFile}.expected
						echo "Creating ${expectedFile}"
						mv ${OUTPUT_DIR}/$outputFile ${expectedFile}
					fi
				done
			else 
				printf "\tNo expected files found: *Failed*\n";
			fi
			rm -f ${softlink}
			exit 1
		fi
		
		if $passed = "true"; then
			if test ! "${UPDATE_MODE}x" = "onx"; then
				printf "\tResult: *Passed*\n";
				if test ! "${REGRESSION_RUN}x" = "x"; then
					if test -x ${BLD_DIR}/bin/regresstorCheckDiff; then
						${BLD_DIR}/bin/regresstorCheckDiff \
							RunID=${RUNID} \
							TestName=${testname} \
							URL=${REGRESSTOR_URL} \
							Passed=1 \
							OutputLocation=none\
							UnitName=Unknown
					fi
				fi
			fi
			rm -f ${LOG_DIR}/$testname.out;
		fi
	else # (test failed to run)
		retValue=1
		if test ! "${REGRESSION_RUN}x" = "x"; then
			if test -x ${BLD_DIR}/bin/regresstorCheckExecution; then
				${BLD_DIR}/bin/regresstorCheckExecution \
					RunID=${RUNID} \
					TestName=${testname} \
					URL=${REGRESSTOR_URL} \
					Passed=0 \
					OutputLocation=${error}\
					UnitName=Unknown
			fi
		fi
	fi	

	rm -f ${softlink}

	return ${retValue}
}

runAndHandleSystemTestStdLocations() {
	EXPECTED_DIR="./expected"
	LOG_DIR="./log"
	OUTPUT_DIR="./output"

	export EXPECTED_DIR
	export LOG_DIR
	export OUTPUT_DIR

	runAndHandleSystemTest "$1" "$2" "$3" 
}

# Creating a shell script that creates a softlink to an executable in the 
# bin or test directory
createSoftlinkToExecutable() {
	# Define executable name
	executable=${1}
	# VMAKE_DIR is optional, and has to point directly to /VMake/ directory
	VMAKE_DIR=${2}
	# Finds Base Functions Script
	if test "${VMAKE_DIR}x" = "x"; then
		BUILD_FUNCTIONS=build-functions.sh
		until test -r ${BUILD_FUNCTIONS} ; do 
			BUILD_FUNCTIONS=../${BUILD_FUNCTIONS}
		done
		. ${BUILD_FUNCTIONS}
	elif test -d ${VMAKE_DIR}; then
	    . ${VMAKE_DIR}/build-functions.sh
	else
		echo "Error in createSoftlinkToExecutable():"
		echo "${VMAKE_DIR} not a directory."
		echo "Either remove or change to reference to VMake directory"
		return 1		
	fi
	
	# Gets Values from Makefile.system
	if test "${VMAKE_DIR}x" = "x"; then	
		getValueFromMakefile_System BIN_DIR 
		getValueFromMakefile_System TST_DIR 
	else
		# This line assumes that there will be a Makefile.system
		# file in the directory below the VMake directory.
		getVariableFromMakefile BIN_DIR ${VMAKE_DIR}/../Makefile.system
		getVariableFromMakefile TST_DIR ${VMAKE_DIR}/../Makefile.system
	fi		
	# need to do this softlink so that MPI can find the executable
	export softlink="./${executable}-softlink"
	if test -x ./${executable} && ! test -h ./${executable}; then
		# for consistency reasons
		ln -sf ./${executable} ${softlink}
	elif test -x ${BIN_DIR}/${executable}; then
		ln -sf ${BIN_DIR}/${executable} ${softlink}
	elif test -x ${TST_DIR}/${executable}; then
		ln -sf ${TST_DIR}/${executable} ${softlink}
	else
		echo "  Error: requested test executable \"${executable}\" not found in neither BIN_DIR ${BIN_DIR} nor TST_DIR ${TST_DIR}"
	## TODO maybe add in exit conditions here
		return 1	
	fi
	return 0
}

removeSoftlinkToExecutable() {
	executable=${1}

	export softlink="./${executable}-softlink"
	if test -x ${softlink}; then
		rm -f ${softlink}
	fi

}

softlinkCreateOrRemove() {
	executable=${1}
	action=${2}
	#VMAKE_DIR is optional, and has to point into a /VMake/ directory.
	VMAKE_DIR=${3}
	# Check whether values have been input
	if test "${executable}x" != "x" && test "${action}x" != "x"; then
		executable=${1}
		action=${2}
		
		# Either create or remove link based on input arguments
		if [ ${action} = create ] ; then
			if ! createSoftlinkToExecutable ${executable} ${VMAKE_DIR}; then
				return 1
				
			fi
		else
			if [ ${action} = remove ] ; then
				removeSoftlinkToExecutable ${executable}
			else
				echo 'Cannot execute action: '${action}
				echo 'Please enter in format:'
				echo '$ softlinkCreateOrRemove ${executable} ${action [create/remove]}'
				return 1
			fi		
		fi
	else
		echo 'Missing inputs to softlinkCreateOrRemove'
		echo 'Please enter in format:'
		echo '$ softlinkCreateOrRemove ${executable} ${action [create/remove]}'	
		return 1
	fi
	return 0
}

RunMPICommand() {
# Executes command with form: {testfilename} {executable} {commandsToExecutable}

	BUILD_FUNCTIONS=build-functions.sh
	until test -r ${BUILD_FUNCTIONS} ; do 
		BUILD_FUNCTIONS=../${BUILD_FUNCTIONS}
	done
	. ${BUILD_FUNCTIONS}
 
	#Extract out name of file and remove from string/
	testname=${1}
	#echo "testname = $testname"
	shift;
	executableName=${1}
	#echo "executableName = $executableName"
	shift;
	command="$@"

	# Create softlink
	softlinkCreateOrRemove ${executableName} 'create'

	#Extract out the processor info from testname
	nproc=`echo ${testname} | cut -d . -f 2 | cut -c 4`
	procToWatch=`echo ${testname} | cut -d . -f 2 | cut -f 1 -d 'o'`

	#Get variables to run mpi
	getValueFromMakefile_System MPI_RUN
	getValueFromMakefile_System MPI_MACHINES
	getValueFromMakefile_System MPI_NPROC
	# To run on SGI machines:
	getValueFromMakefile_System MPI_IMPLEMENTATION
	if test ! "`echo $MPI_IMPLEMENTATION | grep 'sgi'`x" = "x"; then
			MPI_SGIIMPLEMENT=" -d `pwd` "
	else
			MPI_SGIIMPLEMENT=""		
	fi	

	MPI_EXEC="${MPI_RUN} ${MPI_SGIIMPLEMENT} ${MPI_MACHINES} ${MPI_NPROC} ${nproc}"
	echo ${MPI_EXEC}
	# run command
	${MPI_EXEC} ./${executableName}-softlink ${procToWatch} ${command}

	# /Remove softlink
	softlinkCreateOrRemove ${executableName} 'remove'

}

RunScienceTestCheck() {

	getValueFromMakefile_System REGRESSTOR_URL
	getValueFromMakefile_System REGRESSION_RUN
	getValueFromMakefile_System BLD_DIR
	getValueFromMakefile_System RUNID

	outputFileName=${1}
	expectedFileName=${2}
	testname=${3}
	
	if ! test -r ${expectedFileName} ; then
		passed="false"
		printf "\tResult: *Failed*\n";
		printf "\tCan't find expected file ${expectedFileName}\n"
		if test ! "${REGRESSION_RUN}x" = "x"; then
			 if test -x ${BLD_DIR}/bin/regresstorCheckDiff; then
				 ${BLD_DIR}/bin/regresstorCheckDiff \
					 RunID=${RUNID} \
					 TestName=${singletest} \
					 URL=${REGRESSTOR_URL} \
					 Passed=0 \
					 OutputLocation=${diffs} \
					 UnitName=Unknown
			 fi
	   fi		
	else
	
		if ! handleTestResult "${outputFileName}" "${expectedFileName}" ; then
			passed="false"
		else 
			passed="true"	
		fi
	fi		
	if $passed = "true"; then
		printf "\tResult: *Passed*\n";
		if test ! "${REGRESSION_RUN}x" = "x"; then
			if test -x ${BLD_DIR}/bin/regresstorCheckDiff; then
				${BLD_DIR}/bin/regresstorCheckDiff \
					RunID=${RUNID} \
					TestName=${testname} \
					URL=${REGRESSTOR_URL} \
					Passed=1 \
					OutputLocation=none\
					UnitName=Unknown
			fi
		fi
		echo ${LOG_DIR}
		rm -f ${LOG_DIR}/$testname.out;
		rm -f ${LOG_DIR}/$testname.error;
	fi
}

RunScienceTestCheckStdLocations() {

	testName=${1}
	resultsFilename=${2}

	getValueFromMakefile_System MACHINE
	
	EXPECTED_DIR="./expected/${testName}"
	LOG_DIR="./log"
	OUTPUT_DIR="./output/${testName}/$MACHINE"

	export EXPECTED_DIR
	export LOG_DIR
	export OUTPUT_DIR

	RunScienceTestCheck "${OUTPUT_DIR}/${resultsFilename}" "${EXPECTED_DIR}/${resultsFilename}.expected" "${testName}"
}
