#!/bin/sh

. ./systemTesting.sh
nproc=3
rank=2

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

testname="`basename $0 .sh`"

if test "${UPDATE_MODE}x" = "x"; then
	printf "$testname: ";
fi


if ! which Snac 1> /dev/null 2>&1; then
	export PATH="$PATH:../../build/bin"
	if ! which Snac 1> /dev/null 2>&1; then
		echo "Snac could not be found"
		exit 1;
	else 
		progname=`which Snac`
	fi
else
	progname=`which Snac`
fi


if runTest $testname "${EXEC} ${progname} basic-velocity-quickForceCalc.xml"; then
	passed="true"

	if ! handleTestResult vel.${rank} ${testname}.vel.${rank}.expected; then
		passed="false";
	else
		if ! handleTestResult forceCheckSum.${rank} ${testname}.forceCheckSum.${rank}.expected; then
			passed="false";
		fi
	fi

	if $passed = "true"; then
		rm -f $testname.out;
	fi
else
	passed="false"
fi	

rm -f *.0 *.1 *.2

if test $passed = "false"; then
	exit 1
fi
