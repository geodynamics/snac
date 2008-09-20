#!/bin/sh

. ./systemTesting.sh

testname="`basename $0 .sh`"

if test "${UPDATE_MODE}x" = "x"; then
	printf "$testname: ";
fi

if ! which mpipython.exe 1> /dev/null 2>&1; then
	if test "${UPDATE_MODE}x" = "onx"; then
		printf "$testname: ";
	fi
	echo "Pyre not found... test skipped";
	exit 0;
fi	

if runTest $testname ./basic-pyre.sh; then		
	passed="true"
	
	if ! handleTestResult ${testname}.out ${testname}.stdout.expected; then
		passed="false";
	fi	

	if test "${passed}X" = "trueX"; then
		rm -f ${testname}.out;
	fi
else
	passed="false"
fi	

rm -f *.0 

if test $passed = "false"; then
	exit 1
fi
