#!/bin/env python

from math import pi,sin,sqrt,fabs

dumpEvery=100 # Determine the output interval: Ex. 100 = Write every 100 th.

L=1.0            # Domain size
vel=-1.0e-05     # Driving velocity
dt=1.0
Ndata=int(0.03/(dt*fabs(vel)/L)) # Number of time steps = Target total strain / strain increment.

lamb=2.0e+08/3.0 # Lame's constants
mu=2.0e+08

phi=10.0*pi/180.0 # friction angle
psi=10.0*pi/180.0 # dilation angle
cohesion=1.0e+06  # cohesion

# derived parameters
Nphi=(1.0+sin(phi))/(1.0-sin(phi))
Npsi=(1.0+sin(psi))/(1.0-sin(psi))
Nc=2.0*cohesion*sqrt(Nphi)

# Initialize some variables.
t=0.0
Ex=0.0
Eex=0.0
Epx=0.0
dEx=0.0
dEex=0.0
Sx=0.0
Sy=0.0
Sz=0.0

# Prepare an output file
fo=open("analytic_soln.dat","w")

# Write the zeroth data point
print >> fo, 0.0, 0.0

# Start time loop
for i in range(1,Ndata):
	t = dt*i # total elapsed time. Note that dt is constant.

	#dEx=vel*dt/(L-vel*t)
	dEx=vel*dt/L # Increment of the total strain
	dEpx=0.0     # Initialize the increment of plastic strain
	la1 = 0.0    # Lambda1, the consistency parameter.

	fs=Sx-Sy*Nphi+Nc # Yield function. Yielding declared if fs < 0.0.
	if fs <= 0:
		la1 = ((lamb+2.0*mu-lamb*Nphi)*dEx)/(2.0*(lamb+2.0*mu)-2.0*lamb*(Nphi+Npsi)+2.0*(lamb+mu)*Nphi*Npsi)
		dEpx = 2.0*la1 # Plastic strain increment.
#		print la1, dEpx, dEx

	dEex = dEx - dEpx  # Get the increment of elastic strain

	# Stress increment
	ds_x=(lamb+2.0*mu)*dEex+2.0*lamb*la1*Npsi
	ds_y=(lamb+2.0*mu)*la1*Npsi+lamb*(dEex+la1*Npsi)
	ds_z=ds_y

	# Update accumulated strain, plastic strain, and stresses.
	Epx = Epx + dEpx
	Ex = Ex+dEx
	Sx = Sx+ds_x
	Sy = Sy+ds_y
	Sz = Sz+ds_z

#	print fs,Sx,Sy,Sz,ds_x,ds_y,ds_z
#	pressure=(Sx+Sy+Sz)/3.0

	# Write total strain and stress.
	if (i-1) % dumpEvery == 0:
		print >> fo, -1.0*Ex, -1.0*Sx

fo.close()

