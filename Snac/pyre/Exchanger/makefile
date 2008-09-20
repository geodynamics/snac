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
## $Id: Makefile.rules 662 2004-01-20 12:34:17Z PatrickSunter $
##
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# obtain defaults for required variables according to system and project location, and then run the build.
ifndef PROJ_ROOT
	PROJ_ROOT=../..
endif
include ${PROJ_ROOT}/Makefile.system

include Makefile.def

pyb = ${def_pyb}
PYTHON_PACKAGE = ${def_pyb_package}

CXX_EXT = cc
SRCS = ${def_srcs}

HDRS = ${def_hdrs}

PROJ_LIBS = ${def_libs}
PROJ_OBJLISTS = ${def_objlists}
EXTERNAL_LIBS = -L$(EXCHANGER_LIBDIR) -lExchanger -L$(PYTHIA_LIBDIR) -l_mpimodule -ljournal  -L${BLD_DIR}/lib -lSnac -L${STGERMAIN_LIBDIR} -lStGermainFD -lStGermain 

EXTERNAL_INCLUDES = -I$(EXCHANGER_INCDIR) -I$(EXPORT_ROOT)/include -I$(PYTHIA_DIR)/include -I$(PYTHIA_INCDIR) -I$(BLD_INCDIR)/Snac -I${STGERMAIN_INCDIR}/StGermain -I${STGERMAIN_INCDIR} 

packages = MPI XML MATH

include ${PROJ_ROOT}/Makefile.vmake
