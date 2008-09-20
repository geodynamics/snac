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

# Check if we're allowed to use GLIB2
if test "${NOGLIB2}" = "1" ; then
	return 0
fi

case ${SYSTEM} in
        Darwin)
		setValueWithDefault GLIB2_DIR       '/sw/';;
	*)
		setValueWithDefault GLIB2_DIR       '/usr/';;
esac

setValueWithDefault GLIB2_INCDIR    '${GLIB2_DIR}/include/glib-2.0'
if test -r "${GLIB2_INCDIR}/glib.h" ; then
	setValueWithDefault GLIB2_INCLUDES   '-I${GLIB2_INCDIR} -I${GLIB2_DIR}/lib/glib-2.0/include -DHAVE_GLIB2'
	setValueWithDefault GLIB2_LIBDIR     '${GLIB2_DIR}/lib'
	setValueWithDefault GLIB2_LIBFILES   '-lgobject-2.0 -lgmodule-2.0 -lglib-2.0'
	setValueWithDefault GLIB2_LIBS       '-L${GLIB2_LIBDIR} ${GLIB2_LIBFILES}'
	if test "${RPATH_FLAG}x" != "x" ; then
		setValueWithDefault GLIB2_RPATH      '${RPATH_FLAG}${GLIB2_LIBDIR}'
	fi
	setValueWithDefault HAVE_GLIB2       '1'
fi

