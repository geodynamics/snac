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

# Check if we're allowed to use ATK
if test "${NOATK}" = "1" ; then
	return 0
fi

case ${SYSTEM} in
        Darwin)
		setValueWithDefault ATK_DIR       '/sw/';;
	*)
		setValueWithDefault ATK_DIR       '/usr/';;
esac

setValueWithDefault ATK_INCDIR    '${ATK_DIR}/include/atk-1.0'
if test -r "${ATK_INCDIR}/atk/atk.h" ; then
	setValueWithDefault ATK_INCLUDES   '-I${ATK_INCDIR} -DHAVE_ATK'
	setValueWithDefault ATK_LIBDIR     '${ATK_DIR}/lib'
	setValueWithDefault ATK_LIBFILES   '-latk-1.0'
	setValueWithDefault ATK_LIBS       '-L${ATK_LIBDIR} ${ATK_LIBFILES}'
	if test "${RPATH_FLAG}x" != "x" ; then
		setValueWithDefault ATK_RPATH      '${RPATH_FLAG}${ATK_LIBDIR}'
	fi
	setValueWithDefault HAVE_ATK       '1'
fi

