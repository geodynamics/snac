#!/usr/bin/env python
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
##
## Copyright (C), 2003,
##	Steve Quenette, 110 Victoria Street, Melbourne, Victoria, 3053, Australia.
##	Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
##	University of Texas, 1 University Station, Austin, Texas, 78712, USA.
##
## Authors:
##	Eh Tan, ?, Caltech (tan2@gps.caltech.edu)
##	Pururav Thoutireddy, ?, Caltech (puru@cacr.caltech.edu)
##	Eun-seo Choi, ?, Caltech (ces74@gps.caltech.edu)
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
## $Id: CoupledApp.py 3240 2006-07-20 21:11:57Z EunseoChoi $
##
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

import journal
import Snac.pyre.Application

SnacApplication = Snac.pyre.Application.Application

class CoupledApp( SnacApplication ):
	def __init__( self, name = "snaccitcom" ):
		SnacApplication.__init__( self, name )

		self.solverCommunicator = None
		self.myPlus = []
		self.remotePlus = []

		self._info = journal.debug( "application" )
		return


	def findLayout( self, layout ):
		if layout.coarse:
			self.inventory.solver = self.inventory.coarse
			self.inventory.controller = self.inventory.coarseController
			self.inventory.exchanger = self.inventory.cge
			self.solverCommunicator = layout.coarse #self._CommWorld
			self.myPlus = layout.coarsePlus
			self.remotePlus = layout.finePlus
		elif layout.fine:
			self.inventory.solver = self.inventory.fine
			self.inventory.controller = self.inventory.fineController
			self.inventory.exchanger = self.inventory.fge
			self.solverCommunicator = layout.fine #layout
			self.myPlus = layout.finePlus
			self.remotePlus = layout.coarsePlus
		else:
			import journal
			journal.firewall( self.name ).log( "node '%d' is an orphan" % layout.rank )

		self.solver = self.inventory.solver
		self.controller = self.inventory.controller
		self.exchanger = self.inventory.exchanger
		return

	def reportConfiguration( self ):

		rank = self.comm.rank
		if rank != 0:
			return

		SnacApplication.reportConfiguration()
		self._info.line( "    coarse: %r" % self.inventory.coarse.name )
		self._info.line( "    fine: %r" % self.inventory.fine.name )
		self._info.line( "    cge: %r" % self.inventory.cge.name )
		self._info.line( "    fge: %r" % self.inventory.fge.name )
		self._info.line( "    coupler: %r" % self.inventory.coupler.name )
		return

	class Inventory( SnacApplication.Inventory ):
		import pyre.inventory

		import CitcomS.Solver as CitcomSolver

		import CitcomS.Controller as CitcomController
		import StGermain.pyre.Controller as SnacController
		import CitcomS.Solver as CitcomSolver
		import StGermain.pyre.Solver as SnacSolver
		import Snac.pyre.SnacCoupler as Coupler

		#import CitcomS.Layout as Layout
		import StGermain.pyre.Layout as Layout

		from Snac.pyre.CitcomExchanger import CitcomExchanger
		from Snac.pyre.SnacExchanger import SnacExchanger

		coarseController = pyre.inventory.facility( name = "coarseController", default = CitcomController.controller("coarseController","controller") )
		fineController = pyre.inventory.facility( name = "fineController", default = SnacController.controller("fineController","controller") )
		coupler = pyre.inventory.facility( name = "coupler", default = Coupler.coupler())
		layout = pyre.inventory.facility( name = "layout", default=Layout.Layout())
		coarse = pyre.inventory.facility( name = "coarse", default = CitcomSolver.regionalSolver( "coarse", "coarse" ))
		fine = pyre.inventory.facility( name = "fine", default = SnacSolver.Solver( "fine", "fine"))
		cge = pyre.inventory.facility( name = "cge", default = CitcomExchanger("cge","cge"))
		fge = pyre.inventory.facility( name = "fge", default = SnacExchanger("fge","fge"))

		steps = pyre.inventory.int( "steps", default = 1 ),
		inputFilename = pyre.inventory.str( "inputFilename", default = "./coupled-snac-citcom.xml" )
