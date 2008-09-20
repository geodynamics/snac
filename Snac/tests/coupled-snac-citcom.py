#!/usr/bin/env mpipython.exe
#
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#
#  <LicenseText>
#
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#


import StGermain
StGermain.initialise()

from Snac.pyre.CoupledApp import CoupledApp


# main
if __name__ == "__main__":
	import journal

	journal.debug("Exchanger").activate()
	journal.info("  X").activate()
	journal.info("  proc").activate()
	journal.info("  bid").activate()

	app = CoupledApp("app")
	app.main()


# version
__id__ = "$Id: coupled-snac-citcom.py 2960 2005-05-19 23:57:11Z EunseoChoi $"

#  End of file
