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
## $Id: Context.py 3870 2006-10-16 13:56:38Z SteveQuenette $
##
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

import StGermain.Base.Bindings.Context as bindings
import StGermain.Base.IO.Dictionary as Dictionary

def copyright():
    return "StGermain.Base.Context.Context Python module: Copyright (c) 2003 Victorian Partnership for Advanced Computing (VPAC) Ltd. Australia.";

class Context( object ):
	def Print( self ):
		return bindings.Print( self._handle )
	
	def Construct( self ):
		self._handle = bindings.Construct( self._handle )
		return None
	
	def Build( self ):
		return bindings.Build( self._handle )
	
	def Initialise( self ):
		return bindings.Initialise( self._handle )
	
	def Execute( self ):
		return bindings.Execute( self._handle )
		
	def Destroy( self ):
		return bindings.Destroy( self._handle )
		
	def Dt( self ):
		return bindings.Dt( self._handle )
	
	def Step( self, dt ):
		return bindings.Step( self._handle, dt )
	
	def __init__( self, handle ):
		self._handle = handle
		self.dictionary = Dictionary.Dictionary( bindings.GetDictionary( self._handle ) )
		return

	def handle(self):
		return self._handle

	def Delete( self ):
		return bindings.Delete( self._handle )

	def SetTime( self, time ):	
		bindings.SetTime( self._handle, time )
		self.__time = time

	def GetTime( self ):
		return self.__time

	time = property( GetTime, SetTime, doc="Current sim. time.")
	
	def SetStep( self, timeStep ):
		bindings.SetTimeStep( self._handle, timeStep )
		self.__timeStep = timeStep

	def GetStep( self ):	
		return self.__timeStep

	step = property( GetStep, SetStep, doc="Current sim. step.")
