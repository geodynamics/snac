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

#Check what image libraries are avaiable
case ${SYSTEM} in
	Linux)
		setValueWithDefault JPEG_DIR '/usr'
		setValueWithDefault PNG_DIR '/usr'
		setValueWithDefault TIFF_DIR '/usr';;
	Darwin)
		setValueWithDefault JPEG_DIR '/sw'
		setValueWithDefault PNG_DIR '/sw'
		setValueWithDefault TIFF_DIR '/sw';;
esac

setValueWithDefault PNG_INCDIR '${PNG_DIR}/include'
if test -r "${PNG_INCDIR}/png.h" -a "${NOPNG}" != "1" ; then
	setValueWithDefault PNG_INCLUDES '-I${PNG_INCDIR} -DHAVE_PNG'
	setValueWithDefault PNG_LIBDIR '${PNG_DIR}/lib'
	setValueWithDefault PNG_LIBS '-L${PNG_LIBDIR} -lpng'

	setValueWithDefault HAVE_PNG '1'
fi

setValueWithDefault JPEG_INCDIR '${JPEG_DIR}/include'
if test -r "${JPEG_INCDIR}/jpeglib.h" -a "${NOJPEG}" != "1" ; then
	setValueWithDefault JPEG_INCLUDES '-I${JPEG_INCDIR} -DHAVE_JPEG'
	setValueWithDefault JPEG_LIBDIR '${JPEG_DIR}/lib'
	setValueWithDefault JPEG_LIBS '-L${JPEG_LIBDIR} -ljpeg'

	setValueWithDefault HAVE_JPEG '1'
fi

setValueWithDefault TIFF_INCDIR '${TIFF_DIR}/include'
if test -r "${TIFF_INCDIR}/tiff.h" -a "${NOTIFF}" != "1" ; then
	setValueWithDefault TIFF_INCLUDES '-I${TIFF_INCDIR} -DHAVE_TIFF'
	setValueWithDefault TIFF_LIBDIR '${TIFF_DIR}/lib'
	setValueWithDefault TIFF_LIBS '-L${TIFF_LIBDIR} -ltiff'

	setValueWithDefault HAVE_TIFF '1'
fi
