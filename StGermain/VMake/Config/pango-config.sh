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

# Check if we're allowed to use PANGO
if test "${NOPANGO}" = "1" ; then
	return 0
fi

case ${SYSTEM} in
        Darwin)
		setValueWithDefault PANGO_DIR       '/sw/';;
	*)
		setValueWithDefault PANGO_DIR       '/usr/';;
esac

setValueWithDefault PANGO_INCDIR    '${PANGO_DIR}/include/pango-1.0'
if test -r "${PANGO_INCDIR}/pango/pango.h" ; then
	setValueWithDefault PANGO_INCLUDES   '-I${PANGO_INCDIR} -DHAVE_PANGO'
	setValueWithDefault PANGO_LIBDIR     '${PANGO_DIR}/lib'
	setValueWithDefault PANGO_LIBFILES   '-lpangoxft-1.0 -lpangox-1.0 -lpango-1.0'
	setValueWithDefault PANGO_LIBS       '-L${PANGO_LIBDIR} ${PANGO_LIBFILES}'
	if test "${RPATH_FLAG}x" != "x" ; then
		setValueWithDefault PANGO_RPATH      '${RPATH_FLAG}${PANGO_LIBDIR}'
	fi
	setValueWithDefault HAVE_PANGO       '1'
fi

