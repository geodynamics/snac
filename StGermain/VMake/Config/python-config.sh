#!/bin/sh

##  Copyright (C) 2005 Victorian Partnership for Advanced Computing (VPAC) Ltd
##  110 Victoria Street, Melbourne, 3053, Australia.
##
##  This library is free software; you can redistribute it and/or
##  modify it under the terms of the GNU Lesser General Public
##  License as published by the Free Software Foundation; either
##  version 2.1 of the License, or (at your option) any later version.
##
##  This library is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
##  Lesser General Public License for more details.
##
##  You should have received a copy of the GNU Lesser General Public
##  License along with this library; if not, write to the Free Software
##  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

parsePackageConfigOptions $@

# Check if we're allowed to use PYTHON
if test "${NOPYTHON}" = "1" ; then
	return 0
fi

# Obtain Python information
case ${SYSTEM} in
	Linux|CYGWIN|OSF1|SunOS)
		setValueWithDefault PYTHON_DIR '/usr/';;
	Darwin)
		# the default python does not include the api
		setValueWithDefault PYTHON_DIR '/sw/';;
	*)
		echo "Warning: PYTHON_DIR for system \"${SYSTEM}\" unknown. Set the environment variable if you have Python installed.";;
esac

setValueWithDefault PYTHON_VERSION   `python -V 2>&1 | ${SED} 's/Python //' | cut -f 1,2 -d '.'`
setValueWithDefault PYTHON_INCDIR    '${PYTHON_DIR}/include/python${PYTHON_VERSION}'

eval PYTHON_HEADER=${PYTHON_INCDIR}/Python.h

if test -r "${PYTHON_HEADER}" ; then
	setValueWithDefault PYTHON_INCLUDES  '-I${PYTHON_INCDIR} -DHAVE_PYTHON'
	setValueWithDefault PYTHON_BINDIR '${PYTHON_DIR}/bin'
	setValueWithDefault PYTHON        '${PYTHON_BINDIR}/python'
	warnValidExecutable ${PYTHON} python PYTHON "PYTHON_DIR or PYTHON_BINDIR"

	setValueWithDefault PYTHON_LIB 'python${PYTHON_VERSION}'

	if test -x ${PYTHON_DIR}/lib/lib${PYTHON_LIB}.${SO_EXT}; then
		setValueWithDefault PYTHON_LIBDIR '${PYTHON_DIR}/lib'
		setValueWithDefault PYTHON_MODDIR '${PYTHON_LIBDIR}/python${PYTHON_VERSION}'
		setValue PYTHON_HAVE_SO_LIB '1'
	else
		setValueWithDefault PYTHON_LIBDIR '${PYTHON_DIR}/lib/python${PYTHON_VERSION}'
		setValueWithDefault PYTHON_MODDIR '${PYTHON_DIR}/lib/python${PYTHON_VERSION}'
	fi
	
	setValueWithDefault PYTHON_COMPILEALL '${PYTHON} ${PYTHON_MODDIR}/compileall.py'

	case ${SYSTEM} in
		Linux)
			setValueWithDefault PYTHON_EXTERNAL_LIBS '-lpthread -ldl -lutil';;
		CYGWIN)
			setValueWithDefault PYTHON_EXTERNAL_LIBS '-lpthread -lutil';;
		OSF1)
			setValueWithDefault PYTHON_EXTERNAL_LIBS '-lpthread';;
		Darwin)
			setValueWithDefault PYTHON_EXTERNAL_LIBS '-framework CoreServices';;
		*)
			echo "Warning: PYTHON_EXTERNAL_LIBS for system \"${SYSTEM}\" unknown. Set the PYTHON_EXTERNAL_LIBS environment variable.";;
	esac
	
	setValueWithDefault PYTHON_LIBS       '-L${PYTHON_LIBDIR} -l${PYTHON_LIB} ${PYTHON_EXTERNAL_LIBS}'
	
	case ${SYSTEM} in
		CYGWIN)
			setValueWithDefault PYTHON_STATIC_LIB '${PYTHON_LIBDIR}/config/libpython${PYTHON_VERSION}.dll.a';;
		*)
			setValueWithDefault PYTHON_STATIC_LIB '${PYTHON_LIBDIR}/config/libpython${PYTHON_VERSION}.a';;
	esac

	if ! DoCompileConftest ./VMake/Config/python; then
		exitWithErrorIfNotOptionalWithMessage python ${PYTHON_DIR}
	fi

	if ! DoCompileConftest ./VMake/Config/python-compile; then
		exitWithErrorIfNotOptionalWithMessage python-compile ${PYTHON_COMPILEALL}
	fi

	setValueWithDefault HAVE_PYTHON 1
fi
