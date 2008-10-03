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
##
## $Id: Layout.py 3698 2006-07-20 21:09:56Z EunseoChoi $
##
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

def layout(name="layout", facility="layout"):
		    return Layout(name, facility)


from pyre.components.Component import Component
import journal
import mpi


class Layout( Component ):
	def __init__( self, name = "layout", facility = "layout" ):
		Component.__init__( self, name, facility )

		self.coarse = None
		self.fine = None
		self.coarsePlus = []
		self.finePlus = []

		self.comm = None
		self.rank = 0
		self.nodes = 0
		return

	def initialize( self, application ):
		self.discover()
		self.verify( application )
		self.allocateNodes()
		self.createCommunicators()
		return

	def discover( self ):
		self.comm = mpi.world()
		self.rank = self.comm.rank
		self.nodes = self.comm.size

		return

	def verify( self, application ):
		size = self.nodes
		nodes = application.inventory.launcher.inventory.nodes
		if nodes == 0 and size == 1:
			info = journal.info( "layout" )
			info.log( "Running serially without a stager" )

		elif nodes != size:
			firewall = journal.firewall( "layout" )
			firewall.log( "processor count mismatch: %d != %d" % ( nodes, self.nodes ) )

		return

	def allocateNodes( self ):
		return

	def createCommunicators( self ):
		world = self.comm
		myrank = world.rank
		fineGroup = self.inventory.fine
		coarseGroup = self.inventory.coarse

		if self.nodes == 1:
			return
		else:
			self.fine = world.include( fineGroup )
			self.coarse = world.include( coarseGroup )

		for each in coarseGroup:
			self.finePlus.append( world.include( fineGroup + [each] ) )

		for each in fineGroup:
			self.coarsePlus.append( world.include( coarseGroup + [each] ) )

		return


	class Inventory( Component.Inventory ):

		import pyre.inventory

		coarse = pyre.inventory.slice( "coarse", default=[0] )
		fine = pyre.inventory.slice( "fine", default=[1] )













