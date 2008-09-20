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

setValueWithDefault PICELLERATOR_DIR '${BLD_DIR}'
setValueWithDefault PICELLERATOR_INCDIR '${PICELLERATOR_DIR}/include'
setValueWithDefault PICELLERATOR_INCLUDES '-I${PICELLERATOR_INCDIR}'
setValueWithDefault PICELLERATOR_LIBDIR '${PICELLERATOR_DIR}/lib'
setValueWithDefault PICELLERATOR_LIBS '-L${PICELLERATOR_LIBDIR} -lPICellerator'

case $CC_TYPE in
	gnu)
		case $SYSTEM in
			Linux|CYGWIN)
				setValueWithDefault PICELLERATOR_RPATH '-Xlinker -rpath -Xlinker ${PICELLERATOR_LIBDIR}';;
			Darwin)
				# Darwin doesn't support RPATH... use INSTALL NAME instead
				setValueWithDefault PICELLERATOR_RPATH '';;
			OSF1)
				setValueWithDefault PICELLERATOR_RPATH '-Wl,-rpath,${PICELLERATOR_LIBDIR}';;
			*)
				echo "Warning: PICELLERATOR_RPATH for C compiler \"${CC_TYPE}\" unknown for system \"${SYSTEM}\"";;
		esac ;;
	osf)
		# NOTE: This may not work... I think only ONE rpath arguement is accepted/read.
		setValueWithDefault PICELLERATOR_RPATH '-rpath ${PICELLERATOR_LIBDIR}';;
	intel)
		setValueWithDefault PICELLERATOR_RPATH '-Xlinker -rpath -Xlinker ${PICELLERATOR_LIBDIR}';;
	sparc)
		setValueWithDefault PICELLERATOR_RPATH '-R ${PICELLERATOR_LIBDIR}';;
	ibmxl)
		setValueWithDefault PICELLERATOR_RPATH '-R ${PICELLERATOR_LIBDIR}';;
	*)
		echo "Warning: PICELLERATOR_RPATH for C compiler \"${CC_TYPE}\" unknown";;
esac

# Only warn if we're not building PICellerator as part of this build
if test "${BUILDING_PICELLERATOR}" != "1"; then
	warnIfNotValidFile ${PICELLERATOR_DIR} PICellerator PICellerator PICELLERATOR_DIR
	warnIfNotValidFile ${PICELLERATOR_INCDIR}/PICellerator/PICellerator.h PICellerator PICELLERATOR_INCDIR "PICELLERATOR_DIR"
	warnIfNotValidFile ${PICELLERATOR_LIBDIR}/libPICellerator.a PICellerator PICELLERATOR_LIBDIR "PICELLERATOR_DIR" 
	if ! DoCompileConftest ./VMake/Config/PICellerator; then
		exitWithErrorIfNotOptionalWithMessage PICellerator ${PICELLERATOR_DIR}
	fi
else
	echo "    (PICellerator marked as being built, skipping tests)"
fi

setValueWithDefault HAVE_PICELLERATOR 1

