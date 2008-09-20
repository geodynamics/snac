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
** $Id: testComplexMath.c 3847 2006-10-11 02:38:21Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"

#include <stdio.h>

#undef Journal_PrintCmplx
/** Print a complex number.
Will use %.5f formatting */
#define Journal_PrintCmplx( stream, self ) \
                Journal_Printf( stream, #self " = %.7f %c %.7f i\n", (self)[ REAL_PART ], (self)[ IMAG_PART ] >= 0.0 ? '+' : '-', fabs( (self)[ IMAG_PART ] ) )

#define TESTCOMPLEXMATH_TOL 1e-15

int main( int argc, char* argv[] ) {
	MPI_Comm CommWorld;
	int rank;
	int numProcessors;
	int procToWatch;
	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );
	
	Base_Init( &argc, &argv );
	
	DiscretisationGeometry_Init( &argc, &argv );
	MPI_Barrier( CommWorld ); /* Ensures copyright info always come first in output */
	
	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}

	if( rank == procToWatch ) {
		Cmplx x = {1, 2};
		Cmplx y = {-1.5, 3};
		Cmplx u = {-1.5, -3};
		Cmplx v = {1.5, -3};
		Cmplx i = {0, 1};
		Cmplx minus_i = {0, -1};
		Cmplx e = {M_E, 0};
		Cmplx ipi = {0, M_PI};
		Cmplx dest;
		double mod, theta;
		Stream* stream = Journal_Register( InfoStream_Type, "ComplexMath" );
		
		Journal_Printf(stream, "\n----------------- Testing Complex Journal Printing Macro -----------------\n" );
		Journal_PrintCmplx( stream, x );	
		Journal_PrintCmplx( stream, y );	
		Journal_PrintCmplx( stream, u );	
		Journal_PrintCmplx( stream, v );	
		Journal_PrintCmplx( stream, i );	

		Journal_Printf(stream, "\n----------------- Testing Addition -----------------\n" );
		Cmplx_Add( x, y, dest );
		Cmplx_Add( x, y, x );
		Journal_PrintCmplx( stream, dest );	
		Journal_PrintCmplx( stream, x );	
		
		Journal_Printf(stream, "\n----------------- Testing Subtraction -----------------\n" );
		Cmplx_Subtract( x, y, x );
		Cmplx_Subtract( x, y, dest );
		Journal_PrintCmplx( stream, x );		
		Journal_PrintCmplx( stream, dest );	

		Journal_Printf(stream, "\n----------------- Testing Multiplication -----------------\n" );
		Cmplx_Multiply( x, y, dest );
		Cmplx_Multiply( x, y, y );
		Journal_PrintCmplx( stream, dest );	
		Journal_PrintCmplx( stream, y );	

		Journal_Printf(stream, "\n----------------- Testing Division -----------------\n" );
		Cmplx_Division( y, x, y );
		Cmplx_Division( x, y, dest );
		Journal_PrintCmplx( stream, y );	
		Journal_PrintCmplx( stream, dest );	
		
		Journal_Printf(stream, "\n----------------- Testing Real Number Math stuff -----------------\n" );
		Cmplx_AddReal( y, 2, dest );
		Journal_PrintCmplx( stream, dest );	
		Cmplx_RealMinusCmplx( y, 4, dest );
		Journal_PrintCmplx( stream, dest );	
		Cmplx_RealMultiply( y, 0.1, dest );
		Journal_PrintCmplx( stream, dest );	
		Cmplx_RealDivideByCmplx( y, 1.0, dest );
		Journal_PrintCmplx( stream, dest );	
		Cmplx_Multiply( y, dest, dest );
		/* Test here for tolerances */
		if (fabs(dest[IMAG_PART]) <= TESTCOMPLEXMATH_TOL ) {
			Journal_Printf(stream, "Answer within tolerance %g of %.5f + i %.5f\n", 
				TESTCOMPLEXMATH_TOL,
				dest[REAL_PART],
				fabs(0));
		}
		else{
			Journal_Printf(stream, "Answer not within tolerance %g of %.5f + i %.5f\n", 
				TESTCOMPLEXMATH_TOL,
				1,
				0);
		}

		Journal_Printf(stream, "\n----------------- Testing Conjugate -----------------\n" );
		Cmplx_Conjugate( x, x );
		Journal_PrintCmplx( stream, x );	
		Cmplx_Conjugate( x, x );
		Journal_PrintCmplx( stream, x );	

		Journal_Printf(stream, "\n----------------- Testing Complex Polar Stuff -----------------\n" );
		mod = Cmplx_Modulus(x);
		theta = Cmplx_Argument( x );
		Journal_Printf(stream, "x = %2.4lf e^{i %2.4lf} = %2.4lf + %2.4lf\n", mod, theta, mod * cos(theta), mod *sin(theta));
		mod = Cmplx_Modulus(y);
		theta = Cmplx_Argument( y );
		Journal_Printf(stream, "y = %2.4lf e^{i %2.4lf} = %2.4lf + %2.4lf\n", mod, theta, mod * cos(theta), mod *sin(theta));
		mod = Cmplx_Modulus(u);
		theta = Cmplx_Argument( u );
		Journal_Printf(stream, "u = %2.4lf e^{i %2.4lf} = %2.4lf + %2.4lf\n", mod, theta, mod * cos(theta), mod *sin(theta));
		mod = Cmplx_Modulus(v);
		theta = Cmplx_Argument( v );
		Journal_Printf(stream, "v = %2.4lf e^{i %2.4lf} = %2.4lf + %2.4lf\n", mod, theta, mod * cos(theta), mod *sin(theta));
		mod = Cmplx_Modulus(i);
		theta = Cmplx_Argument( i );
		Journal_Printf(stream, "i = %2.4lf e^{i %2.4lf} = %2.4lf + %2.4lf\n", mod, theta, mod * cos(theta), mod *sin(theta));
		mod = Cmplx_Modulus(minus_i);
		theta = Cmplx_Argument( minus_i );
		Journal_Printf(stream, "-i = %2.4lf e^{i %2.4lf} = %2.4lf + %2.4lf\n", mod, theta, mod * cos(theta), mod *sin(theta));
	
		Journal_Printf(stream, "\n----------------- Testing Complex to real Power Stuff -----------------\n" );
		Cmplx_RealPower( x, 2.0, dest );
		Journal_PrintCmplx( stream, dest );	
		Cmplx_Multiply( x, x, dest );
		Journal_PrintCmplx( stream, dest );	
		Cmplx_Multiply( dest, x, dest );
		Journal_PrintCmplx( stream, dest );	
		Cmplx_RealPower( x, 3.0, dest );
		Journal_PrintCmplx( stream, dest );	
		Cmplx_Sqrt( y, dest );
		Journal_PrintCmplx( stream, dest );	
		Cmplx_RealPower( dest, 2.0, dest );
		Journal_PrintCmplx( stream, dest );	
		
		Journal_Printf(stream, "\n----------------- Testing Complex to complex Power Stuff -----------------\n" );
		Cmplx_CmplxPower( x, y, dest );
		Journal_PrintCmplx( stream, dest );	


		Journal_Printf(stream, "\n----------------- Testing The Most Beautiful Equation in Mathematics e^{i \\pi} + 1 = 0 -----------------\n" );
		Journal_Printf( stream, "e^{i \\pi} = ");
		Cmplx_CmplxPower( e, ipi, dest );
		Journal_PrintCmplx( stream, dest );	

		Journal_Printf(stream, "\n----------------- Another Beautiful Equation i^i = e^{-\\pi/2} -----------------\n" );
		Cmplx_CmplxPower( i, i, dest );
		Journal_Printf( stream, "e^{-\\pi/2} = %2.5lf = ", exp( -M_PI * 0.5 ) );
		Journal_PrintCmplx( stream, dest );	

		Journal_Printf(stream, "\n----------------- Testing Exponential -----------------\n" );
		Journal_Printf( stream, "e^{2 + 3i} = ");
		x[ REAL_PART ] = 2.0;
		x[ IMAG_PART ] = 3.0;
		Cmplx_Exp( x, dest );
		Journal_PrintCmplx( stream, dest );	
		Cmplx_CmplxPower( e, x, dest );
		Journal_PrintCmplx( stream, dest );	

		Journal_Printf( stream, "e^{-5 + 7i} = ");
		x[ REAL_PART ] = -5.0;
		x[ IMAG_PART ] = 7.0;
		Cmplx_Exp( x, dest );
		Journal_PrintCmplx( stream, dest );	
		Cmplx_CmplxPower( e, x, dest );
		Journal_PrintCmplx( stream, dest );	

		Journal_Printf( stream, "e^{i \\pi} = ");
		Cmplx_Exp( ipi, dest );
		Journal_PrintCmplx( stream, dest );	
		
		Journal_Printf(stream, "\n----------------- Testing Copy and Zero -----------------\n" );
		Journal_PrintCmplx( stream, x );	
		Journal_PrintCmplx( stream, dest );	
		Cmplx_Copy( x, dest );
		Journal_PrintCmplx( stream, dest );	
		Cmplx_Zero( dest );
		Journal_PrintCmplx( stream, dest );	

	}
	
	printf("\n");
	DiscretisationGeometry_Finalise();
	
	Base_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0;
}
