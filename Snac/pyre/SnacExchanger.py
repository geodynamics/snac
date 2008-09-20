#!/usr/bin/env python
#
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#
# <LicenseText>
#
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#

import CitcomS.Components.Exchanger as exchanger
Exchanger=exchanger.exchanger()
import sys

class SnacExchanger(Exchanger):


	def __init__(self, name, facility):
		Exchanger.__init__(self, name, facility)
		self.cge_t = 0
		self.fge_t = 0
		self.toApplyBC = True

		# exchanged information is non-dimensional
		self.inventory.dimensional = False
		self.inventory.transformational = False

		return


	def initialize(self, solver):
		Exchanger.initialize(self, solver)

		import Snac.pyre.Exchanger
		self.module = Snac.pyre.Exchanger

		# create temperature sources (interior).
		self.temp_interior = range(self.numSrc)
		self.source["TIntr"] = range(self.numSrc)
		self.TII = range(self.numSrc)

		# create stress sources (boundary).
		self.stress_boundary = range(self.numSrc)
		self.source["SBC"] = range(self.numSrc)
		self.SBC = range(self.numSrc)

		self._context=solver._context
		self.all_variables = self._context.handle()

		self.module.initConvertor(self.inventory.dimensional,
								  self.inventory.transformational,
								  self.all_variables)

		return


	def launch(self, solver):
		Exchanger.launch(self,solver)
		self.VBC.recv()
		self.VBC.impose()
		return

	def createMesh(self):
		self.globalBBox = self.module.createGlobalBoundedBox(self.all_variables)

		mycomm = self.communicator
		self.remoteBBox = self.module.exchangeBoundedBox(self.globalBBox,
														 mycomm.handle(),
														 self.sinkComm.handle(),
														 0)
		# create a Boundary instance for velocity sink.
		self.velo_boundary, self.myBBox = self.module.createBoundary(self.all_variables)

		# create Boundary/Interior instances for temperature and stress sources, respectively.
		for i in range(len(self.temp_interior)):
			self.temp_interior[i] = self.module.createEmptyInterior()
			self.stress_boundary[i] = self.module.createEmptyBoundary()

		return


	def createSourceSink(self):
		self.createSink()
		self.createSource()
		return


	def createSink(self):
		self.sink["VBC"] = self.module.Sink_create(self.sinkComm.handle(),
												  self.numSrc,
												  self.velo_boundary)
		return


	def createSource(self):
		for i, comm, b in zip(range(self.numSrc),
							  self.srcComm,
							  self.temp_interior):
			# sink is always in the last rank of a communicator
			sinkRank = comm.size - 1
			self.source["TIntr"][i] = self.module.SnacSource_create( comm.handle(),
																	sinkRank,
																	b,
																	self.myBBox,
																	self.all_variables)

		for i, comm, b in zip(range(self.numSrc),
							  self.srcComm,
							  self.stress_boundary):
			# sink is always in the last rank of a communicator
			sinkRank = comm.size - 1
			self.source["SBC"][i] = self.module.SnacSource_create( comm.handle(),
																  sinkRank,
																  b,
																  self.myBBox,
																  self.all_variables)

		return


	def createBC(self):
		import Inlet
		self.VBC = Inlet.VInlet(self.velo_boundary,
							   self.sink["VBC"],
							   self.all_variables)

		import Outlet
		for i, src in zip(range(self.numSrc),
						  self.source["SBC"]):
			self.SBC[i] = Outlet.SOutlet(src,
											self.all_variables)

		return


	def createII(self):
		import Outlet
		for i, src in zip(range(self.numSrc),
						  self.source["TIntr"]):
			self.TII[i] = Outlet.TOutlet(src,
										self.all_variables)
		return


	def initTemperature(self):
		return


	def preVSolverRun(self):
		self.applyBoundaryConditions()
		return


	def NewStep(self):
		if self.catchup:
			# send temperture field to CGE
			self.send_Interior()
			# send stress field to CGE
			self.send_Boundary()
		return


	def send_Interior(self):
		for ii in self.TII:
			ii.send()
		return


	def send_Boundary(self):
		for bc in self.SBC:
			bc.send()
		return


	def applyBoundaryConditions(self):
		if self.toApplyBC:
			self.VBC.recv()
			self.toApplyBC = False

		self.VBC.impose()

		# applyBC only when previous step is a catchup step
		if self.catchup:
			self.toApplyBC = True

		return


	def stableTimestep(self, dt):
		if self.catchup:
			mycomm = self.communicator
			self.cge_t = self.module.exchangeTimestep(dt,
													  mycomm.handle(),
													  self.sinkComm.handle(),
													  0)
			self.fge_t = 0
			self.catchup = False
			self.updateCouplingInfo()

		self.fge_t += dt
		old_dt = dt

		if self.fge_t >= self.cge_t:
			dt = dt - (self.fge_t - self.cge_t)
			self.fge_t = self.cge_t
			self.catchup = True
			# print "FGE: CATCHUP!"

		# store timestep for interpolating boundary velocities
		self.VBC.storeTimestep(self.fge_t, self.cge_t)

##		if self.communicator.rank==0:
##			print "%s - old dt = %g   exchanged dt = %g" % (
##				self.__class__, old_dt, dt)
##			print "cge_t = %g  fge_t = %g" % (self.cge_t, self.fge_t)

		return dt


	def updateCouplingInfo(self):
		self.createMesh()
		self.createSourceSink()
		self.updateBC()
		self.createII()
		return


	def updateBC(self):
		self.VBC.storeVold()
		self.createBC()
		self.VBC.readVold()

		return


	def exchangeSignal(self, signal):
		mycomm = self.communicator
		newsgnl = self.module.exchangeSignal(signal,
											 mycomm.handle(),
											 self.sinkComm.handle(),
											 0)
		return newsgnl



	class Inventory(Exchanger.Inventory):

		import pyre.inventory


# version
__id__ = "$Id: SnacExchanger.py 3240 2006-07-20 21:11:57Z EunseoChoi $"

# End of file
