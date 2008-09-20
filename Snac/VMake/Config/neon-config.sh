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

# Obtain libxml information
NEON_CONFIG_DEFAULT=`${WHICH} neon-config 2> /dev/null`
if whichFailed "${NEON_CONFIG_DEFAULT}"; then
	NEON_CONFIG_DEFAULT="/usr/bin/neon-config"
fi
if test -x ${NEON_CONFIG_DEFAULT}; then
	setValueWithDefault NEON_CONFIG "${NEON_CONFIG_DEFAULT}"
	assertValidExecutable "${NEON_CONFIG}" neon-config

	NEON_DIR_DEFAULT=`$NEON_CONFIG --prefix`
	setValueWithDefault NEON_DIR "${NEON_DIR_DEFAULT}"

	NEON_CFLAGS_DEFAULT=`$NEON_CONFIG --cflags`
	setValueWithDefault NEON_CFLAGS "${NEON_CFLAGS_DEFAULT}"

	NEON_INCLUDES_DEFAULT=`$NEON_CONFIG --cflags`
	setValueWithDefault NEON_INCLUDES "${NEON_CFLAGS_DEFAULT}"

	NEON_LIBS_DEFAULT=`$NEON_CONFIG --libs`
	setValueWithDefault NEON_LIBS "${NEON_LIBS_DEFAULT}"

	setValue HAVE_NEON 1
fi
