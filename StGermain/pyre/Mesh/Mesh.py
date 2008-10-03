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
## $Id: Application.py 663 2004-01-20 12:38:41Z PatrickSunter $
##
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

import journal
from StGermain.pyre.Component import Component
import StGermain.Base.IO.Dictionary
from StGermain.Base.IO.Dictionary import Dictionary

class DummyMeshLayout( Component ):
	def __init__( self, name = "dummy", facility = "Meshayout" ):
		Component.__init__( self, name, facility )
		return

	class Inventory( Component.Inventory ):
		inventory = []



class Mesh( Component ):
	def __init__( self, name, layout, facility = "Mesh" ):
		Component.__init__( self, name, facility )
		self.inventory.MeshLayout = layout

		self._dictionary = Dictionary( StGermain.Base.IO.Dictionary.new() )
		self.inventory.MeshLayout._dictionarySource = self # meshlayout writes to the same dictionary, not its own
		return

	class Inventory( Component.Inventory ):
		import pyre.inventory

		MeshLayout = pyre.inventory.facility( name = "MeshLayout", factory = DummyMeshLayout ),
