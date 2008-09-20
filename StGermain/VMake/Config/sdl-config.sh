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

# SDL Simple Direct Media Layer pkg for manipulating windows, keyboard etc

parsePackageConfigOptions $@

# Check if we're allowed to use SDL
if test "${NOSDL}" = "1" ; then
	return 0
fi

case ${SYSTEM} in
        Darwin)
		if test "${SDL_DIR}x" = "x"; then
			if test -r "/sw/include/SDL/SDL.h" ; then
				setValueWithDefault SDL_DIR       '/sw'
			elif test -r "/Library/Frameworks/SDL.framework/Headers/SDL.h" ; then
				setValueWithDefault SDL_DIR       '/Library/Frameworks/SDL.framework/'
			else
				setValueWithDefault SDL_DIR       '/usr'
			fi
		fi
		;;
        *)
		setValueWithDefault SDL_DIR       '/usr';;
esac
if test -d "${SDL_DIR}/Headers"; then
	export FRAMEWORK_SDL=1
fi
if test "${FRAMEWORK_SDL}x" = "x"; then
	setValueWithDefault SDL_INCDIR    '${SDL_DIR}/include'
	SDL_INCDIR_CHECK=`warnIfNotValidFile "${SDL_INCDIR}/SDL/SDL.h" SDL "SDL_INCDIR SDL_DIR"`
else
	setValueWithDefault SDL_INCDIR    '${SDL_DIR}/Headers'
	SDL_INCDIR_CHECK=`warnIfNotValidFile "${SDL_INCDIR}/SDL.h" SDL "SDL_INCDIR SDL_DIR"`
fi
	

if test "${SDL_INCDIR_CHECK}x" != "x"; then
        echo $SDL_INCDIR_CHECK
else
	setValueWithDefault SDL_INCLUDES   '-I${SDL_INCDIR} -I${SDL_INCDIR}/SDL/'
	if test "${FRAMEWORK_SDL}x" = "x"; then
		appendVariableIfReadable SDL_INCLUDES "${SDL_INCDIR}/SDL/SDL.h" "-DHAVE_SDL -DSDL_INSTALLED"
		setValueWithDefault SDL_LIBDIR    '${SDL_DIR}/lib/'
	else
		appendVariableIfReadable SDL_INCLUDES "${SDL_INCDIR}/SDL.h" "-DHAVE_SDL -DSDL_INSTALLED"
		setValueWithDefault SDL_LIBDIR    '${SDL_DIR}/'
	fi

	case ${SYSTEM} in 
		Darwin)
			if test "${FRAMEWORK_SDL}x" = "x"; then
				setValueWithDefault SDL_LIBFILES  '-lSDL -framework Cocoa'
			else
				setValueWithDefault SDL_LIBFILES  '-framework SDL -framework Cocoa'
			fi
			;;
		*)
			setValueWithDefault SDL_LIBFILES  '-lSDL';;
	esac		

	setValueWithDefault SDL_LIBS      '-L${SDL_LIBDIR} ${SDL_LIBFILES}'
	setValueWithDefault SDL_MAIN_LIBS '-L${SDL_LIBDIR} -lSDLmain ${SDL_LIBFILES}'

	setValueWithDefault HAVE_SDL '1'
fi	
