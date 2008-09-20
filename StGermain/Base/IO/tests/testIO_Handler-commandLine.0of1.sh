#!/bin/sh

TEST_SCRIPT=./VMake/executableTester.sh
until test -r ${TEST_SCRIPT} ; do
        TEST_SCRIPT=../${TEST_SCRIPT}
done
. ${TEST_SCRIPT}

runAndHandleSystemTest "testIO_Handler-commandLine data/duplicate.xml data/journal.xml fake command.line Args" "$0" "$@"
