#!/bin/bash

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

#StGermain stuff
setValueWithDefault STGERMAIN_DIR '${BLD_DIR}'
setValueWithDefault STGERMAIN_INCDIR '${STGERMAIN_DIR}/include'
setValueWithDefault STGERMAIN_INCLUDES '-I${STGERMAIN_INCDIR}'
setValueWithDefault STGERMAIN_LIBDIR '${STGERMAIN_DIR}/lib'
setValueWithDefault STGERMAIN_LIBS '-L${STGERMAIN_LIBDIR} -lStGermain'

case $CC_TYPE in
	gnu)
		case $SYSTEM in
			Linux|CYGWIN)
				setValueWithDefault STGERMAIN_RPATH '-Xlinker -rpath -Xlinker ${STGERMAIN_LIBDIR}';;
			Darwin)
				# Darwin doesn't support RPATH... use INSTALL NAME instead
				setValueWithDefault STGERMAIN_RPATH '';;
			OSF1)
				setValueWithDefault STGERMAIN_RPATH '-Wl,-rpath,${STGERMAIN_LIBDIR}';;
			SunOS)
				setValueWithDefault STGERMAIN_RPATH '';; # Alan not convinced it does it				
			*)
				echo "Warning: STGERMAIN_RPATH for C compiler \"${CC_TYPE}\" unknown for system \"${SYSTEM}\"";;
		esac ;;
	osf)
		# NOTE: This may not work... I think only ONE rpath arguement is accepted/read.
		setValueWithDefault STGERMAIN_RPATH '-rpath ${STGERMAIN_LIBDIR}';;
	intel)
		setValueWithDefault STGERMAIN_RPATH '-Xlinker -rpath -Xlinker ${STGERMAIN_LIBDIR}';;
	sparc)
		setValueWithDefault STGERMAIN_RPATH '-R ${STGERMAIN_LIBDIR}';;		
	ibmxl)
		setValueWithDefault STGERMAIN_RPATH '-R ${STGERMAIN_LIBDIR}';;		
	pgi)
		setValueWithDefault STGERMAIN_RPATH '-Wl,-rpath,${STGERMAIN_LIBDIR}';;
	*)
		echo "Warning: STGERMAIN_RPATH for C compiler \"${CC_TYPE}\" unknown";;
esac

# Only warn if we're not building StGermain as part of this build
if test "${BUILDING_STGERMAIN}" != "1"; then
	warnIfNotValidFile ${STGERMAIN_DIR} StGermain StGermain STGERMAIN_DIR
	warnIfNotValidFile ${STGERMAIN_INCDIR}/StGermain/StGermain.h StGermain STGERMAIN_INCDIR "STGERMAIN_DIR"
	warnIfNotValidFile ${STGERMAIN_LIBDIR}/libStGermain.a StGermain STGERMAIN_LIBDIR "STGERMAIN_DIR" 
	if ! DoCompileConftest ./VMake/Config/StGermain; then
		exitWithErrorIfNotOptionalWithMessage StGermain ${STGERMAIN_DIR}
	fi
else
	echo "    (StGermain marked as being built, skipping tests)"
fi

setValueWithDefault HAVE_STGERMAIN 1
