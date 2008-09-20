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

# Check if we're allowed to use LIBAVCODEC
if test "${NOLIBAVCODEC}" = "1" ; then
	return 0
fi

case ${SYSTEM} in
	Darwin)
		setValueWithDefault LIBAVCODEC_DIR       '/sw';;
	*)
		setValueWithDefault LIBAVCODEC_DIR       '/usr';;
esac

PKG_CONFIG=`${WHICH} pkg-config 2> /dev/null`
if whichFailed "${PKG_CONFIG}"; then
	NO_PKG_CONFIG=1
fi
if test -r ${LIBAVCODEC_DIR}/lib/pkgconfig/libavcodec.pc; then
	export PKG_CONFIG_PATH="${PKG_CONFIG_PATH}:${LIBAVCODEC_DIR}/lib/pkgconfig"
	if `${PKG_CONFIG} --exists libavcodec`; then
		LIBAVCODEC_CFLAGS_DEFAULT=`$PKG_CONFIG --cflags libavcodec`
		setValueWithDefault LIBAVCODEC_CFLAGS "${LIBAVCODEC_CFLAGS_DEFAULT}"

		LIBAVCODEC_INCLUDES_DEFAULT=`$PKG_CONFIG --cflags libavcodec`
		setValueWithDefault LIBAVCODEC_INCLUDES "${LIBAVCODEC_CFLAGS_DEFAULT}"

		LIBAVCODEC_LIBS_DEFAULT=`$PKG_CONFIG --libs libavcodec`
		setValueWithDefault LIBAVCODEC_LIBS "${LIBAVCODEC_LIBS_DEFAULT}"

		LIBAVCODEC_LIBS_DIR=`$PKG_CONFIG --libs-only-L libavcodec | ${SED} 's/\-L//g'`
		LIBAVCODEC_RPATH=""
		for x in $LIBAVCODEC_LIBS_DIR
		do
			LIBAVCODEC_RPATH="${LIBAVCODEC_RPATH} ${RPATH_FLAG}${x}"
		done
		setValueWithDefault LIBAVCODEC_RPATH "${LIBAVCODEC_RPATH}"
		
		setValueWithDefault HAVE_LIBAVCODEC       '1'
	else
		echo "Warning: pkg-config does not know of libavcodec. Set LIBAVCODEC_DIR"
	fi
else
	setValueWithDefault LIBAVCODEC_INCDIR    '${LIBAVCODEC_DIR}/include/'
	if test -r "${LIBAVCODEC_INCDIR}/ffmpeg/avcodec.h" ; then
		setValueWithDefault LIBAVCODEC_INCLUDES   '-I${LIBAVCODEC_INCDIR} -DHAVE_LIBAVCODEC'
		setValueWithDefault LIBAVCODEC_LIBDIR     '${LIBAVCODEC_DIR}/lib'
		setValueWithDefault LIBAVCODEC_LIBFILES   '-lavcodec'
		setValueWithDefault LIBAVCODEC_LIBS       '-L${LIBAVCODEC_LIBDIR} ${LIBAVCODEC_LIBFILES}'
		if test "${RPATH_FLAG}x" != "x" ; then
			setValueWithDefault LIBAVCODEC_RPATH      '${RPATH_FLAG}${LIBAVCODEC_LIBDIR}'
		fi
		setValueWithDefault HAVE_LIBAVCODEC       '1'
	fi
fi
