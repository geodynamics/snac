#!/usr/bin/env mpipython.exe

import StGermain
StGermain.initialise()
# We have to init here because a default to the inventory in snac requires construction of StGermain things... the only way to
# fix this is to upgrade to pythia-0.8, which will mean citcom and exchanger needs to too.

import Snac.pyre.Application

a = Snac.pyre.Application.Application()
a.run()
