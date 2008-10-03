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
** Role:
** 	Tests Stg_Component copying
**
** $Id: testHierarchyTable.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"
#include "Base/Automation/Automation.h"

#include "Regresstor/libRegresstor/Regresstor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

const Type A_Type = "A";
const Type B_Type = "B";
const Type C_Type = "C";
const Type D_Type = "D";
const Type AA_Type = "AA";
const Type BB_Type = "BB";

int main( int argc, char *argv[] ) {
	int		rank;
	int		procCount;
	int		procToWatch;
	Stream*		stream;
	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_size( MPI_COMM_WORLD, &procCount );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	
	BaseFoundation_Init( &argc, &argv );
	BaseIO_Init( &argc, &argv );
	BaseContainer_Init( &argc, &argv );
	BaseAutomation_Init( &argc, &argv );

	stream = Journal_Register( Info_Type, __FILE__ );
	
	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	if( rank == procToWatch ) {
		HierarchyTable* hierarchyTable = HierarchyTable_GetHierarchyTable();

		RegisterParent( B_Type, A_Type );
		RegisterParent( C_Type, B_Type );
		RegisterParent( D_Type, C_Type );
		
		RegisterParent( BB_Type, AA_Type );

		Journal_PrintBool( stream, IsChild( A_Type, A_Type ) ); 	
		Journal_PrintBool( stream, IsChild( A_Type, B_Type ) ); 	
		Journal_PrintBool( stream, IsChild( A_Type, C_Type ) ); 	
		Journal_PrintBool( stream, IsChild( A_Type, D_Type ) ); 	
		Journal_PrintBool( stream, IsChild( A_Type, AA_Type ) ); 	
		Journal_PrintBool( stream, IsChild( A_Type, BB_Type ) ); 		
		Journal_Printf( stream, "\n");

		Journal_PrintBool( stream, IsChild( B_Type, A_Type ) ); 	
		Journal_PrintBool( stream, IsChild( B_Type, B_Type ) ); 	
		Journal_PrintBool( stream, IsChild( B_Type, C_Type ) ); 	
		Journal_PrintBool( stream, IsChild( B_Type, D_Type ) ); 	
		Journal_PrintBool( stream, IsChild( B_Type, AA_Type ) ); 	
		Journal_PrintBool( stream, IsChild( B_Type, BB_Type ) ); 	
		Journal_Printf( stream, "\n");
		
		Journal_PrintBool( stream, IsChild( C_Type, A_Type ) ); 	
		Journal_PrintBool( stream, IsChild( C_Type, B_Type ) ); 	
		Journal_PrintBool( stream, IsChild( C_Type, C_Type ) ); 	
		Journal_PrintBool( stream, IsChild( C_Type, D_Type ) ); 	
		Journal_PrintBool( stream, IsChild( C_Type, AA_Type ) ); 	
		Journal_PrintBool( stream, IsChild( C_Type, BB_Type ) ); 
		Journal_Printf( stream, "\n");

		Journal_PrintBool( stream, IsChild( D_Type, A_Type ) ); 	
		Journal_PrintBool( stream, IsChild( D_Type, B_Type ) ); 	
		Journal_PrintBool( stream, IsChild( D_Type, C_Type ) ); 	
		Journal_PrintBool( stream, IsChild( D_Type, D_Type ) ); 	
		Journal_PrintBool( stream, IsChild( D_Type, AA_Type ) ); 	
		Journal_PrintBool( stream, IsChild( D_Type, BB_Type ) ); 	
		Journal_Printf( stream, "\n");

		Journal_PrintBool( stream, IsChild( AA_Type, A_Type ) ); 	
		Journal_PrintBool( stream, IsChild( AA_Type, B_Type ) ); 	
		Journal_PrintBool( stream, IsChild( AA_Type, C_Type ) ); 	
		Journal_PrintBool( stream, IsChild( AA_Type, D_Type ) ); 	
		Journal_PrintBool( stream, IsChild( AA_Type, AA_Type ) ); 	
		Journal_PrintBool( stream, IsChild( AA_Type, BB_Type ) ); 
		Journal_Printf( stream, "\n");

		Journal_PrintBool( stream, IsChild( BB_Type, A_Type ) ); 	
		Journal_PrintBool( stream, IsChild( BB_Type, B_Type ) ); 	
		Journal_PrintBool( stream, IsChild( BB_Type, C_Type ) ); 	
		Journal_PrintBool( stream, IsChild( BB_Type, D_Type ) ); 	
		Journal_PrintBool( stream, IsChild( BB_Type, AA_Type ) ); 	
		Journal_PrintBool( stream, IsChild( BB_Type, BB_Type ) ); 
		Journal_Printf( stream, "\n");

		HierarchyTable_PrintParents( hierarchyTable, A_Type, stream );
		HierarchyTable_PrintParents( hierarchyTable, B_Type, stream );
		HierarchyTable_PrintParents( hierarchyTable, C_Type, stream );
		HierarchyTable_PrintParents( hierarchyTable, D_Type, stream );
		HierarchyTable_PrintParents( hierarchyTable, AA_Type, stream );
		HierarchyTable_PrintParents( hierarchyTable, BB_Type, stream );
		
		HierarchyTable_PrintChildren( hierarchyTable, A_Type, stream );
		HierarchyTable_PrintChildren( hierarchyTable, B_Type, stream );
		HierarchyTable_PrintChildren( hierarchyTable, C_Type, stream );
		HierarchyTable_PrintChildren( hierarchyTable, D_Type, stream );
		HierarchyTable_PrintChildren( hierarchyTable, AA_Type, stream );
		HierarchyTable_PrintChildren( hierarchyTable, BB_Type, stream );
	}

	
	BaseAutomation_Finalise();
	BaseContainer_Finalise();
	BaseIO_Finalise();

	BaseFoundation_Finalise();

	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}

