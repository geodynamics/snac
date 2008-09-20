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

# Set warning and symbol level
case ${CC_TYPE} in
	gnu)
		setValueWithDefault CC_SYMBOLLEVEL '-g'
		setValueWithDefault CC_WARNINGLEVEL '-Wall';;
	osf)
		setValueWithDefault CC_SYMBOLLEVEL '-g3'
		setValueWithDefault CC_WARNINGLEVEL "-msg_enable level3 -msg_disable unstructmem -msg_disable undefenum -msg_disable nonstandcast -msg_disable mixfuncvoid";;
	intel)
		setValueWithDefault CC_SYMBOLLEVEL '-g'
		setValueWithDefault CC_WARNINGLEVEL '-w1';;
	ibmxl)
		setValueWithDefault CC_SYMBOLLEVEL '-g'
		setValueWithDefault CC_WARNINGLEVEL '-qformat=all -qwarn64';;
	sparc)
		setValueWithDefault CC_SYMBOLLEVEL '-g'
		setValueWithDefault CC_WARNINGLEVEL '';;
	*)
		echo "Warning: CC_WARNINGLEVEL for C compiler \"${CC_TYPE}\" unknown. Please set."
		echo "Warning: CC_SYMBOLLEVEL for C compiler \"${CC_TYPE}\" unknown. Please set.";;  
esac

setValueWithDefault CC_DEBUGLEVEL "-DDEBUG -DQUERY_FP"
