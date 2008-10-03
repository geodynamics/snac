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
** $Id: ComplexMath.c 3733 2006-08-01 07:36:27Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "ComplexMath.h"
#include <math.h>
#include <string.h>

#define a 	self[ REAL_PART ]
#define b 	self[ IMAG_PART ]
#define c 	Y[ REAL_PART ]		
#define d 	Y[ IMAG_PART ]		

/**  
 * X = a + b . i, Y = c + d . i
 * X + Y = (a + b.i) + (c + d.i) = (a + c) + i(b + d) 
destination may be stored in either X or Y */
void Cmplx_Add( Cmplx self, Cmplx Y, Cmplx destination ) {
	destination[ REAL_PART ] = a + c;
	destination[ IMAG_PART ] = b + d;
}

/** 
X = a + b i, Y = c + d i
X - Y = (a + b.i) - (c + d.i) = (a - c) + i(b - d) 
destination may be stored in either X or Y */
void Cmplx_Subtract( Cmplx self, Cmplx Y, Cmplx destination ) {
	destination[ REAL_PART ] = a - c;
	destination[ IMAG_PART ] = b - d;
}

/** 
X = a + b . i, Y = c + d . i
X . Y = (a + b.i)(c + d.i) = (ac - bd) + i(ad + bc) 
destination may be stored in either X or Y */
void Cmplx_Multiply( Cmplx self, Cmplx Y, Cmplx destination ) {
	double real = a * c - b * d;
	double imag = a * d + b * c;
	destination[ REAL_PART ] = real;
	destination[ IMAG_PART ] = imag;
}

/** 
X = a + b i, Y = c + d i
X / Y = \frac{a + b i}{c + d i} = \frac{(ac + bd) + i(bc - ad)}{c^2 + d^2} 
destination may be stored in either X or Y */
void Cmplx_Division( Cmplx self, Cmplx Y, Cmplx destination ) {
	double denominator = c * c + d * d;
	double real = (a * c + b * d)/denominator;
	double imag = (b * c - a * d)/denominator;
	
	destination[ REAL_PART ] = real;
	destination[ IMAG_PART ] = imag;
}
/** Add real to complex number */
void Cmplx_AddReal( Cmplx self, double real, Cmplx destination ) {
	destination[ REAL_PART ] = real + a;
	destination[ IMAG_PART ] = b;
}
/** Subtract a complex number from a real number */
void Cmplx_RealMinusCmplx( Cmplx self, double real, Cmplx destination ) {
	destination[ REAL_PART ] = real - a;
	destination[ IMAG_PART ] = -b;
}

/**  
x(a + bi) = xa + xb i */
void Cmplx_RealMultiply( Cmplx self, double real, Cmplx destination ) {
	destination[ REAL_PART ] = real * a;
	destination[ IMAG_PART ] = real * b;
}

/** 
x/(a + bi) = x/(a^2 + b^2) * (a - bi) */
void Cmplx_RealDivideByCmplx( Cmplx self, double real, Cmplx destination ) {
	double factor = real/(a*a + b*b);
	destination[ REAL_PART ] = a * factor;
	destination[ IMAG_PART ] = -b * factor;
}

/** exp( a + bi ) = exp( a ) [ cos(b) + i sin(b) ] */
void Cmplx_Exp( Cmplx self, Cmplx destination ) {
	double realPart = self[ REAL_PART ];
	double imagPart = self[ IMAG_PART ];

	destination[ REAL_PART ] = exp( realPart ) * cos( imagPart );
	destination[ IMAG_PART ] = exp( realPart ) * sin( imagPart );
}
/** Copy complex value from one variable to another. 
This nicely replaces = sign for assignments */
void Cmplx_Copy( Cmplx X, Cmplx destination ) {
	memcpy( destination, X, sizeof(Cmplx) );
}

/** Sets a complex variable to zero */
void Cmplx_Zero( Cmplx X ) {
	memset( X, 0, sizeof(Cmplx) );
}

/**
X = a + b i
\bar X = a - b i
conjugate may be stored in X */
void Cmplx_Conjugate( Cmplx self, Cmplx conjugate ) {
	conjugate[ REAL_PART ] = a;
	conjugate[ IMAG_PART ] = -b;
}

/** 
Defined as per: Eric W. Weisstein. "Complex Argument." From MathWorld--A Wolfram Web Resource. http://mathworld.wolfram.com/ComplexArgument.html */
double Cmplx_Argument( Cmplx self ) {
	double argument = atan( b / a );
	
	if ( a > 0.0 ) 		/* If number is in first or fourth quadrant */
		return argument;
	else if ( a < 0.0 ) {
		if ( b > 0.0 ) /* Number is in Second Quadrant */
			return argument + M_PI;  
		else           /* Number is in Third Quadrant */
			return argument - M_PI;  
	}
	/* If a == 0 then: arg(x) = 
		\pi/2     if b > 0.0 
		undefined if b == 0.0 
		-\pi/2    if b < 0.0 */
	else {
		if      ( b > 0.0 ) return  M_PI_2;
		else if ( b < 0.0 ) return -M_PI_2;
		else                return  0.0;
	}
}

/** 
X = a + b i
|X| = \sqrt{a^2 + b^2}
 */
double Cmplx_Modulus( Cmplx self ) {
	return sqrt( a * a + b * b );
}

/** 
X = a + b i = |X|(\cos(\theta) + i \sin( \theta )) = |X| e^{i\theta}
X^n = (|X| e^{i\theta})^n = |X|^n e^{i n \theta} = |X|^n (cos(\theta) + i sin(\theta) )
destination may be stored in 'self' */
void Cmplx_RealPower( Cmplx self, double y, Cmplx destination ) {
	double mod   = pow( Cmplx_Modulus(self), y );
	double theta = Cmplx_Argument( self ) * y;

	destination[ REAL_PART ] = mod * cos(theta);
	destination[ IMAG_PART ] = mod * sin(theta);
}

/** 
see Eric W. Weisstein. "Complex Exponentiation." From MathWorld--A Wolfram Web Resource. http://mathworld.wolfram.com/ComplexExponentiation.html
destination may be stored in either 'self' or 'power' */
void Cmplx_CmplxPower( Cmplx self, Cmplx Y, Cmplx destination ) {
	double argX = Cmplx_Argument( self );
	double modXSquared = a*a + b*b;
	double mod   = pow( modXSquared, c * 0.5 ) * exp( -argX * d );
	double theta = argX * c + 0.5 * d * log(modXSquared);

	destination[ REAL_PART ] = mod * cos(theta);
	destination[ IMAG_PART ] = mod * sin(theta);
}

/** Rotate complex value through an angle in radians. 
This rotates the complex number on the complex plane. */
void Cmplx_Rotate( Cmplx self, double angle, Cmplx dest ) {
	Cmplx rotationFactor;

	rotationFactor[ REAL_PART ] = cos( angle );
	rotationFactor[ IMAG_PART ] = sin( angle );

	Cmplx_Multiply( self, rotationFactor, dest );
}
