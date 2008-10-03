#!/bin/sh

TEST_SCRIPT=./VMake/executableTester.sh
until test -r ${TEST_SCRIPT} ; do
        TEST_SCRIPT=../${TEST_SCRIPT}
done
. ${TEST_SCRIPT}

runAndHandleSystemTest "testHexaMD --allowUnusedCPUs=False --allowUnbalancing=True --meshSizeI=11 --meshSizeJ=2 --meshSizeK=2" "$0" "$@"
