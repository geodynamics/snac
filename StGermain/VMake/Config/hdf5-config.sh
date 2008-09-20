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

# Check if we're allowed to use HDF5
if test "${NOHDF5}" = "1" ; then
	return 0
fi

# Obtain math library information
setValueWithDefault HDF5_DIR '/usr'
setValueWithDefault HDF5_INCDIR '${HDF5_DIR}/include'

#warnIfNotValidFile "${HDF5_INCDIR}/hdf5.h" HDF5 HDF5_INCDIR

setValueWithDefault HDF5_INCLUDES '-I${HDF5_INCLUDES}'
setValueWithDefault HDF5_LIBDIR '${HDF5_LIBDIR}/lib'
setValueWithDefault HDF5_LIBFILES '-lhdf5'
setValueWithDefault HDF5_LIBS '-L${HDF5_LIBDIR} ${HDF5_LIBFILES}'

