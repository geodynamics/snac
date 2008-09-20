##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
##
## Copyright (C), 2003, 
##	Steve Quenette, 110 Victoria Street, Melbourne, Victoria, 3053, Australia.
##	Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
##	University of Texas, 1 University Station, Austin, Texas, 78712, USA.
##
## Authors:
##	Stevan M. Quenette, Senior Software Engineer, VPAC.
##	Stevan M. Quenette, Visitor in Geophysics, Caltech.
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
## $Id: Make.mm 1260 2004-04-20 20:01:15Z SteveQuenette $
##
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

include Makefile.def

PROJECT = snac2dx
PACKAGE = ${def_bin}

PROJ_BIN = $(BLD_BINDIR)/$(PACKAGE)
PROJ_TMPDIR = $(BLD_TMPDIR)/$(PROJECT)/$(PACKAGE)
PROJ_CLEAN += $(PROJ_BIN)
#PROJ_CC_FLAGS += -I$(BLD_INCDIR)/$(PROJECT) -I$(BLD_INCDIR)/StGermain `xml2-config --cflags`
#PROJ_LIBRARIES = ${addprefix $(BLD_LIBDIR)/, ${def_libs}}
#LIBS = $(PROJ_LIBRARIES) -lm $(EXTERNAL_LIBS) `xml2-config --libs`
LIBS =
EXPORT_BINS = $(PROJ_BIN)
PROJ_BINDIR = $(EXPORT_BINDIR)

PROJ_SRCS = ${def_srcs}

all: $(PROJ_BIN) export

export:: export-binaries

$(PROJ_BIN): $(PROJ_SRCS)
	${CC} -o $@ $< $(CC_FLAGS) $(CC_BUILD_DEFINES) $(CC_BUILD_INCLUDES) -I$(PROJ_INCDIR) $(LCFLAGS) $(LIBS)

check::
