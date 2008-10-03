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
##	Raquibul Hassan, Computational Engineer, VPAC. (raq@vpac.org)
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
## $Id: Make.mm 3462 2006-02-19 06:53:24Z WalterLandry $
##
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

include Makefile.def

PROJECT = StGermain

#hack ... need to add a "libdl" external to MM and smarts to know that a dll's extension is called a "dylib" on a mac.
DL_LIBS=-ldl
MODULE_EXT=$(EXT_SO)
SO_EXT=$(EXT_SO)

PROJ_TIDY += $(PROJ_TESTS)
PROJ_CLEAN = $(PROJ_TIDY)
TST_DIR = $(BLD_ROOT)/tests/$(TARGET_TAG)

PROJ_LIBRARIES = ${addprefix $(BLD_LIBDIR)/, ${def_libs}}
LIBS = $(PROJ_LIBRARIES) -lm $(EXTERNAL_LIBS) `xml2-config --libs` ${DL_LIBS}
SH = sh
PROJ_TESTS = ${addprefix ${TST_DIR}/, ${basename ${def_srcs}}}
PROJ_CHECKS = ${def_checks}
PROJ_CC_FLAGS += -I${BLD_INCDIR}/StGermain `xml2-config --cflags`

PASSTOTALFILE = "${BLD_TMPDIR}/.passtotal"
FAILTOTALFILE = "${BLD_TMPDIR}/.failtotal"

DUMP_VARS_TO_TMP_FILE = \
	echo "TST_DIR=${TST_DIR}" > tmp.sh; \
	echo "MPI_DIR=${MPI_DIR}" >> tmp.sh; \
	echo "MPI_LIBDIR=${MPI_LIBDIR}" >> tmp.sh; \
	echo "MPI_INCDIR=${MPI_INCDIR}" >> tmp.sh;

all: $(PROJ_TESTS)


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


release: tidy
	cvs release .

update: clean
	cvs update .

#--------------------------------------------------------------------------
#

${TST_DIR}/%: %.c ${PROJ_LIBRARIES}
	@if test ! -d ${TST_DIR}; then mkdir -p ${TST_DIR}; fi
	${CC} -o $@ $< $(CC_FLAGS) $(PROJ_CC_FLAGS) $(CC_BUILD_DEFINES) $(CC_BUILD_INCLUDES) $(LCFLAGS) $(LIBS)
