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
** $Id: testLine.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"

#include <stdio.h>


int main( int argc, char* argv[] ) {
	Coord		points[4] = { { 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 0.5, 0.9, 0.5 }, { 1.2, 0.7, 0.8 } };
	Coord		insidePoint = { 0.7, 0.3, 0.0 };
	Coord		outsidePoint = { -0.2, 0.3, 0.0 };
	Stg_Line		lines[4];
	Index		i;
	MPI_Comm	CommWorld;
	int		rank;
	int		numProcessors;
	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );
	
	Base_Init( &argc, &argv );
	
	DiscretisationGeometry_Init( &argc, &argv );
	MPI_Barrier( CommWorld ); /* Ensures copyright info always come first in output */
	
	/*
	** When lines are to be used as boundaries (ie. determining whether other points lay
	** to the left or right of the line), points must be specified from left to right, where
	** the region in front of the line is considered the 'inside'.
	*/
	Stg_Line_CalcFromPoints( lines[0], points[0], points[1] );
	Stg_Line_CalcFromPoints( lines[1], points[2], points[0] );
	Stg_Line_CalcFromPoints( lines[2], points[1], points[3] );
	Stg_Line_CalcFromPoints( lines[3], points[3], points[2] );
	
	for( i = 0; i < 4; i++ ) {
		printf( "%s\n", Stg_Line_PointIsInside( lines[i], insidePoint ) ? "True" : "False" );
		printf( "%s\n", Stg_Line_PointIsInside( lines[i], outsidePoint ) ? "True" : "False" );
	}
	
	DiscretisationGeometry_Finalise();
	
	Base_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0;
}
