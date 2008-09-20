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
## $Id: Dictionary.py 3462 2006-02-19 06:53:24Z WalterLandry $
##
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

import StGermain.Base.Bindings.Dictionary as bindings

def copyright():
	return "StGermain.Base.IO.Dictionary Python module: Copyright (c) 2003 Victorian Partnership for Advanced Computing (VPAC) Ltd. Australia.";

def new():
	return bindings.New()

class Dictionary:
	def Print( self ):
		return bindings.Print( self._handle )
	
	def add( self, name, value ):
		# If the value has a handle, add it as a struct/dictionary
		try:
			return bindings.AddStruct( self._handle, name, value._handle )
		except AttributeError:
			if value.__class__ == "<type 'list'>":
				bindings.AddList( self._handle, name, value )
				addToList( self._handle, name, value )
			else:
				return bindings.AddString( self._handle, name, value )
	
	def addToList( self, listName, value ):
		#TODO addToList( self, listName, value )
		return None
	
	def LoadFromFile( self, filename ):
		return bindings.LoadFromFile( self._handle, filename )
	
	def __init__( self, handle ):
		self._handle = handle
		return

# version
__id__ = "$Id: Dictionary.py 3462 2006-02-19 06:53:24Z WalterLandry $"
