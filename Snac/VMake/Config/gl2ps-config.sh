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

case ${SYSTEM} in
	Darwin)
		setValueWithDefault GL2PS_DIR '/sw';;
	*)
		setValueWithDefault GL2PS_DIR '/usr';;
esac

setValueWithDefault GL2PS_INCDIR '${GL2PS_DIR}'
if test -r "${GL2PS_INCDIR}/gl2ps.h" -a "${NOGL2PS}" != "1" ; then
	setValueWithDefault GL2PS_INCLUDES '-I${GL2PS_INCDIR} -DHAVE_GL2PS'
	setValueWithDefault GL2PS_LIBDIR '${GL2PS_DIR}'
	setValueWithDefault GL2PS_LIBS '-L${GL2PS_LIBDIR} -lgl2ps'

	setValueWithDefault HAVE_GL2PS '1'
fi