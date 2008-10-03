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
##	The pyre representation of StGermain solver.
##
## Assumptions:
##	None as yet.
##
## Comments:
##	Based off CitcomS' implementation.
##
## $Id: Solver.py 3872 2006-10-16 14:12:16Z SteveQuenette $
##
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

def solver(name='solver', facility='solver'):
	return Solver(name, facility)

from pyre.simulations.Solver import Solver as BaseSolver
import journal,sys

from StGermain.Base.IO.Dictionary import Dictionary

class Solver( BaseSolver ):

	def __init__( self, name="solver", facility="solver" ):
		BaseSolver.__init__( self, name, facility )

		# StGermain stuff
		self._context = None
		self.communicator = None
		self._dictionary = None
		self.dt_prev = 0.0

		# Coupler stuff
		self.coupler = None
		self.exchanger = None
		self.myPlus = []
		self.remotePlus = []

		self.start_cpu_time = 0
		self.cpu_time = 0
		self.model_time = 0
		self.fptime = None
		return

	def initialize( self, application ):
		BaseSolver.initialize( self, application )

		# StGermain stuff
		self._dictionary = application._dictionary
		self.communicator = application.solverCommunicator
		if not application._context:
			application._context = application.BuildContext()
		self._context = application._context

		# StGermain build... required now... as in the coupled case we control the coupling-boundary and it needs to be
		# prepared first. Also The geometry needs to be set.
		self._context.Construct()
		self._context.Build()
		self._context.Initialise()

		# Coupler stuff
		try:
			application.inventory.coupler
		except AttributeError:
			pass
		else:
			self.myPlus = application.myPlus
			self.remotePlus = application.remotePlus
			self.exchanger = application.exchanger
			self.coupler = application.inventory.coupler
			if self.coupler:
				self.coupler.initialize( self )
		return

	def launch( self, application ):
		BaseSolver.launch( self, application )
		# Coupler stuff
		if self.coupler:
			self.coupler.launch( self )
		return

	# What does this function do?
	def newStep( self, t, step ):
		BaseSolver.newStep( self, t, step )

		# Coupler stuff
		if self.coupler:
			self.coupler.newStep()
		return

	def stableTimestep( self ):
		dt = self._context.Dt()
		if self.dt_prev > dt:
			dt = self.dt_prev
		else:
			self.dt_prev = dt

		# Coupler stuff
		if self.coupler:
			# negotiate with other solver(s)
			dt = self.coupler.stableTimestep( dt )

		return BaseSolver.stableTimestep( self, dt )

	def advance( self, dt ):
		BaseSolver.advance( self, dt )

		# For imposing Force Boundary condition in coupled case
		if self.coupler:
		    self.coupler.preVSolverRun()

		# StGermain stuff
		self._context.Step( dt )

		return

	# What does this function do?
	def endTimestep( self, t, steps, done ):
		BaseSolver.endTimestep( self, t )

		# Coupler stuff
		if self.coupler:
			done = self.coupler.endTimestep( steps, done )

		return done

	def endSimulation( self, step ):
		BaseSolver.endSimulation( self, step, self.t )

		# StGermain stuff
		self._context.Stg_Class_Delete()

		return

	def save(self,step):
		return

	class Inventory( BaseSolver.Inventory ):
		import pyre.inventory

