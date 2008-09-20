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

# Snark Stuff 
setValueWithDefault SNARK_DIR '${BLD_DIR}'
setValueWithDefault SNARK_INCDIR '${SNARK_DIR}/include'
setValueWithDefault SNARK_LIBDIR '${SNARK_DIR}/lib'

# Only warn when Snark not build if its different to the common build dir
if test "${SNARK_INCDIR}" != "${INC_DIR}"; then
	warnIfNotValidFile "${SNARK_INCDIR}/Snark/Snark.h" Snark SNARK_INCDIR "SNARK_DIR"
fi
if test "${SNARK_LIBDIR}" != "${LIB_DIR}"; then
	warnIfNotValidFile "${SNARK_LIBDIR}/libSnark.a" Snark SNARK_LIBDIR "SNARK_DIR"
fi

setValueWithDefault SNARK_LIBFILES '-lSnark -lpseudoStG_Framework'
setValueWithDefault SNARK_LIBS '-L${SNARK_LIBDIR} ${SNARK_LIBFILES}'

setValueWithDefault SNARK_INCLUDES '-I${STGERMAIN_INCDIR} -I${SNARK_INCDIR} -I${SNARK_INCDIR}/Snark -I${SNARK_INCDIR}/pseudoStG_Framework'

case $CC_TYPE in
        gnu)
                case $SYSTEM in
                        Linux|CYGWIN)
                                setValueWithDefault SNARK_RPATH '-Xlinker -rpath -Xlinker ${SNARK_LIBDIR}';;
                        Darwin)
                                # Darwin doesn't support RPATH... use INSTALL NAME instead
                                setValueWithDefault SNARK_RPATH '';;
                        OSF1)
                                setValueWithDefault STGErMAIN_RPATH '-Wl,-rpath,${SNARK_LIBDIR}';;
                        SunOS)
                                setValueWithDefault SNARK_RPATH '';; # Alan not convinced it does it                                                                                                                                     
                        *)
                                echo "Warning: SNARK_RPATH for C compiler \"${CC_TYPE}\" unknown for system \"${SYSTEM}\"";;
                esac ;;
        osf)
                # NOTE: This may not work... I think only ONE rpath arguement is accepted/read.
                setValueWithDefault SNARK_RPATH '-rpath ${SNARK_LIBDIR}';;
        intel)
                setValueWithDefault SNARK_RPATH '-Xlinker -rpath -Xlinker ${SNARK_LIBDIR}';;
        sparc)
                setValueWithDefault SNARK_RPATH '-R ${SNARK_LIBDIR}';;
        ibmxl)
                setValueWithDefault SNARK_RPATH '-R ${SNARK_LIBDIR}';;
        *)
                echo "Warning: SNARK_RPATH for C compiler \"${CC_TYPE}\" unknown";;
esac
