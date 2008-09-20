#!/usr/bin/env python
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

## $Id: Application.py 3698 2006-07-20 21:09:56Z EunseoChoi $
##
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

import journal
import mpi,sys
from mpi.Application import Application as mpiApplication
import StGermain.Base.IO.Dictionary
from StGermain.Base.IO.Dictionary import Dictionary
import StGermain.pyre.Dictionary as PyreDictionary


class Application( mpiApplication ):
	def __init__( self, name ):
		mpiApplication.__init__( self, name )

		# This holds the root dictionary
		self._dictionarySource = None
		self._dictionary = Dictionary( StGermain.Base.IO.Dictionary.new() )

		self.comm = None
		self.rank = None
		self.nodes = None
		self.solver = None
		#ccccc
		self.controller = None

		self._context = None

		return

	def main(self, *args, **kwds):
		self.initialize()
		self.reportConfiguration()
		self.launch()
		return

	def getContext( self ):
		if not self._context:
			self._context = self.BuildContext()
		return self._context

	def configure( self, registry ):
		report = mpiApplication.configure( self, registry )
		self._dictionary = PyreDictionary.addProperties( self, registry )
		PyreDictionary.linkComponentDictionaries( self )
		return report


	def initialize( self ):
		self._dictionary.LoadFromFile( self.inventory.inputFilename )

		self.inventory.layout.initialize( self )
		self.comm = self.inventory.layout.comm
		self.rank = self.inventory.layout.rank
		self.nodes = self.inventory.layout.nodes
		self.findLayout( self.inventory.layout )
		## self.inventory.controller.initialize( self )
		## assumes findLayout always defines self.controller
		self.controller.initialize( self )
		return

	def launch( self ):
##		self.inventory.controller.launch( self )
##		self.inventory.controller.march( steps = self.inventory.steps, totalTime = self.inventory.stop )
		self.controller.launch( self )
		self.controller.march( totalTime = self.inventory.stop, steps = self.inventory.steps )
		return

	def findLayout( self, layout ):
		self.solverCommunicator = self.comm
		self.solver = self.inventory.solver
		self.controller = self.inventory.controller
		return

	def reportConfiguration( self ):
		rank = mpi.world().rank

		if rank != 0:
			return

		self._info.line("configuration:" )
#		self._info.line("  properties:" )
#		self._info.line("     name: %r" % self.inventory.name )
#		self._info.line("     full name: %r" % self.inventory.fullname )

		self._info.line( "  facilities:" )
		self._info.line( "    journal: %r" % self.inventory.journal.name )
		self._info.line( "    launcher: %r" % self.inventory.launcher.name )

		self._info.line( "    solver: %r" % self.inventory.solver.name )
		self._info.line( "    controller: %r" % self.inventory.controller.name )
		self._info.line( "    layout: %r" % self.inventory.controller.name )
		return

	class Inventory( mpiApplication.Inventory ):
		import pyre.inventory

		import StGermain.pyre.Solver as Solver
		import StGermain.pyre.Controller as Controller
		import StGermain.pyre.Layout as Layout

		inputFilename = pyre.inventory.str( name = "inputFilename", default = "./input.xml" )
		solver = pyre.inventory.facility( name = "solver", factory = Solver.solver )
		controller = pyre.inventory.facility( name = "controller", factory = Controller.controller )
		layout = pyre.inventory.facility( name = "layout", factory = Layout.layout )
		steps = pyre.inventory.int( "steps", default=0 )
		stop = pyre.inventory.float( "stop", default=1e20 )


