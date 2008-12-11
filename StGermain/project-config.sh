#!/bin/sh
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
##
## Copyright (C), 2003, Victorian Partnership for Advanced Computing (VPAC) Ltd, 110 Victoria Street, Melbourne, 3053, Australia.
##
## Authors:
##	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
##	Patrick D. Sunter, Software Engineer, VPAC. (pds@vpac.org)
##	Luke J. Hodkinson, Computational Engineer, VPAC. (lhodkins@vpac.org)
##	Siew-Ching Tan, Software Engineer, VPAC. (siew@vpac.org)
##	Alan H. Lo, Computational Engineer, VPAC. (alan@vpac.org)
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
##
## $Id: project-config.sh 3864 2006-10-16 09:21:03Z SteveQuenette $
##
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# Be Bourne compatible
if test -n "${ZSH_VERSION+set}" && (emulate sh) >/dev/null 2>&1; then
	emulate sh
	NULLCMD=:
elif test -n "${BASH_VERSION+set}" && (set -o posix) >/dev/null 2>&1; then
	set -o posix
fi

. './build-functions.sh'

setValue PROJECT 'StGermain'
getVariableFromMakefile VERSION makefile
export STGERMAIN_VERSION=${VERSION}

# Setup Makefile shortcuts
. ./VMake/Config/makefile-shortcuts.sh

. ./VMake/Config/compiler-config.sh
. ./VMake/Config/math-config.sh
. ./VMake/Config/xml-config.sh
. ./VMake/Config/mpi-config.sh
##. ./VMake/Config/python-config.sh --optional
##. ./VMake/Config/PETSc-config.sh --optional
##. ./VMake/Config/OpenGl-config.sh
##. ./VMake/Config/sdl-config.sh
##. ./VMake/Config/hdf5-config.sh
##. ./VMake/Config/neon-config.sh

dumpPkgConfigFile ${BLD_DIR}/lib/pkgconfig
