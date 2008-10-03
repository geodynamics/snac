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

# gLucifer Stuff
setValueWithDefault GLUCIFER_DIR      '${BLD_DIR}'
setValueWithDefault GLUCIFER_INCDIR   '${GLUCIFER_DIR}/include'
setValueWithDefault GLUCIFER_INCLUDES  '-I${GLUCIFER_INCDIR}'
setValueWithDefault GLUCIFER_LIBDIR   '${GLUCIFER_DIR}/lib'
setValueWithDefault GLUCIFER_LIBFILES '-lglucifer'
setValueWithDefault GLUCIFER_LIBS     '-L${GLUCIFER_LIBDIR} ${GLUCIFER_LIBFILES}'

case $CC_TYPE in
        gnu)
                case $SYSTEM in
                        Linux|CYGWIN)
                                setValueWithDefault GLUCIFER_RPATH '-Xlinker -rpath -Xlinker ${GLUCIFER_LIBDIR}';;
                        Darwin)
                                # Darwin doesn't support RPATH... use INSTALL NAME instead
                                setValueWithDefault GLUCIFER_RPATH '';;
                        OSF1)
                                setValueWithDefault GLUCIFER_RPATH '-Wl,-rpath,${GLUCIFER_LIBDIR}';;
                        *)
                                echo "Warning: GLUCIFER_RPATH for C compiler \"${CC_TYPE}\" unknown for system \"${SYSTEM}\"";;
                esac ;;
        osf)
                # NOTE: This may not work... I think only ONE rpath arguement is accepted/read.
                setValueWithDefault GLUCIFER_RPATH '-rpath ${GLUCIFER_LIBDIR}';;
        intel)
                setValueWithDefault GLUCIFER_RPATH '-Xlinker -rpath -Xlinker ${GLUCIFER_LIBDIR}';;
        sparc)
                setValueWithDefault GLUCIFER_RPATH '-R ${GLUCIFER_LIBDIR}';;
        ibmxl)
                setValueWithDefault GLUCIFER_RPATH '-R ${GLUCIFER_LIBDIR}';;
        *)
                echo "Warning: GLUCIFER_RPATH for C compiler \"${CC_TYPE}\" unknown";;
esac

# Only warn if we're not building glucifer as part of this build
if test "${BUILDING_GLUCIFER}" != "1"; then
	warnIfNotValidFile ${GLUCIFER_DIR} glucifer glucifer GLUCIFER_DIR
	warnIfNotValidFile ${GLUCIFER_INCDIR}/glucifer/glucifer.h glucifer GLUCIFER_INCDIR "GLUCIFER_DIR"
	warnIfNotValidFile ${GLUCIFER_LIBDIR}/libglucifer.a glucifer GLUCIFER_LIBDIR "GLUCIFER_DIR" 
	if ! DoCompileConftest ./VMake/Config/glucifer; then
		exitWithErrorIfNotOptionalWithMessage glucifer ${GLUCIFER_DIR}
	fi
else
	echo "    (glucifer marked as being built, skipping tests)"
fi

setValueWithDefault HAVE_GLUCIFER 1

