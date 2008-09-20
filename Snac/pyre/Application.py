#!/usr/bin/env python
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
## $Id: Application.py 3240 2006-07-20 21:11:57Z EunseoChoi $
##
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

import StGermain.pyre.Application

StGermainApplication = StGermain.pyre.Application.Application

class Application( StGermainApplication ):
	def __init__( self, name = "Snac" ):
		StGermainApplication.__init__( self, name )
		self._stgCommunicator = None
		return

	def BuildContext( self ):
		import StGermain.pyre.Bindings.pyre
		import Snac.Context

		# move StGermain initialization somewhere before the instantiation of Snac.Application:
		# e.g. in CoupledApp.py, basic-pyre.py, etc.
		#StGermain.initialise()
		self._stgCommunicator = StGermain.pyre.Bindings.pyre.ConvertFromPyreComm( self.solverCommunicator )
		return Snac.Context.Context( self._dictionary, self._stgCommunicator )



	class Inventory( StGermainApplication.Inventory ):
		import pyre.inventory

		from StGermain.pyre.Mesh.RegularMeshLayout import RegularMeshLayout
		from StGermain.pyre.Mesh.Mesh import Mesh as StGMesh

		Mesh = pyre.inventory.facility( name="Mesh", default=StGMesh(name = "mesh", layout = RegularMeshLayout()) )
		outputPath = pyre.inventory.str( name = "outputPath", default = "./" )
		dumpEvery = pyre.inventory.int( name = "dumpEvery", default = 10 )

		density = pyre.inventory.int( name = "density", default = 2700 )
		gravity = pyre.inventory.float( name = "gravity", default = 0 )
		demf = pyre.inventory.float( name = "demf", default = 0.8 )

