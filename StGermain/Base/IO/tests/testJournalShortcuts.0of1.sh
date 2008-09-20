#!/bin/sh

TEST_SCRIPT=./VMake/executableTester.sh
until test -r ${TEST_SCRIPT} ; do
        TEST_SCRIPT=../${TEST_SCRIPT}
done
. ${TEST_SCRIPT}

OUTPUT_DIR=./output
export OUTPUT_DIR

runAndHandleSystemTest "testJournalShortcuts " "$0" "$@"
