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

setValueWithDefault UNDERWORLD_DIR '${BLD_DIR}'
setValueWithDefault UNDERWORLD_INCDIR '${UNDERWORLD_DIR}/include'
setValueWithDefault UNDERWORLD_INCLUDES '-I${UNDERWORLD_INCDIR}'
setValueWithDefault UNDERWORLD_LIBDIR '${UNDERWORLD_DIR}/lib'
setValueWithDefault UNDERWORLD_LIBS '-L${UNDERWORLD_LIBDIR} -lUnderworld'

case $CC_TYPE in
	gnu)
		case $SYSTEM in
			Linux|CYGWIN)
				setValueWithDefault UNDERWORLD_RPATH '-Xlinker -rpath -Xlinker ${UNDERWORLD_LIBDIR}';;
			Darwin)
				# Darwin doesn't support RPATH... use INSTALL NAME instead
				setValueWithDefault UNDERWORLD_RPATH '';;
			OSF1)
				setValueWithDefault UNDERWORLD_RPATH '-Wl,-rpath,${UNDERWORLD_LIBDIR}';;
			*)
				echo "Warning: UNDERWORLD_RPATH for C compiler \"${CC_TYPE}\" unknown for system \"${SYSTEM}\"";;
		esac ;;
	osf)
		# NOTE: This may not work... I think only ONE rpath arguement is accepted/read.
		setValueWithDefault UNDERWORLD_RPATH '-rpath ${UNDERWORLD_LIBDIR}';;
	intel)
		setValueWithDefault UNDERWORLD_RPATH '-Xlinker -rpath -Xlinker ${UNDERWORLD_LIBDIR}';;
	sparc)
		setValueWithDefault UNDERWORLD_RPATH '-R ${UNDERWORLD_LIBDIR}';;
	ibmxl)
		setValueWithDefault UNDERWORLD_RPATH '-R ${UNDERWORLD_LIBDIR}';;
	*)
		echo "Warning: UNDERWORLD_RPATH for C compiler \"${CC_TYPE}\" unknown";;
esac

# Only warn if we're not building Underworld as part of this build
if test "${BUILDING_UNDERWORLD}" != "1"; then
	warnIfNotValidFile ${UNDERWORLD_DIR} Underworld Underworld UNDERWORLD_DIR
	warnIfNotValidFile ${UNDERWORLD_INCDIR}/Underworld/Underworld.h Underworld UNDERWORLD_INCDIR "UNDERWORLD_DIR"
	warnIfNotValidFile ${UNDERWORLD_LIBDIR}/libUnderworld.a Underworld UNDERWORLD_LIBDIR "UNDERWORLD_DIR" 
	if ! DoCompileConftest ./VMake/Config/Underworld; then
		exitWithErrorIfNotOptionalWithMessage Underworld ${UNDERWORLD_DIR}
	fi
else
	echo "    (Underworld marked as being built, skipping tests)"
fi

setValueWithDefault HAVE_UNDERWORLD 1

