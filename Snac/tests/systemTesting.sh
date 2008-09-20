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
## <copyright-release-tag>
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
## $Id: systemTesting.sh 3018 2005-06-10 06:56:52Z AlanLo $
##
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# system info
if test -r './tmp.sh'; then
	. './tmp.sh'
fi


runTest() {
	testname=$1
	output=${testname}.out
	error=${testname}.error
	shift;
	command="$@"
	# set soft time limit to 10 minutes
	ulimit -S -t 600
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
	expected=$2
	singletest=`basename ${expected} .expected`
	diffs="${singletest}.diffs"
	savedResult="${singletest}.actual"
	mv -f $result $savedResult

	# check the outputs
	if diff ${expected} ${savedResult} > ${diffs}; then
		rm -f ${diffs} ${savedResult};
		if test "${UPDATE_MODE}x" = "x"; then
			echo "Passed"
			if test ! "${REGRESSION_RUN}x" = "x"; then
				if test -x ${BLD_DIR}/bin/regresstorCheckDiff; then
					${BLD_DIR}/bin/regresstorCheckDiff \
						RunID=${RUNID} \
						TestName=${singletest} \
						URL=${REGRESSTOR_URL} \
						Passed=1 \
						OutputLocation=none\
						UnitName=Unknown
				fi
			fi
		fi
		return 0;
	else
		if test "${UPDATE_MODE}x" = "onx"; then
			# update the expected value
			echo Patching expected file: $expected
			mv $savedResult $expected
			rm -f $diffs 
		else
			echo "*Failed*";
			echo "  Output stored in $savedResult";
			echo "  Diffs with expected stored in ${diffs}";
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
