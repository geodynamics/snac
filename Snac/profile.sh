##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
##
## Copyright (C), 2003, Victorian Partnership for Advanced Computing (VPAC) Ltd, 110 Victoria Street, Melbourne, 3053, Australia.
##
## Authors:
##	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
##
## This program is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by the
## Free Software Foundation; either version 2, or (at your option) any
## later version.
## 
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
## 
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
##
## Role:
##	Set up a sh/bash environment for using StGermain built things.
##
## $Id: profile.sh 1081 2004-03-26 06:50:27Z PatrickSunter $
##
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

readDefinedPath() {
	if test "$1x" != "x"; then
		echo $1
	elif test -r ./Makefile.system; then
		grep "$2=" ./Makefile.system | cut -d'=' -f2
	else
		echo "`pwd`/$3"
	fi	
	return 0
}

export PATH=`readDefinedPath "$BIN_DIR" BIN_DIR build/bin`:$PATH
export PYTHONPATH=`readDefinedPath "$PYC_DIR" PYC_DIR build/Python`:$PYTHONPATH
export LD_LIBRARY_PATH=`readDefinedPath "$LIB_DIR" LIB_DIR build/lib`:$LD_LIBRARY_PATH
export DYLD_LIBRARY_PATH=`readDefinedPath "$LIB_DIR" LIB_DIR build/lib`:$DYLD_LIBRARY_PATH
