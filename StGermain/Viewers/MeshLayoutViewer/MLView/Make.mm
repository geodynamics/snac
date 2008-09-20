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
include GLUT/default.def

PROJECT = StGermain
PACKAGE = ${def_bin}

#hack
DL_INCDIR=/usr/include
DL_LIBDIR=/usr/lib
DL_LIBS=-ldl
MODULE_EXT=so

PROJ_BIN = $(BLD_BINDIR)/$(PACKAGE)
PROJ_TMPDIR = $(BLD_TMPDIR)/$(PROJECT)/$(PACKAGE)
PROJ_CLEAN += $(PROJ_BIN)
PROJ_CC_FLAGS += -I$(BLD_INCDIR)/$(PROJECT) -I$(BLD_INCDIR)/StGermain `xml2-config --cflags` -I${DL_INCDIR} -DMODULE_EXT=\"${MODULE_EXT}\"
PROJ_LIBRARIES = ${addprefix $(BLD_LIBDIR)/, ${def_libs}}
LIBS = $(PROJ_LIBRARIES) -lm $(EXTERNAL_LIBS) `xml2-config --libs` -L${DL_LIBDIR} ${DL_LIBS}

EXPORT_BINS = $(PROJ_BIN)
PROJ_BINDIR = $(EXPORT_BINDIR)

PROJ_SRCS = ${def_srcs}

all: $(PROJ_BIN) export

export:: export-binaries

$(PROJ_BIN): $(PROJ_OBJS)
	${CC} -o $@ $(PROJ_OBJS) $(CC_FLAGS) $(CC_BUILD_DEFINES) $(CC_BUILD_INCLUDES) -I$(PROJ_INCDIR) $(LCFLAGS) $(LIBS)

check::
