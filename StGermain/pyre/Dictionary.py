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
from StGermain.Base.IO.Dictionary import Dictionary


# Fill in the dictionary with the inventory's properties
def addProperties( object, registry ):
	if object._dictionary == None:
		if object._dictionarySource == None:
			dictionary = Dictionary()
		else:
			dictionary = object._dictionarySource._dictionary
	else:
		dictionary = object._dictionary

	for name, _value in object.inventory._propertyRegistry.iteritems():
		value = object.inventory.__getattribute__( name )
		dictionary.add( name, value.__str__() );

	return dictionary

# Fill in the dictionary with the inventory's component dictionaries, recursive
def linkComponentDictionaries( object ):
	dictionary = object._dictionary
	for name, _value in object.inventory._facilityRegistry.iteritems():
		component = object.inventory.__getattribute__( name )
		
		# Ignore components that don't have a dictionary and those that do, but have a source dictionary (i.e. are sharing)
		try:
			if component._dictionary != None:
				if component._dictionarySource == None:
					linkComponentDictionaries( component )
					dictionary.add( component.name, component._dictionary )
		except AttributeError:
			None
