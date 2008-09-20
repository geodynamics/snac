#!/bin/sh

. './systemTesting.sh'

testname="`basename $0 .sh`"

if test "${UPDATE_MODE}x" = "x"; then
	printf "$testname: ";
fi

MPIPYTHON=""
if which mpipython.exe 1> /dev/null 2>&1; then
	MPIPYTHON="mpipython.exe"
elif which mpipython 1> /dev/null 2>&1; then
	MPIPYTHON="mpipython"
elif test -x "$BIN_DIR/mpipython"; then
	MPIPYTHON="mpipython"
	PATH="$BIN_DIR:$PATH"
	PYTHONPATH=
else
	echo "mpipython.exe and mpipython not found... test skipped";
	exit 0;
fi	

if runTest $testname "$MPIPYTHON basic-python.py"; then		
	passed="true"
	
	if ! handleTestResult timeStep.0 test-basic.timeStep.0.expected; then
		passed="false";
	fi	

	if test "${passed}X" = "trueX"; then
		rm $testname.out;
	fi
else
	passed="false"
fi	

rm -f *.0

if test $passed = "false"; then
	exit 1
fi
