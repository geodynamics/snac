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

# Check if we're allowed to use FREETYPE
if test "${NOFREETYPE}" = "1" ; then
	return 0
fi

case ${SYSTEM} in
        Darwin)
		setValueWithDefault FREETYPE_DIR       '/sw/';;
	*)
		setValueWithDefault FREETYPE_DIR       '/usr/';;
esac

setValueWithDefault FREETYPE_INCDIR    '${FREETYPE_DIR}/include/freetype'
if test -r "${FREETYPE_INCDIR}/freetype.h" ; then
	setValueWithDefault FREETYPE_INCLUDES   '-I${FREETYPE_INCDIR} -I${FREETYPE_INCDIR}/config -DHAVE_FREETYPE'
	setValueWithDefault FREETYPE_LIBDIR     '${FREETYPE_DIR}/lib'
	setValueWithDefault FREETYPE_LIBFILES   '-lttf'
	setValueWithDefault FREETYPE_LIBS       '-L${FREETYPE_LIBDIR} ${FREETYPE_LIBFILES}'
	if test "${RPATH_FLAG}x" != "x" ; then
		setValueWithDefault FREETYPE_RPATH      '${RPATH_FLAG}${FREETYPE_LIBDIR}'
	fi
	setValueWithDefault HAVE_FREETYPE       '1'
fi

