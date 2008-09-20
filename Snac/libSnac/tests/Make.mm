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
## $Id: Make.mm 2599 2005-02-01 05:35:05Z PatrickSunter $
##
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

include Makefile.def

PROJECT = Snac

#hack
DL_INCDIR=/usr/include
DL_LIBDIR=/usr/lib
DL_LIBS=-ldl
MODULE_EXT=so

PROJ_TIDY += $(PROJ_TESTS)
PROJ_CLEAN = $(PROJ_TIDY)
TST_DIR = $(BLD_ROOT)/tests/$(TARGET_TAG)

PROJ_CC_FLAGS += -I$(BLD_INCDIR)/$(PROJECT) -I$(BLD_INCDIR)/StGermain -I$(STGERMAIN_INCDIR)/ -I$(STGERMAIN_INCDIR)/StGermain `xml2-config --cflags` -I${DL_INCDIR} -DMODULE_EXT=\"${MODULE_EXT}\" -DCURR_MODULE_NAME=\"${def_tst}\"
PROJ_LIBRARIES = ${addprefix $(BLD_LIBDIR)/, ${def_libs}} 
LIBS = $(PROJ_LIBRARIES) -L${BLD_LIBDIR} -lStGermainFD -lStGermain -lm $(EXTERNAL_LIBS) `xml2-config --libs` -L${DL_LIBDIR} ${DL_LIBS}
SH = sh
PROJ_TESTS = ${addprefix ${TST_DIR}/, ${basename ${def_srcs}}}
PROJ_TEST_OBJS = ${addprefix ${BLD_TMPDIR}/test-${def_tst}/, ${addsuffix .o, ${basename ${def_test_support_srcs}}}}
PROJ_CHECKS = ${def_checks}
PROJ_PATCH_TESTS = $(addsuffix .expected-patch,$(basename ${def_checks}))

PASSTOTALFILE = "${BLD_TMPDIR}/.passtotal"
FAILTOTALFILE = "${BLD_TMPDIR}/.failtotal"


DUMP_VARS_TO_TMP_FILE = \
	echo "TST_DIR=${TST_DIR}" > tmp.sh; \
	echo "MPI_DIR=${MPI_DIR}" >> tmp.sh; \
	echo "MPI_LIBDIR=${MPI_LIBDIR}" >> tmp.sh; \
	echo "MPI_INCDIR=${MPI_INCDIR}" >> tmp.sh;

.SECONDARY: ${PROJ_TEST_OBJS}

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


patchtests: ${PROJ_PATCH_TESTS}



release: tidy
	cvs release .

update: clean
	cvs update .

#--------------------------------------------------------------------------
#

${BLD_TMPDIR}/test-${def_tst}/%.o: %.c ${PROJ_LIBRARIES}
	@if test ! -d ${BLD_TMPDIR}/test-${def_tst}; then ${MKDIR} ${BLD_TMPDIR}/test-${def_tst}; fi
	${CC} -c -o $@  $<  $(CC_FLAGS) $(CC_BUILD_DEFINES) $(CC_BUILD_INCLUDES) -I$(PROJ_INCDIR)

${TST_DIR}/%: %.c ${PROJ_LIBRARIES} ${PROJ_TEST_OBJS}
	@if test ! -d ${TST_DIR}; then mkdir -p ${TST_DIR}; fi
	${CC} -o $@ $< ${PROJ_TEST_OBJS} $(CC_FLAGS) $(CC_BUILD_DEFINES) $(CC_BUILD_INCLUDES) -I$(PROJ_INCDIR) $(LCFLAGS) $(LIBS)

%.sh-patch: 
	@$(DUMP_VARS_TO_TMP_FILE) \
	UPDATE_MODE="on" ${SH} ./${*}.sh; \
	rm tmp.sh;
