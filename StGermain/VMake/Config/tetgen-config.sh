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

# Obtain tetgen library information

parsePackageConfigOptions $@

# Check if we're allowed to use LIBAVCODEC
if test "${NOTETGEN}" = "1" ; then
	return 0
fi

TETGEN_URL=http://tetgen.berlios.de/

setValueWithDefault TETGEN_DIR '/usr'
setValueWithDefault TETGEN_INCDIR '${TETGEN_DIR}'
setValueWithDefault TETGEN_LIBDIR '${TETGEN_DIR}/lib'

warnIfNotValidFile "${TETGEN_INCDIR}/tetgen.h" "TetGen ${TETGEN_URL}" TETGEN_INCDIR TETGEN_DIR
warnIfNotValidFile "${TETGEN_INCDIR}/libtet.a" "TetGen ${TETGEN_URL} (remember to do 'make libtet')" TETGEN_LIBDIR TETGEN_DIR

setValueWithDefault TETGEN_INCLUDES '-I${TETGEN_INCDIR}'

setValueWithDefault TETGEN_LIBS '-L${TETGEN_LIBDIR} -ltet'
