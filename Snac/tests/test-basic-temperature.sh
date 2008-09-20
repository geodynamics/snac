#!/bin/sh

. ./systemTesting.sh

testname="`basename $0 .sh`"

if test "${UPDATE_MODE}x" = "x"; then
	printf "$testname: ";
fi

if ! which Snac 1> /dev/null 2>&1; then
	export PATH="$PATH:../../build/bin"
	if ! which Snac 1> /dev/null 2>&1; then
		echo "Snac could not be found"
		exit 1;
	fi
fi


if runTest $testname "Snac basic-temp.xml"; then
	passed="true"

	if ! handleTestResult temperature.0 ${testname}.temperature.0.expected; then
		passed="false";
	fi	

	if $passed = "true"; then
		rm -f $testname.out;
		exit 0
	fi
else
	passed="false"
fi	

rm -f *.0 

if test $passed = "false"; then
	exit 1
fi
