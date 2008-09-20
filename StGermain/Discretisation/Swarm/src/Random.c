/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003, Victorian Partnership for Advanced Computing (VPAC) Ltd, 110 Victoria Street, Melbourne, 3053, Australia.
**
** Authors:
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**	Patrick D. Sunter, Software Engineer, VPAC. (pds@vpac.org)
**	Luke J. Hodkinson, Computational Engineer, VPAC. (lhodkins@vpac.org)
**	Siew-Ching Tan, Software Engineer, VPAC. (siew@vpac.org)
**	Alan H. Lo, Computational Engineer, VPAC. (alan@vpac.org)
**	Raquibul Hassan, Computational Engineer, VPAC. (raq@vpac.org)
**
**  This library is free software; you can redistribute it and/or
**  modify it under the terms of the GNU Lesser General Public
**  License as published by the Free Software Foundation; either
**  version 2.1 of the License, or (at your option) any later version.
**
**  This library is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**  Lesser General Public License for more details.
**
**  You should have received a copy of the GNU Lesser General Public
**  License along with this library; if not, write to the Free Software
**  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
**
** $Id: Random.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"

#include "types.h"
#include "shortcuts.h"
#include "Random.h"

#include <assert.h>

/* Park and Miller "minimul standard" constants */
const long int Swarm_Random_RandMax = 2147483647;	/* Swarm_Random_RandMax = (2^31)-1, taken from max int */
static const long int a = 16807;			/* a = 7^5, taken from algorithm */
static const unsigned long int mask = 123459876;	/* taken from algorithm */

/* Schrage's algorithm constants */
static const long int q = 127773;			/* q = [Swarm_Random_RandMax / a] */
static const long int r = 2836;				/* r = m mod a */

/* Park and Miller "minimul standard" variables */
static long Swarm_Random_maskedSeed;


void Swarm_Random_Seed( long seed ) {
	assert( mask - seed != 0 );
	Swarm_Random_maskedSeed = seed ^ mask;
}


double Swarm_Random_Random() {
	const long int tmp = Swarm_Random_maskedSeed / q;
	Swarm_Random_maskedSeed = a * (Swarm_Random_maskedSeed - tmp * q) - r * tmp;
	if( Swarm_Random_maskedSeed < 0 ) {
		Swarm_Random_maskedSeed += Swarm_Random_RandMax;
	}
	return Swarm_Random_maskedSeed;
}

double Swarm_Random_Random_WithMinMax( double min, double max ) {
	return ( max - min ) * Swarm_Random_Random() / (double)Swarm_Random_RandMax + min;
}
