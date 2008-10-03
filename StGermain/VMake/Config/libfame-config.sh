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

# Check if we're allowed to use LIBFAME
if test "${NOLIBFAME}" = "1" ; then
	return 0
fi

case ${SYSTEM} in
        Darwin)
		setValueWithDefault LIBFAME_DIR       '/sw/';;
	*)
		setValueWithDefault LIBFAME_DIR       '/usr/';;
esac

setValueWithDefault LIBFAME_INCDIR    '${LIBFAME_DIR}/include'
warnIfNotValidFile "${LIBFAME_INCDIR}/fame.h" LIBFAME LIBFAME_INCDIR

if test -r "${LIBFAME_INCDIR}/fame.h" ; then
	setValueWithDefault LIBFAME_INCLUDES   '-I${LIBFAME_INCDIR} -DHAVE_LIBFAME'
	setValueWithDefault LIBFAME_LIBDIR     '${LIBFAME_DIR}/lib'
	setValueWithDefault LIBFAME_LIBFILES   '-lfame'
	setValueWithDefault LIBFAME_LIBS       '-L${LIBFAME_LIBDIR} ${LIBFAME_LIBFILES}'
	if test "${RPATH_FLAG}x" != "x" ; then
		setValueWithDefault LIBFAME_RPATH      '${RPATH_FLAG}${LIBFAME_LIBDIR}'
	fi
	setValueWithDefault HAVE_LIBFAME       '1'
fi

