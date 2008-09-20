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
*/
/** \file
**  Role:
**	Random number generator.
**
** Assumptions:
**	This is not thread safe (seed is a global variable).
**
** Comments:
**	This is an implementation of Park and Miller, "Random Number Generators: Good ones are hard to find", Communications of the
**		ACM, October 1988, Volume 31, No 10, pages 1192-1201. Also reference is Numerical Methods, Second Edition, and the
**		GNU Scientific Library.
**	
**	Park and Miller "minimul standard" algorithm:  maskedSeed_j+1 = (a * maskedSeed_j) mod randMax,
**		but the product of a and m-1 is the greater than the size of int, hence Schrange's algorithm is used.
**	Schrage's algorithm applied: (a * maskedSeed_j) mod randMax = a * (maskedSeed_j mod q) - r * [maskedSeed_j / q],
**		 and where the answer is < 0, add randMax.
**	The 'mod' is equated with the algorithm: maskedSeed_j mod q = maskedSeed_j - [maskedSeed_j / q] * q,
**		hence Schrage's application becomes: 
**			tmp = [maskedSeed_j / q]
**			a * (maskedSeed_j - tmp * q) - r * tmp
**	
**	The seed cannot be the mask value (123459876), else an assertion is thrown.
**
** $Id: Random.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Swarm_Random_h__
#define __Discretisation_Swarm_Random_h__
	
	/* Maximum random number */
	extern const long int Swarm_Random_RandMax;
	
	/* Seed */
	void Swarm_Random_Seed( long seed );
	
	/* Random */
	double Swarm_Random_Random();
	double Swarm_Random_Random_WithMinMax( double min, double max ) ;
	
#endif /* __Discretisation_Swarm_Random_h__ */
