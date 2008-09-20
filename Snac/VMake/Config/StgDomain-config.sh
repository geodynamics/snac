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

setValueWithDefault STGDOMAIN_DIR '${BLD_DIR}'
setValueWithDefault STGDOMAIN_INCDIR '${STGDOMAIN_DIR}/include'
setValueWithDefault STGDOMAIN_INCLUDES '-I${STGDOMAIN_INCDIR}'
setValueWithDefault STGDOMAIN_LIBDIR '${STGDOMAIN_DIR}/lib'
setValueWithDefault STGDOMAIN_LIBS '-L${STGDOMAIN_LIBDIR} -lStgDomain'

case $CC_TYPE in
	gnu)
		case $SYSTEM in
			Linux|CYGWIN)
				setValueWithDefault STGDOMAIN_RPATH '-Xlinker -rpath -Xlinker ${STGDOMAIN_LIBDIR}';;
			Darwin)
				# Darwin doesn't support RPATH... use INSTALL NAME instead
				setValueWithDefault STGDOMAIN_RPATH '';;
			OSF1)
				setValueWithDefault STGDOMAIN_RPATH '-Wl,-rpath,${STGDOMAIN_LIBDIR}';;
			*)
				echo "Warning: STGDOMAIN_RPATH for C compiler \"${CC_TYPE}\" unknown for system \"${SYSTEM}\"";;
		esac ;;
	osf)
		# NOTE: This may not work... I think only ONE rpath arguement is accepted/read.
		setValueWithDefault STGDOMAIN_RPATH '-rpath ${STGDOMAIN_LIBDIR}';;
	intel)
		setValueWithDefault STGDOMAIN_RPATH '-Xlinker -rpath -Xlinker ${STGDOMAIN_LIBDIR}';;
	sparc)
		setValueWithDefault STGDOMAIN_RPATH '-R ${STGDOMAIN_LIBDIR}';;
	ibmxl)
		setValueWithDefault STGDOMAIN_RPATH '-R ${STGDOMAIN_LIBDIR}';;
	*)
		echo "Warning: STGDOMAIN_RPATH for C compiler \"${CC_TYPE}\" unknown";;
esac

# Only warn if we're not building StgDomain as part of this build
if test "${BUILDING_STGDOMAIN}" != "1"; then
	warnIfNotValidFile ${STGDOMAIN_DIR} StgDomain StgDomain STGDOMAIN_DIR
	warnIfNotValidFile ${STGDOMAIN_INCDIR}/StgDomain/StgDomain.h StgDomain STGDOMAIN_INCDIR "STGDOMAIN_DIR"
	warnIfNotValidFile ${STGDOMAIN_LIBDIR}/libStgDomain.a StgDomain STGDOMAIN_LIBDIR "STGDOMAIN_DIR" 
	if ! DoCompileConftest ./VMake/Config/StgDomain; then
		exitWithErrorIfNotOptionalWithMessage StgDomain ${STGDOMAIN_DIR}

	fi
else
	echo "    (StgDomain marked as being built, skipping tests)"
fi

setValueWithDefault HAVE_STGDOMAIN 1

