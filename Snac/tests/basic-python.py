#!/usr/bin/env mpipython

from StGermain import initialise as Initialise
import StGermain.Base.IO.Dictionary
from StGermain.Base.IO.Dictionary import Dictionary
from Snac.Context import Context
import StGermain.Bindings.MPI as MPI

Initialise()

d = Dictionary( StGermain.Base.IO.Dictionary.new() )
d.LoadFromFile( "basic.xml" )
context = Context( d, MPI.COMM_WORLD() )
context.Construct()
context.Build()
context.Initialise()
context.Execute()
context.Destroy()
