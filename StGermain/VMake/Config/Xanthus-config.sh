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

setValueWithDefault XANTHUS_DIR '${BLD_DIR}'
setValueWithDefault XANTHUS_INCDIR '${XANTHUS_DIR}/include'
setValueWithDefault XANTHUS_INCLUDES '-I${XANTHUS_INCDIR}'
setValueWithDefault XANTHUS_LIBDIR '${XANTHUS_DIR}/lib'
setValueWithDefault XANTHUS_LIBS '-L${XANTHUS_LIBDIR} -lXanthus'

case $CC_TYPE in
	gnu)
		case $SYSTEM in
			Linux|CYGWIN)
				setValueWithDefault XANTHUS_RPATH '-Xlinker -rpath -Xlinker ${XANTHUS_LIBDIR}';;
			Darwin)
				# Darwin doesn't support RPATH... use INSTALL NAME instead
				setValueWithDefault XANTHUS_RPATH '';;
			OSF1)
				setValueWithDefault XANTHUS_RPATH '-Wl,-rpath,${XANTHUS_LIBDIR}';;
			*)
				echo "Warning: XANTHUS_RPATH for C compiler \"${CC_TYPE}\" unknown for system \"${SYSTEM}\"";;
		esac ;;
	osf)
		# NOTE: This may not work... I think only ONE rpath arguement is accepted/read.
		setValueWithDefault XANTHUS_RPATH '-rpath ${XANTHUS_LIBDIR}';;
	intel)
		setValueWithDefault XANTHUS_RPATH '-Xlinker -rpath -Xlinker ${XANTHUS_LIBDIR}';;
	sparc)
		setValueWithDefault XANTHUS_RPATH '-R ${XANTHUS_LIBDIR}';;
	ibmxl)
		setValueWithDefault XANTHUS_RPATH '-R ${XANTHUS_LIBDIR}';;
	*)
		echo "Warning: XANTHUS_RPATH for C compiler \"${CC_TYPE}\" unknown";;
esac

# Only warn if we're not building Xanthus as part of this build
if test "${BUILDING_XANTHUS}" != "1"; then
	warnIfNotValidFile ${XANTHUS_DIR} Xanthus Xanthus XANTHUS_DIR
	warnIfNotValidFile ${XANTHUS_INCDIR}/Xanthus/Xanthus.h Xanthus XANTHUS_INCDIR "XANTHUS_DIR"
	warnIfNotValidFile ${XANTHUS_LIBDIR}/libXanthus.a Xanthus XANTHUS_LIBDIR "XANTHUS_DIR" 
	if ! DoCompileConftest ./VMake/Config/Xanthus; then
		exitWithErrorIfNotOptionalWithMessage Xanthus ${XANTHUS_DIR}
	fi
else
	echo "    (Xanthus marked as being built, skipping tests)"
fi

setValueWithDefault HAVE_XANTHUS 1

