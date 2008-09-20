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

# Check if we're allowed to use VTK
if test "${NOVTK}" = "1" ; then
	return 0
fi

# VTK Stuff
setValueWithDefault VTK_DIR       '/usr/local/'
setValueWithDefault VTK_INCDIR    '${VTK_DIR}/include/vtk'

if test -r "${VTK_INCDIR}/vtkRenderer.h" ; then
	setValueWithDefault HAVE_VTK  '1'
	setValueWithDefault VTK_INCLUDES   '-I${VTK_INCDIR} -DHAVE_VTK'

	setValueWithDefault VTK_LIBDIR    '${VTK_DIR}/lib/vtk/'
	setValueWithDefault VTK_LIBS      '-L${VTK_LIBDIR} ${VTK_LIBFILES}'

	# Check vtk libs (not all may have been compiled by the user.)
	appendVariableIfReadable VTK_LIBFILES "${VTK_LIBDIR}/libvtkParallel.so"   -lvtkParallel
	appendVariableIfReadable VTK_LIBFILES "${VTK_LIBDIR}/libvtkPatented.so"   -lvtkPatented
	appendVariableIfReadable VTK_LIBFILES "${VTK_LIBDIR}/libvtkHybrid.so"     -lvtkHybrid
	appendVariableIfReadable VTK_LIBFILES "${VTK_LIBDIR}/libvtkRendering.so"  -lvtkRendering
	appendVariableIfReadable VTK_LIBFILES "${VTK_LIBDIR}/libvtkIO.so"         -lvtkIO
	appendVariableIfReadable VTK_LIBFILES "${VTK_LIBDIR}/libvtkImaging.so"    -lvtkImaging
	appendVariableIfReadable VTK_LIBFILES "${VTK_LIBDIR}/libvtkGraphics.so"   -lvtkGraphics
	appendVariableIfReadable VTK_LIBFILES "${VTK_LIBDIR}/libvtkFiltering.so"  -lvtkFiltering
	appendVariableIfReadable VTK_LIBFILES "${VTK_LIBDIR}/libvtkCommon.so"     -lvtkCommon
	appendVariableIfReadable VTK_LIBFILES "${VTK_LIBDIR}/libvtkfreetype.so"   -lvtkfreetype
	appendVariableIfReadable VTK_LIBFILES "${VTK_LIBDIR}/libvtkexpat.so"      -lvtkexpat
	appendVariableIfReadable VTK_LIBFILES "${VTK_LIBDIR}/libvtktiff.so"       -lvtktiff
	appendVariableIfReadable VTK_LIBFILES "${VTK_LIBDIR}/libvtkpng.so"        -lvtkpng
	appendVariableIfReadable VTK_LIBFILES "${VTK_LIBDIR}/libvtkjpeg.so"       -lvtkjpeg
	appendVariableIfReadable VTK_LIBFILES "${VTK_LIBDIR}/libvtkzlib.so"       -lvtkzlib
	if test "${RPATH_FLAG}x" != "x" ; then
		setValueWithDefault VTK_RPATH      '${RPATH_FLAG}${VTK_LIBDIR}'
	fi
fi




