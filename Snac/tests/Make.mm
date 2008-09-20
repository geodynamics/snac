##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
##
## Copyright (C), 2003, 
##	Steve Quenette, 110 Victoria Street, Melbourne, Victoria, 3053, Australia.
##	Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
##	University of Texas, 1 University Station, Austin, Texas, 78712, USA.
##
## Authors:
##	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
##	Stevan M. Quenette, Visitor in Geophysics, Caltech.
##	Luc Lavier, Research Scientist, The University of Texas. (luc@utig.ug.utexas.edu)
##	Luc Lavier, Research Scientist, Caltech.
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
## $Id: Make.mm 1315 2004-04-30 11:31:29Z PatrickSunter $
##
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

include Makefile.def

PROJECT = Snac

PROJ_TIDY += $(PROJ_TESTS)
PROJ_CLEAN = $(PROJ_TIDY)
TST_DIR = $(BLD_ROOT)/tests/$(TARGET_TAG)

SH = sh
PROJ_CHECKS = ${def_checks}

PROJ_PATCH_TESTS = $(addsuffix .expected-patch,$(basename ${def_checks}))

DUMP_VARS_TO_TMP_FILE = \
	echo "TST_DIR=${TST_DIR}" > tmp.sh; \
	echo "MPI_DIR=${MPI_DIR}" >> tmp.sh; \
	echo "MPI_LIBDIR=${MPI_LIBDIR}" >> tmp.sh; \
	echo "MPI_INCDIR=${MPI_INCDIR}" >> tmp.sh;

PASSTOTALFILE = "${BLD_TMPDIR}/.passtotal"
FAILTOTALFILE = "${BLD_TMPDIR}/.failtotal"


all: 

check:: test

test:
	@if test ! -f ${PASSTOTALFILE}; then \
		echo "0" | cat > ${PASSTOTALFILE}; \
		echo "0" | cat > ${FAILTOTALFILE}; \
	fi; \
	$(DUMP_VARS_TO_TMP_FILE) \
	list='${PROJ_CHECKS}'; \
	for i in $$list; do \
		if test "x$$i" != "x"; then \
			if ${SH} ./$$i; then \
				expr `cat ${PASSTOTALFILE}` + 1 > ${PASSTOTALFILE}; \
			else \
				expr `cat ${FAILTOTALFILE}` + 1 > ${FAILTOTALFILE}; \
			fi; \
		fi; \
	done; \
	rm tmp.sh; \
	if test ${MAKELEVEL} = 0; then \
		echo "--------------------------------------------"; \
		echo "Summary of results:"; \
		echo "Passes = `cat ${PASSTOTALFILE}`"; \
		echo "Fails = `cat ${FAILTOTALFILE}`"; \
		rm -f ${PASSTOTALFILE} ${FAILTOTALFILE}; \
	fi;	


patchtests: ${PROJ_PATCH_TESTS}


release: tidy
	cvs release .

update: clean
	cvs update .

#--------------------------------------------------------------------------
#

%.sh-patch: 
	@$(DUMP_VARS_TO_TMP_FILE) \
	UPDATE_MODE="on" ${SH} ./${*}.sh; \
	rm tmp.sh;
