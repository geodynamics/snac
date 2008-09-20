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

# X11 Stuff

# Check if we're allowed to use X11
if test "${NOX11}" = "1" ; then
	return 0
fi

setValueWithDefault X11_DIR '/usr/X11R6/'
setValueWithDefault X11_INCDIR '${X11_DIR}/include/'

warnIfNotValidFile "${X11_INCDIR}/X11/Xlib.h" X11 X11_INCDIR X11_DIR
if test -r "${X11_INCDIR}/X11/Xlib.h" ; then

	setValueWithDefault X11_LIBDIR '${X11_DIR}/lib/'
	setValueWithDefault X11_LIBS '-L${X11_LIBDIR} -lX11 -lXmu'

	case ${SYSTEM} in
		Linux)
			setValueWithDefault X11_INCLUDES '-I${X11_INCDIR}';;
		Darwin)
			setValueWithDefault OSSERVICES_INCDIR /System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/OSServices.framework/Headers
			setValueWithDefault X11_INCLUDES '-I${X11_INCDIR} -I${OSSERVICES_INCDIR}';;
	esac

	if DoCompileConftest ./VMake/Config/X11; then
		echo "X11_INCLUDES+= -DHAVE_X11" >> Makefile.system
		setValue HAVE_X11 '1'
	else
		echo "    (X11 interactive rendering will be unavailable)"
	fi
fi
