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

setValueWithDefault STGFEM_DIR '${BLD_DIR}'
setValueWithDefault STGFEM_INCDIR '${STGFEM_DIR}/include'
setValueWithDefault STGFEM_INCLUDES '-I${STGFEM_INCDIR}'
setValueWithDefault STGFEM_LIBDIR '${STGFEM_DIR}/lib'
setValueWithDefault STGFEM_LIBS '-L${STGFEM_LIBDIR} -lStgFEM -lStgFEM_SLE_PETScLinearAlgebra'

case $CC_TYPE in
	gnu)
		case $SYSTEM in
			Linux|CYGWIN)
				setValueWithDefault STGFEM_RPATH '-Xlinker -rpath -Xlinker ${STGFEM_LIBDIR}';;
			Darwin)
				# Darwin doesn't support RPATH... use INSTALL NAME instead
				setValueWithDefault STGFEM_RPATH '';;
			OSF1)
				setValueWithDefault STGFEM_RPATH '-Wl,-rpath,${STGFEM_LIBDIR}';;
			*)
				echo "Warning: STGFEM_RPATH for C compiler \"${CC_TYPE}\" unknown for system \"${SYSTEM}\"";;
		esac ;;
	osf)
		# NOTE: This may not work... I think only ONE rpath arguement is accepted/read.
		setValueWithDefault STGFEM_RPATH '-rpath ${STGFEM_LIBDIR}';;
	intel)
		setValueWithDefault STGFEM_RPATH '-Xlinker -rpath -Xlinker ${STGFEM_LIBDIR}';;
	sparc)
		setValueWithDefault STGFEM_RPATH '-R ${STGFEM_LIBDIR}';;
	ibmxl)
		setValueWithDefault STGFEM_RPATH '-R ${STGFEM_LIBDIR}';;
	*)
		echo "Warning: STGFEM_RPATH for C compiler \"${CC_TYPE}\" unknown";;
esac

# Only warn if we're not building StgFEM as part of this build
if test "${BUILDING_STGFEM}" != "1"; then
	warnIfNotValidFile ${STGFEM_DIR} StgFEM StgFEM STGFEM_DIR
	warnIfNotValidFile ${STGFEM_INCDIR}/StgFEM/StgFEM.h StgFEM STGFEM_INCDIR "STGFEM_DIR"
	warnIfNotValidFile ${STGFEM_LIBDIR}/libStgFEM.a StgFEM STGFEM_LIBDIR "STGFEM_DIR" 
	if ! DoCompileConftest ./VMake/Config/StgFEM; then
		exitWithErrorIfNotOptionalWithMessage StgFEM ${STGFEM_DIR}

	fi
else
	echo "    (StgFEM marked as being built, skipping tests)"
fi

setValueWithDefault HAVE_STGFEM 1

