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
## $Id: Make.mm 955 2004-03-04 18:43:37Z SteveQuenette $
##
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

include Makefile.def
include Python/default.def

PROJECT = Snac
PYTHON_PACKAGE = ${def_pyb_package}
PACKAGE = ${def_pyb}module

BLD_MODDIR = $(EXPORT_ROOT)/modules/$(PYTHON_PACKAGE)
PROJ_DLL = $(BLD_MODDIR)/$(PACKAGE).$(EXT_SO)
PROJ_TMPDIR = $(BLD_TMPDIR)/$(PROJECT)/$(PACKAGE)
PROJ_CLEAN += $(PROJ_DLL)

PROJ_SRCS = ${def_srcs}
PROJ_CXX_FLAGS += -I$(PYTHIA_DIR)/include -I$(PYTHIA_INCDIR) -I$(EXCHANGER_INCDIR) -I$(EXPORT_ROOT)/include -I$(BLD_INCDIR)/StGermain -I$(BLD_INCDIR)/Snac -I$(STGERMAIN_INCDIR)/ -I$(STGERMAIN_INCDIR)/StGermain `xml2-config --cflags`
PROJ_LIBRARIES = -fpic -L$(TOOLS_LIBDIR) -L$(EXCHANGER_LIBDIR) -L$(PYTHIA_LIBDIR) $(PYTHIA_DIR)/modules/mpi/mpimodule.so -L$(PYTHIA_DIR)/lib -L$(BLD_LIBDIR) -L$(STGERMAIN_LIBDIR)/ $(MPI_LIBPATH) $(MPI_LIBS) -lSnac -lStGermain `xml2-config --libs`  -lExchanger -ljournal 

# hack to ensure dir is built by product_dirs
BLD_BINDIR = $(BLD_MODDIR)

all: DLL

DLL: product_dirs $(PROJ_OBJS)
	$(CXX) -o $(PROJ_DLL) $(PROJ_OBJS) $(COMPILER_LCXX_SOFLAGS) $(LCXXFLAGS) $(PROJ_LIBRARIES) $(EXTERNAL_LIBPATH) $(EXTERNAL_LIBS)
