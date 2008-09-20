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
## 	Launcher of apps in a generic fashion.
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
## $Id: tester.sh 3018 2005-06-10 06:56:52Z AlanLo $
##
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# Be Bourne compatible
if test -n "${ZSH_VERSION+set}" && (emulate sh) >/dev/null 2>&1; then
	emulate sh
	NULLCMD=:
elif test -n "${BASH_VERSION+set}" && (set -o posix) >/dev/null 2>&1; then
 	set -o posix
fi

# handle input
program=${1}
if test "x${2}" = "x"; then
	nproc=1
else
	nproc=${2}
fi
if test "x${3}" = "x"; then
	rank=0
else
	rank=${3}
fi
fulltest=${program}.${rank}of${nproc}
expected=${fulltest}.expected
cleanedExpected=${expected}.cleaned
out=${fulltest}.out
cleanedOut=${out}.cleaned
diffs=${fulltest}.diffs
error=${fulltest}.error

# system info
if test -r ./tmp.sh; then
	. ./tmp.sh
fi
if test "x${MPI_DIR}" = "x"; then
	MPI_DIR="/usr/local/mpich"
fi
if test "x${MPI_BINDIR}" = "x"; then
	MPI_BINDIR="${MPI_DIR}/bin"
fi
if test "x${MPI_RUN}" = "x"; then
	MPI_RUN="${MPI_BINDIR}/mpirun"
fi
if test "x${MPI_NPROC}" = "x"; then
	MPI_NPROC="-np"
fi
EXEC="${MPI_RUN} ${MPI_MACHINES} ${MPI_NPROC} ${nproc}"

if test "x${TST_DIR}" = "x"; then
	TST_DIR=.
fi

if test "x${GREP}" = "x"; then
	GREP="grep"
fi

if test "${UPDATE_MODE}x" = "x"; then
	printf "`basename $fulltest`: ";
fi	

# run the test
if ${EXEC} ${TST_DIR}/${program} ${rank} > ${out} 2> ${error}; then
	${GREP} -v "TODO" ${out} | ${GREP} -v "(ptr)" | $GREP -v "(func ptr)" | $GREP -v "(context ptr)" | ${GREP} -v "Process" |  ${GREP} -v "sizeOfSelf" > ${cleanedOut};
else
	if test "${UPDATE_MODE}x" = "onx"; then
		printf "`basename $fulltest`: ";
	fi	
	echo "*Failed to run*"
	echo "  command was ${EXEC} ${TST_DIR}/${program}"
	echo "  Output stored in $out";
	echo "  Stderr stored in ${error}";
	exit 1;
fi

rm -f ${error}

# clean the expected file
${GREP} -v "TODO" ${expected} | ${GREP} -v "(ptr)" | $GREP -v "(func ptr)" | $GREP -v "(context ptr)" | ${GREP} -v "Process" | ${GREP} -v "sizeOfSelf" > ${cleanedExpected};

unitname=`${GREP} RegressionTest_Init ${program}.c | ${SED} 's/[ \t"]//g' | ${SED} 's/[(),]/ /g' | cut -f 2 -d ' '`
if test "${unitname}x" = "x"; then
	unitname="Unknown"
fi

# check the outputs
if diff ${cleanedExpected} ${cleanedOut} > ${diffs}; then
	rm -f ${cleanedExpected} ${cleanedOut} ${out} ${diffs};
	if test "${UPDATE_MODE}x" = "x"; then
		echo "Passed"
		if test ! "${REGRESSION_RUN}x" = "x"; then
			if test -x ${BLD_DIR}/bin/regresstorCheckDiff; then
				${BLD_DIR}/bin/regresstorCheckDiff \
					RunID=${RUNID} \
					TestName=`basename ${fulltest}` \
					URL=${REGRESSTOR_URL} \
					Passed=1 \
					OutputLocation=none\
					UnitName=${unitname}
			fi
		fi
	fi
	exit 0;
else
	rm -f ${cleanedExpected} ${cleanedOut};
	if test "${UPDATE_MODE}x" = "onx"; then
		# update the expected value
		echo Patching expected file: $expected
		mv $out $expected
		rm $diffs
	else
		echo "*Failed*";
		echo "  Output stored in $out";
		echo "  Diffs with expected stored in ${diffs}";
		if test ! "${REGRESSION_RUN}x" = "x"; then
			if test -x ${BLD_DIR}/bin/regresstorCheckDiff; then
				${BLD_DIR}/bin/regresstorCheckDiff \
					RunID=${RUNID} \
					TestName=`basename ${fulltest}` \
					URL=${REGRESSTOR_URL} \
					Passed=0 \
					OutputLocation=${diffs} \
					UnitName=${unitname}
			fi
		fi;
	fi
	exit 1;
fi
