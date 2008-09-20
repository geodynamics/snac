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

#Obtain OpenGl library information

# Check if we're allowed to use OPENGL
if test "${NOOPENGL}" = "1" ; then
	return 0
fi

case ${SYSTEM} in
	Darwin)
		setValueWithDefault GL_INCDIR '/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers/'
		setValueWithDefault GL_LIBDIR '/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/'
		warnIfNotValidFile "${GL_INCDIR}/gl.h" OpenGL GL_INCDIR;;
	*)
		setValueWithDefault GL_DIR '/usr'
		setValueWithDefault GL_INCDIR '${GL_DIR}/include'
		if test "${SYSTEM_64}" = "1" && test -r ${GL_DIR}/lib64; then
			setValueWithDefault GL_LIBDIR '${GL_DIR}/lib64'
		else
			setValueWithDefault GL_LIBDIR '${GL_DIR}/lib'
		fi
		warnIfNotValidFile "${GL_INCDIR}/GL/gl.h" OpenGL GL_INCDIR;;
esac
if test "${RPATH_FLAG}x" != "x" ; then
	setValueWithDefault GL_RPATH '${RPATH_FLAG}${GL_LIBDIR}'
fi

setValueWithDefault GL_INCLUDES   '-I${GL_INCDIR} -I${GL_INCDIR}/GL'
appendVariableIfReadable GL_INCLUDES "${GL_INCDIR}/gl.h" -DHAVE_GL
appendVariableIfReadable GL_INCLUDES "${GL_INCDIR}/GL/gl.h" -DHAVE_GL

case ${SYSTEM} in 
	Darwin)
		GLU_HEADER="${GL_INCDIR}/glu.h"
		;;
	*)
		GLU_HEADER="${GL_INCDIR}/GL/glu.h"
		;;
esac

if ! test -r $GLU_HEADER; then
	echo "Error: GL/glu.h not found. Please install the appropriate package for this on your system"
	echo ""
	echo "Note: glu.h appears differently on different systems. For example, GL, xorg, mesa...etc"
	echo "If you are using fedora core 5, use yum to install mesa"
	exitWithErrorIfNotOptional
fi

if test "x`echo ${GL_INCLUDES} | grep HAVE_GL`" != "x"; then
	setValueWithDefault HAVE_GL '1'
fi

setValueWithDefault GL_LIBFILES  '-lGL -lGLU'
setValueWithDefault GL_LIBS      '-L${GL_LIBDIR} ${GL_LIBFILES}'

# Check if we're allowed to use OSMESA
if test "${NOOSMESA}" = "1" ; then
	return 0
fi

setValueWithDefault OSMESA_DIR           '${GL_DIR}'
setValueWithDefault OSMESA_INCDIR        '${OSMESA_DIR}/include'
setValueWithDefault OSMESA_INCLUDES      '-I${OSMESA_INCDIR} -I${OSMESA_INCDIR}/GL/'

if test -r "${OSMESA_INCDIR}/osmesa.h" ; then 
	OSMESA_HEADER="${OSMESA_INCDIR}/osmesa.h"
elif test -r "${OSMESA_INCDIR}/GL/osmesa.h" ; then
	OSMESA_HEADER="${OSMESA_INCDIR}/GL/osmesa.h"
fi

if test "${SYSTEM_64}" = "1" && test -r ${OSMESA_DIR}/lib64; then
	setValueWithDefault OSMESA_LIBDIR        '${OSMESA_DIR}/lib64'
else
	setValueWithDefault OSMESA_LIBDIR        '${OSMESA_DIR}/lib'
fi

setValueWithDefault OSMESA_LIBFILES      '-lOSMesa'
setValueWithDefault OSMESA_LIBS          '-L${OSMESA_LIBDIR} ${OSMESA_LIBFILES}'

if DoCompileConftest ./VMake/Config/OpenGl; then
	# Success
	echo "OSMESA_INCLUDES+= -DHAVE_OSMESA" >> Makefile.system
	setValueWithDefault HAVE_OSMESA '1'
	if test "${RPATH_FLAG}x" != "x" ; then
		setValueWithDefault OSMESA_RPATH '${RPATH_FLAG}${OSMESA_LIBDIR}'
	fi
else
	echo "    (OSMESA offscreen rendering will be unavailable)"
fi

