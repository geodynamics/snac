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

# Check if we're allowed to use GTK2
if test "${NOGTK2}" = "1" ; then
	return 0
fi

case ${SYSTEM} in
        Darwin)
		setValueWithDefault GTK2_DIR       '/sw/';;
	*)
		setValueWithDefault GTK2_DIR       '/usr/';;
esac

setValueWithDefault GTK2_INCDIR    '${GTK2_DIR}/include/gtk-2.0'
if test -r "${GTK2_INCDIR}/gtk/gtk.h" ; then
	setValueWithDefault GTK2_INCLUDES   '-I${GTK2_INCDIR} -I${GTK2_DIR}/lib/gtk-2.0/include -DHAVE_GTK2'
	setValueWithDefault GTK2_LIBDIR     '${GTK2_DIR}/lib'
	setValueWithDefault GTK2_LIBFILES   '-lgtk-x11-2.0 -lgdk-x11-2.0 -lgdk_pixbuf-2.0'
	setValueWithDefault GTK2_LIBS       '-L${GTK2_LIBDIR} ${GTK2_LIBFILES}'
	if test "${RPATH_FLAG}x" != "x" ; then
		setValueWithDefault GTK2_RPATH      '${RPATH_FLAG}${GTK2_LIBDIR}'
	fi
	setValueWithDefault HAVE_GTK2       '1'
fi

