#!/usr/bin/env mpipython

import os.path
import sys

import StGermain.Dummy.Dictionary as Dictionary
import StGermain.Dummy.Bindings.MPI as MPI
import Snac.Context as Context

# load MPI
commWorld = MPI.COMM_WORLD()

# create the dictionary and read in data
if len( sys.argv ) >= 2:
	inputFile = sys.argv[1]
else: 
	inputFile = "input.xml"

dict = Dictionary.Dictionary()
dict.LoadFromFile( inputFile )

# create the Snark context
context = Context.Context( dict, commWorld )
dict.Print()

context.Build()
context.Init()

# TODO: "unpack" this so that users can add stuff more easily
context.Run()
