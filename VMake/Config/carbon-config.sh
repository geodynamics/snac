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

# Carbon - Mac Windowing API

if test "${SYSTEM}" = "Darwin" ; then
	setValueWithDefault CARBON_DIR '/System/Library/Frameworks/Carbon.framework'
	setValueWithDefault CARBON_INCDIR '${CARBON_DIR}/Headers/'
	
	if test -r "${CARBON_INCDIR}/Carbon.h" ;  then
		setValueWithDefault CARBON_INCLUDES '-I${CARBON_INCDIR} -DHAVE_CARBON'
		setValueWithDefault HAVE_CARBON '1'
		
		setValueWithDefault CARBON_LIBS '-framework Carbon -framework AGL'
	fi
fi	
