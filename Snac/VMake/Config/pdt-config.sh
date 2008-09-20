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

setValueWithDefault PDT_DIR '/usr/local/pdt'
assertValidDirectory ${PDT_DIR} PDT PDT_DIR

if test "${PDT_CONFIG_ARCH}x" = "x"; then
	# Attempt to be smart and pickup the arch
	NUM_ARCH=`find ${PDT_DIR} -maxdepth 1 -mindepth 1 -type d | grep -v man | grep -v include | grep -v etc | wc -l | sed 's/^[ ]*//'`
	case ${NUM_ARCH} in
		0)
			echo "Error - No PDT_CONFIG_ARCH found in ${PDT_DIR}. Check that PDT has been installed properly."
			exit 1
			;;
		1)
			setValueWithDefault PDT_CONFIG_ARCH \
				`find ${PDT_DIR} -maxdepth 1 -mindepth 1 -type d | grep -v man | grep -v include | grep -v etc | xargs basename`
			;;
		*)
			echo "Error - More than one architecture found in ${PDT_DIR}. Please select and set PDT_CONFIG_ARCH and PDT_OPTIONS."
			exit 1
			;;
	esac
fi

assertValidExecutable "$PDT_DIR/$PDT_CONFIG_ARCH/bin/cparse" "cparse"
assertValidExecutable "$PDT_DIR/$PDT_CONFIG_ARCH/bin/cxxparse" "cxxparse"
assertValidExecutable "$PDT_DIR/$PDT_CONFIG_ARCH/bin/f95parse" "f95parse"


setValueWithDefault PDTPARSECXX "${PDT_DIR}/${PDT_CONFIG_ARCH}/bin/cxxparse"
setValueWithDefault PDTPARSEC "${PDT_DIR}/${PDT_CONFIG_ARCH}/bin/cparse"
setValueWithDefault PDTPARSEF "${PDT_DIR}/${PDT_CONFIG_ARCH}/bin/f95parse"

