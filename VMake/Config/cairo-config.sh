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
CAIRO_CONFIG_DEFAULT=`${WHICH} pkg-config 2> /dev/null`
if whichFailed "${CAIRO_CONFIG_DEFAULT}"; then
	CAIRO_CONFIG_DEFAULT="/usr/bin/pkg-config"
fi
setValueWithDefault CAIRO_CONFIG "${CAIRO_CONFIG_DEFAULT}"
assertValidExecutable "${CAIRO_CONFIG}" pkg-config

#CAIRO_DIR_DEFAULT=`$CAIRO_CONFIG --prefix cairo`
# cairo doesn't like --prefix
CAIRO_DIR_DEFAULT=`$CAIRO_CONFIG cairo`
setValueWithDefault CAIRO_DIR "${CAIRO_DIR_DEFAULT}"

CAIRO_CFLAGS_DEFAULT=`$CAIRO_CONFIG --cflags cairo`
setValueWithDefault CAIRO_CFLAGS "${CAIRO_CFLAGS_DEFAULT}"

CAIRO_INCLUDES_DEFAULT=`$CAIRO_CONFIG --cflags cairo`
setValueWithDefault CAIRO_INCLUDES "${CAIRO_CFLAGS_DEFAULT}"

CAIRO_LIBS_DEFAULT=`$CAIRO_CONFIG --libs cairo`
setValueWithDefault CAIRO_LIBS "${CAIRO_LIBS_DEFAULT}"
