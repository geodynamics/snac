/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003,
**	Steve Quenette, 110 Victoria Street, Melbourne, Victoria, 3053, Australia.
**	Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
**	University of Texas, 1 University Station, Austin, Texas, 78712, USA.
**
** Authors:
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**	Stevan M. Quenette, Visitor in Geophysics, Caltech.
**	Luc Lavier, Research Scientist, The University of Texas. (luc@utig.ug.utexas.edu)
**	Luc Lavier, Research Scientist, Caltech.
**
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2, or (at your option) any
** later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**
** $Id: Tetrahedra.c 2227 1970-01-02 14:47:05Z SteveQuenette $
**
** NOTE: Because of "static" used in functions... not thread safe... i.e. only one instance of Snac can run in any one executable.
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>

#include "units.h"
#include "types.h"
#include "shortcuts.h"
#include "Tetrahedra.h"
#include <math.h>

Volume Tetrahedra_Volume( Coord coord1, Coord coord2, Coord coord3, Coord coord4 ) {
	static Volume volume;
	static double x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;

	x1 = coord1[0]-coord1[0];
	y1 = coord1[1]-coord1[1];
	z1 = coord1[2]-coord1[2];

	x2 = coord2[0]-coord1[0];
	y2 = coord2[1]-coord1[1];
	z2 = coord2[2]-coord1[2];

	x3 = coord3[0]-coord1[0];
	y3 = coord3[1]-coord1[1];
	z3 = coord3[2]-coord1[2];

	x4 = coord4[0]-coord1[0];
	y4 = coord4[1]-coord1[1];
	z4 = coord4[2]-coord1[2];

        volume = (x4*(y2*z3-z2*y3)+y4*(z2*x3-x2*z3)+z4*(x2*y3-x3*y2))/6.0;

	return fabs(volume);
}


Area Tetrahedra_SurfaceArea( Coord coord1, Coord coord2, Coord coord3 ) {
	static Area area;
	static double a, b, c;
	static double x2, y2, z2, x3, y3, z3;

	x2 = coord2[0]-coord1[0];
	y2 = coord2[1]-coord1[1];
	z2 = coord2[2]-coord1[2];

	x3 = coord3[0]-coord1[0];
	y3 = coord3[1]-coord1[1];
	z3 = coord3[2]-coord1[2];

	a=y2*z3-z2*y3;
	b=z2*x3-x2*z3;
	c=x2*y3-y2*x3;
	area = sqrt(a*a+b*b+c*c)/2.0f;

	return area;
}


void Tetrahedra_SurfaceNormal( Coord coord1, Coord coord2, Coord coord3, Normal* normal ) {
	static double ax, ay, az, aa;
	static double x2, y2, z2, x3, y3, z3;

	x2 = coord2[0]-coord1[0];
	y2 = coord2[1]-coord1[1];
	z2 = coord2[2]-coord1[2];

	x3 = coord3[0]-coord1[0];
	y3 = coord3[1]-coord1[1];
	z3 = coord3[2]-coord1[2];

        ax = y2*z3-z2*y3;
        ay = z2*x3-x2*z3;
        az = x2*y3-y2*x3;

	aa = 1.*sqrt(ax*ax+ay*ay+az*az);

        (*normal)[0] = +1.0f * ax/aa;
	(*normal)[1] = +1.0f * ay/aa;
	(*normal)[2] = +1.0f * az/aa;
}
