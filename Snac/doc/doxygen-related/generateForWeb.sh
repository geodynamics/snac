#!/bin/sh

export PROJ_ROOT=`pwd | sed 's/doc$//'`
export REVLINE=`svn info | grep Revision`

# Find the "INPUT" field and add the root project dir to it
cat "web.doxyconf.in" | sed 's:\(^[ \t]*INPUT[ \t]*=\):\1 '"${PROJ_ROOT}:"  > "web.doxyconf.in.1"

# Find the "STRIP_FROM_PATH" field and add the root project dir to it
cat "web.doxyconf.in.1" | sed 's:\(^[ \t]*STRIP_FROM_PATH[ \t]*=\):\1 '"${PROJ_ROOT}:"  > "web.doxyconf.in.2"

# Find the "PROJECT_NUMBER" field and add the project revision to it
cat "web.doxyconf.in.2" | sed 's+\(^[ \t]*PROJECT_NUMBER[ \t]*=\)+\1 '"${REVLINE}+"  > "web.doxyconf.in.3"

# Run doxygen
doxygen "web.doxyconf.in.3"


# Clean up
rm -f "web.doxyconf.in.1" "web.doxyconf.in.2" "web.doxyconf.in.3"

