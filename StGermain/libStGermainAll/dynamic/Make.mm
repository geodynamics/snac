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
PACKAGE = lib${def_lib}

#hack
DL_INCDIR=/usr/include
DL_LIBDIR=/usr/lib
DL_LIBS=-ldl
MODULE_EXT=$(EXT_SO)
SO_EXT=$(EXT_SO)

#PROJ_LIB = $(BLD_LIBDIR)/$(PACKAGE).a
PROJ_DLL = $(BLD_LIBDIR)/$(PACKAGE).$(EXT_SO)
PROJ_TMPDIR = $(BLD_TMPDIR)/$(PROJECT)/$(PACKAGE)
PROJ_CLEAN += $(PROJ_LIB) $(PROJ_DLL) $(PROJ_OBJS)
PROJ_CC_FLAGS += -I$(BLD_INCDIR)/$(PROJECT) `xml2-config --cflags` -I${DL_INCDIR} -DMODULE_EXT=\"${MODULE_EXT}\"
PROJ_INCDIR = $(BLD_INCDIR)/${def_inc}
PROJ_LIBRARIES = ${addprefix $(BLD_LIBDIR)/, ${def_libs}}
LIBS = -L$(BLD_LIBDIR) -lStGermainFE -lStGermainFD -lStGermain -lStGermainDiscretisation -lStGermainBase $(PROJ_LIBRARIES) -lm $(EXTERNAL_LIBS) `xml2-config --libs` -L${DL_LIBDIR} ${DL_LIBS}

PROJ_SRCS = ${def_srcs}
PROJ_OBJS_IN_TMP = ${addprefix $(PROJECT)/$(PACKAGE)/, ${addsuffix .o, ${basename ${PROJ_SRCS}}}}
PROJ_OBJLIST = ${BLD_TMPDIR}/${PROJECT}.${PACKAGE}.objlist

SED = sed
OBJLISTS=${addprefix ${BLD_TMPDIR}/${PROJECT}.lib, ${addsuffix .objlist, ${def_objlists}}}
SUBPROJ_OBJS=${patsubst %, `cat % | ${SED} 's/^/${subst /,\/,${BLD_TMPDIR}}\//g' | ${SED} 's/ / ${subst /,\/,${BLD_TMPDIR}}\//g'` , ${OBJLISTS}}


#all: $(PROJ_LIB) createObjList appendSubLibs $(PROJ_DLL) export
all: DLL export

createObjList:: 
	@echo ${PROJ_OBJS_IN_TMP} | cat > ${PROJ_OBJLIST}
	@if test ! "${OBJLISTS}x" = "x"; then cat ${OBJLISTS} | cat >> ${PROJ_OBJLIST}; fi

appendSubLibs::
	$(AR) -q $(PROJ_LIB) $(SUBPROJ_OBJS)
	$(RANLIB) $(RANFLAGS) $(PROJ_LIB)

DLL: product_dirs $(PROJ_OBJS)
	$(CC) -o $(PROJ_DLL) $(PROJ_OBJS) $(SUBPROJ_OBJS) $(LCC_SOFLAGS) $(LCFLAGS) $(LIBS)

#export:: export-headers
export:: export-headers export-libraries
EXPORT_HEADERS = ${def_hdrs}
EXPORT_LIBS = $(PROJ_LIB) $(PROJ_DLL)

check::
