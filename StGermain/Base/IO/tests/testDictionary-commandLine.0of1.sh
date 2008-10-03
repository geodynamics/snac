#!/bin/sh

TEST_SCRIPT=./VMake/executableTester.sh
until test -r ${TEST_SCRIPT} ; do
        TEST_SCRIPT=../${TEST_SCRIPT}
done
. ${TEST_SCRIPT}

runAndHandleSystemTest "testDictionary-commandLine -param=hey --option --output-dir= --Ra=1.0e4.0 --foo.bar=5 --vpac.csd.steve=cool --foo.bot=7 --sports[]=hockey --sports[]=chess --sports[1]=tennis --sles[].name=pressure --sles[].name=temperature --sles[0].solver=mg --sles[1].solver=direct --some.crazy[].shit=here" "$0" "$@"
