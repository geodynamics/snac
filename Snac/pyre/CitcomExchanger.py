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

class CitcomExchanger(Exchanger):


	def __init__(self, name, facility):
		Exchanger.__init__(self, name, facility)
		self.cge_t = 0
		self.fge_t = 0
		self.toApplyBC = True

		# exchanged information is dimensional
		self.inventory.dimensional = True
		self.inventory.transformational = True

		return


	def initialize(self, solver):
		Exchanger.initialize(self, solver)
		# restart and use temperautre field of previous run?
		self.restart = solver.inventory.ic.inventory.restart
		if self.restart:
			self.ic_initTemperature = solver.inventory.ic.initTemperature()

		self.all_variables = solver.all_variables
##	import CitcomS.Exchanger
##	self.module = CitcomS.Exchanger

		# create velocity sources (boundary)
		self.velo_boundary = range(self.numSrc)
		self.source["VBC"] = range(self.numSrc)
		self.VBC = range(self.numSrc)

		self.module.initConvertor(self.inventory.dimensional,
								  self.inventory.transformational,
								  self.all_variables)

		return

	def launch(self, solver):
		Exchanger.launch(self,solver)

		return

	def createMesh(self):
		dimensional = self.inventory.dimensional
		self.globalBBox = self.module.createGlobalBoundedBox(self.all_variables)
		self.remoteBBox = self.module.exchangeBoundedBox( self.globalBBox,
														  self.communicator.handle(),
														  self.srcComm[0].handle(),
														  self.srcComm[0].size - 1)
		# create an Interior instance for temperature sink.
		self.temp_interior, self.myBBox = self.module.createInterior( self.remoteBBox,
																 self.all_variables)

		# create an Boundary instance for stress sink.
		self.stress_boundary, self.myBBox = self.module.createBoundary( self.all_variables,
																	  self.inventory.excludeTop,
																	  self.inventory.excludeBottom)
		# create Boundary instances for velocity sources.
		for i in range(len(self.velo_boundary)):
			self.velo_boundary[i] = self.module.createEmptyBoundary()

		return


	def createSourceSink(self):
		self.createSource()
		self.createSink()
		return


	def createSource(self):
		for i, comm, b in zip(range(self.numSrc),
							  self.srcComm,
							  self.velo_boundary):
			# sink is always in the last rank of a communicator
			sinkRank = comm.size - 1
			self.source["VBC"][i] = self.module.CitcomSource_create(comm.handle(),
																   sinkRank,
																   b,
																   self.myBBox,
																   self.all_variables,)
		return


	def createSink(self):
		self.sink["TIntr"] = self.module.Sink_create(self.sinkComm.handle(),
													self.numSrc,
													self.temp_interior)
		self.sink["SBC"] = self.module.Sink_create(self.sinkComm.handle(),
												  self.numSrc,
												  self.stress_boundary)
		return


	def createBC(self):
		import CitcomS.Components.Exchanger.Outlet as Outlet
		for i, src in zip(range(self.numSrc),
						  self.source["VBC"]):
			self.VBC[i] = Outlet.VOutlet(src,
										 self.all_variables)

		import CitcomS.Components.Exchanger.Inlet as Inlet
		self.SBC = Inlet.SInlet(self.stress_boundary,
									self.sink["SBC"],
									self.all_variables)
		return


	def createII(self):
		import CitcomS.Components.Exchanger.Inlet as Inlet
		self.TII = Inlet.TInlet(self.temp_interior,
							   self.sink["TIntr"],
							   self.all_variables)
		return


	def initTemperature(self):
		self.module.initTemperature(self.remoteBBox, self.all_variables)
        # send temperture field to FGE
        #self.module.sendTemperature(self.exchanger)
		return

	#    def restartTemperature(self):
	#        interior = range(self.numSrc)
	#        source = range(self.numSrc)
	#
	#        for i in range(len(interior)):
	#            interior[i] = self.module.createEmptyInterior()
	#
	#        for i, comm, b in zip(range(self.numSrc),
	#                              self.srcComm,
	#                              interior):
	#            # sink is always in the last rank of a communicator
	#            sinkRank = comm.size - 1
	#            source[i] = self.module.createSource(comm.handle(),
	#                                                 sinkRank,
	#                                                 b,
	#                                                 self.all_variables,
	#                                                 self.myBBox)
	#
	#        import Outlet
	#        for i, src in zip(range(self.numSrc), source):
	#            outlet = Outlet.VTOutlet(src, self.all_variables, "t")
	#            outlet.send()
	#
	# Any modification of read-in temperature is done here
	# Note: modifyT is called after sending unmodified T to FGE.
	# If T is modified before sending, FGE's T will lose sharp feature.
	# FGE has to call modifyT too to ensure consistent T field.
	#        self.modifyT(self.remoteBBox)
	#
	#        return




	def postVSolverRun(self):
		self.applyBoundaryConditions()
		return


	def NewStep(self):
		# receive temperature field from FGE (Snac)
		self.recv_impose_Interior()
		self.recv_impose_Boundary()
		return


	def recv_impose_Interior(self):
		self.TII.recv()
		self.TII.impose()
		return


	def recv_impose_Boundary(self):
		self.SBC.recv()
		self.SBC.impose()
		return


	def applyBoundaryConditions(self):
		for bc in self.VBC:
			bc.send()

		return


	def stableTimestep(self, dt):
		dimensional = self.inventory.dimensional
		new_dt = self.module.exchangeTimestep(dt,
											  self.communicator.handle(),
											  self.srcComm[0].handle(),
											  self.srcComm[0].size - 1)

##		print "%s - old dt = %g   exchanged dt = %g" % (
##			self.__class__, dt, new_dt)

		self.updateCouplingInfo()
		return dt


	def updateCouplingInfo(self):
		self.createMesh()
		self.createSourceSink()
		self.createBC()
		self.createII()
		return


	def exchangeSignal(self, signal):
		newsgnl = self.module.exchangeSignal(signal,
											 self.communicator.handle(),
											 self.srcComm[0].handle(),
											 self.srcComm[0].size - 1)
		return newsgnl



	class Inventory(Exchanger.Inventory):

		import pyre.inventory

		excludeTop = pyre.inventory.bool("excludeTop", default=False)
		excludeBottom = pyre.inventory.bool("excludeBottom", default=False)
		incompressibility = pyre.inventory.bool("incompressibility", default=True)



# version
__id__ = "$Id: CitcomExchanger.py 3240 2006-07-20 21:11:57Z EunseoChoi $"

# End of file
