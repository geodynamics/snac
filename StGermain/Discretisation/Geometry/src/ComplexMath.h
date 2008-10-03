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
**	Robert B. Turnbull, Monash Cluster Computing. (Robert.Turnbull@sci.monash.edu.au)
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
** Role:
**    Provides basic vector operations.
**
** Assumptions:
**
** Comments:
**
** $Id: ComplexMath.h 3735 2006-08-01 08:05:14Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Geometry_ComplexMath_h__
#define __Discretisation_Geometry_ComplexMath_h__

#include "units.h"

/** Define names for index into complex numbers. 
Which makes it clear what index it refers to.*/
#define REAL_PART 0
/** see comments for REAL_PART */
#define IMAG_PART 1

void Cmplx_Add( Cmplx X, Cmplx Y, Cmplx destination ) ;
void Cmplx_Subtract( Cmplx X, Cmplx Y, Cmplx destination ) ;
void Cmplx_Multiply( Cmplx X, Cmplx Y, Cmplx destination ) ;
void Cmplx_Division( Cmplx X, Cmplx Y, Cmplx destination ) ;

void Cmplx_AddReal( Cmplx X, double real, Cmplx destination ) ;
void Cmplx_RealMinusCmplx( Cmplx X, double real, Cmplx destination ) ;
void Cmplx_RealMultiply( Cmplx X, double real, Cmplx destination ) ;
void Cmplx_RealDivideByCmplx( Cmplx X, double real, Cmplx destination ) ;
void Cmplx_Exp( Cmplx X, Cmplx destination );
void Cmplx_Copy( Cmplx X, Cmplx destination );
void Cmplx_Zero( Cmplx X ) ;

void Cmplx_Conjugate( Cmplx X, Cmplx conjugate ) ;

double Cmplx_Argument( Cmplx X ) ;
double Cmplx_Modulus( Cmplx X ) ;

void Cmplx_RealPower( Cmplx X, double power, Cmplx destination ) ;
void Cmplx_CmplxPower( Cmplx X, Cmplx power, Cmplx destination ) ;

/** Calculate the square root of a complex number */
#define Cmplx_Sqrt( self, destination ) \
	Cmplx_RealPower( self, 0.5, destination )

/** Print a complex number. 
Currently uses %.5g formatting */ 
#define Journal_PrintCmplx( stream, self ) \
		Journal_Printf( stream, #self " = %.5g %c %.5g i\n", (self)[ REAL_PART ], (self)[ IMAG_PART ] >= 0.0 ? '+' : '-', fabs( (self)[ IMAG_PART ] ) )

#endif
