#!/usr/bin/env mpipython.exe

import StGermain
StGermain.initialise()

# We have to init here because a default to the inventory in snac requires construction of StGermain things... the only way to
# fix this is to upgrade to pythia-0.8, which will mean citcom and exchanger needs to too.

from Snac.pyre.Application import Application
import StGermain.pyre.Debug as Debug


a = Application()

## a.run()
## first bits of a.run()

# build storage for the user input
registry = a.createRegistry()
a.registry = registry
# command line
help, a.argv = a.processCommandline( registry )
# curator
curator = a.createCurator()
a.initializeCurator(curator, registry)
# look for my settings
a.initializeConfiguration()
# try to collect input from other sources
a.collectUserInput(registry)
# update user options from the command line
a.updateConfiguration(registry)
# transfer user input to my inventory
unknownProperties, unknownComponents = a.applyConfiguration()
# initialize the trait cascade
a.init()
# print a startup page
a.generateBanner()
# the main application behavior
if help:
	a.help()
elif a._showHelpOnly:
	pass
elif a.verifyConfiguration(unknownProperties, unknownComponents, a.inventory.typos):
	a.initialize()
	a.reportConfiguration()
	Debug.printRegistry( a )
#	Debug.printInventory( a )

'''
a.defaults()
print "(0)"
registry = a.prime()
print "(1)"
a._registry.unknown = a.configure( registry )
print "(2)"

Debug.printRegistry( a )
Debug.printInventory( a )
'''
