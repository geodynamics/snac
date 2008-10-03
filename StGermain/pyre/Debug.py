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
## $Id$
##
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


def indent( level ):
	str = ''
	for i in range( level ):
		str += '\t'
	return  str


def _printRegistry( registry, level ):
	print indent( level ) + 'Properties are:'
	print indent( level + 1 ) + '---'
	for name, descriptor in registry.properties.iteritems():
		print indent( level + 1 ) + 'name: ' + name
		print indent( level + 1 ) + 'value: ' + descriptor.value
		print indent( level + 1 ) + '---'

	print indent( level ) + 'Components are:'
	print indent( level + 1 ) + '---'
	for name, value in registry.facilities.iteritems():
		print indent( level + 1 ) + 'name: ' + name
		print indent( level + 1 ) + 'value: {'
		_printRegistry( value, level + 2 )
		print indent( level + 1 ) + '}'
		print indent( level + 1 ) + '---'


def printRegistry( application, level = 0 ):
	print indent( level ) + 'Input Registry:'
	_printRegistry( application.registry, level + 1 )


def _printInventory( inventory, level ):
	print indent( level ) + 'Properties are:'
	print indent( level + 1 ) + '---'
	for name, _value in inventory._traitRegistry.iteritems():
		print indent( level + 1 ) + 'name: ' + name
		# The raw value itself is stored on the inventory. _value is
		# the pyre property of the value.
		print indent( level + 1 ) + 'type: ' + _value.type
		print indent( level + 1 ) + 'value: ', _value.__get__(inventory)
		print indent( level + 1 ) + '---'

	print indent( level ) + 'Components are:'
	print indent( level + 1 ) + '---'
	for name, _value in inventory._facilityRegistry.iteritems():
		print indent( level + 1 ) + 'facility name: ' + name

		# The raw value itself (i.e. the component) is stored on the
		# inventory. _value is the facility of the value.
		value = inventory.__getattribute__( name )
		print indent( level + 1 ) + 'component name: ' + value.name
		print indent( level + 1 ) + 'value: {'
		_printInventory( value.inventory, level + 2 )
		print indent( level + 1 ) + '}'
		print indent( level + 1 ) + '---'


def printInventory( application, level = 0 ):
	print indent( level ) + 'Inventory:'
	_printInventory( application.inventory, level + 1 )
