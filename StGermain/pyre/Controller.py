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
## Role:
##	The pyre interface to control a StGermain application.
##
## Assumptions:
##	None as yet.
##
## Comments:
##	Based off CitcomS' implementation.
##
## $Id: Controller.py 3698 2006-07-20 21:09:56Z EunseoChoi $
##
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

def controller(name="controller", facility="controller"):
    return Controller(name, facility)

from pyre.simulations.SimulationController import SimulationController
import journal
import sys

class Controller( SimulationController ):


	def __init__( self, name, facility ):
		SimulationController.__init__( self, name, facility )

		self.step = 0
		self.clock = 0.0
		self._context = None
		self.done = False
		self.solver = None
		return


	def initialize( self, application ):
		self._context = application.getContext()
		self.solver = application.solver
		self.solver.initialize( application )
		return


	def launch( self, application ):
		# 0th step
		self.solver.launch( application )

		# do io for 0th step
		self.save()
		return
	'''
	def setClock( self, time ):
		if self._context:
			self._context.time = time
		self.__clock = time

	def getClock( self ):
		return self.__clock

	clock = property( getClock, setClock, doc="Current sim clock time." )

	def setStep( self, step ):
		if self._context:
			self._context.step = step
		self.__step = step

	def getStep( self ):
		return self.__step

	step = property( getStep, setStep, doc="Current sim step." )
	'''


	def march( self, totalTime = 0, steps = 0 ):
		"""explicit time loop"""

		self.step = 1

		if( steps == 0 and totalTime == 0 ):
			self.endSimulation()
			return

		while 1:
			# notify solvers we are starting a new timestep
			self.startTimestep()

			# compute an acceptable timestep
			dt = self.stableTimestep()

			# advance
			self.advance( dt )

			# update smulation clock and step number
			self.clock += dt
			self.step += 1

			# notify solver we finished a timestep
			self.endTimestep( totalTime, steps )

			# do io
			self.save()

			# are we done?
			if self.done:
				break

		# Notify solver we are done
		self.endSimulation()

		return

	def endTimestep( self, totalTime, steps ):
		# are we done?
		if steps and self.step >= steps:
			self.done = True
		if totalTime and self.clock >= totalTime:
			self.done = True

		# solver can terminate time marching by returning True
		self.done = self.solver.endTimestep( self.clock, self.step, self.done )
		return

	def endSimulation( self ):
		self.solver.endSimulation( self.step )
		return

	def save( self ):
		#TODO: Controller::save
		#if not ( step % self.inventory.monitoringFrequency ):
		self.solver.save( self.step )
		return
