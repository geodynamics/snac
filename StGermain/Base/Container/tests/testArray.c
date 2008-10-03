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
** $Id: testArray.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main( int argc, char* argv[] ) {
	MPI_Comm			CommWorld;
	int				rank;
	int				numProcessors;
	int				procToWatch;
	
	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );

	BaseFoundation_Init( &argc, &argv );
	BaseIO_Init( &argc, &argv );
	BaseContainer_Init( &argc, &argv );

	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	
	if( rank == procToWatch ) {
		Stream*			stream;
		double			sourceArray[] = { 0.1f, 0.2f, 0.3f, 0.4f };
		Array*			array0; /* Heap (new) */
		double*			array0Ptr;
		Array			array1; /* Stack (init) */
		double*			array1Ptr;
		Array*			array0deep;
		Array*			array0shallow;
		Array*			array1deep;
		Array*			array1shallow;
		Index			i;
		Bool			fail;
		
		stream = Journal_Register( Info_Type, "myStream" );
		
		
		/* Test 1: Construct the Array(s) */
		array0 = Array_New( sizeof(double), 4 );
		Array_Init( &array1, sizeof(double), 4 );
		Journal_Printf( stream, "Passed test 1: Construct the Array(s)\n" );
		
		/* Test 2: Do we trust Array_Ptr, Array_Count & Array_ElementSize? */
		if( 
			Array_Ptr(  array0 ) == array0->ptr &&
			Array_Ptr(  array0 ) != 0 &&
			Array_Count(  array0 ) == 4 &&
			Array_ElementSize(  array0 ) == sizeof(double) &&
			Array_Ptr( &array1 ) == array1.ptr &&
			Array_Ptr( &array1 ) != 0 &&
			Array_Count( &array1 ) == 4 &&
			Array_ElementSize( &array1 ) == sizeof(double) &&
			Array_Ptr( array0 ) != Array_Ptr( &array1 ) )
		{
			Journal_Printf( stream, "Passed test 2: Do we trust Array_Ptr, Array_Count & Array_ElementSize?\n" );
		}
		else {
			Journal_Firewall( 0, stream, "Failed test 2: Do we trust Array_Ptr, Array_Count & Array_ElementSize?\n" );
		}
		
		
		/* Test 3: Do we trust Array_At? */
		array0Ptr = (double*)Array_Ptr( array0 );
		array1Ptr = (double*)Array_Ptr( &array1 );
		for( i = 0; i < 4; i++ ) {
			array0Ptr[i] = sourceArray[i];
			array1Ptr[i] = sourceArray[i];
		}
		
		fail = False;
		for( i = 0; i < 4; i++ ) {
			if( 
				*((double*)Array_At(  array0, i )) != sourceArray[i] || 
				*((double*)Array_At( &array1, i )) != sourceArray[i] ) 
			{
				fail = True;
			}
		}
		if( fail ) {
			Journal_Firewall( 0, stream, "Failed test 3: Do we trust Array_At?\n" );
		}
		else {
			Journal_Printf( stream, "Passed test 3: Do we trust Array_At?\n" );
		}
		
		
		/* Test 4: Do we trust Array_Copy (Deep and Shallow)? */
		array0deep =	Stg_Class_Copy(  array0, 0, True,  0, 0 );
		array0shallow =	Stg_Class_Copy(  array0, 0, False, 0, 0 );
		array1deep =	Stg_Class_Copy( &array1, 0, True,  0, 0 );
		array1shallow =	Stg_Class_Copy( &array1, 0, False, 0, 0 );
		
		Journal_Firewall(
			array0->ptr != array0deep->ptr &&
			array0->ptr == array0shallow->ptr &&
			array1.ptr  != array1deep->ptr &&
			array1.ptr  == array1shallow->ptr,
			stream, 
			"Failed test 4: Do we trust Array_Copy (Deep and Shallow)?\n" );
		
		fail = False;
		for( i = 0; i < 4; i++ ) {
			if( 
				*((double*)Array_At( array0deep,    i )) != sourceArray[i] || 
				*((double*)Array_At( array0shallow, i )) != sourceArray[i] || 
				*((double*)Array_At( array1deep,    i )) != sourceArray[i] || 
				*((double*)Array_At( array1shallow, i )) != sourceArray[i] ) 
			{
				fail = True;
			}
		}
		if( fail ) {
			Journal_Firewall( 0, stream, "Failed test 4: Do we trust Array_Copy (Deep and Shallow)?\n" );
		}
		else {
			Journal_Printf( stream, "Passed test 4: Do we trust Array_Copy (Deep and Shallow)?\n" );
		}
		
		
		/* Test 5: Do we trust Array_Owner/Array_IOwn? */
		Journal_Firewall(
			 Array_IOwn( array0 ) &&
			 Array_IOwn( array0deep ) &&
			!Array_IOwn( array0shallow ) &&
			 Array_IOwn( &array1 ) &&
			 Array_IOwn( array1deep ) &&
			!Array_IOwn( array1shallow ) &&
			Array_Owner( array0 ) != Array_Owner( array0deep ) &&
			Array_Owner( array0 ) == Array_Owner( array0shallow ) &&
			Array_Owner( &array1 ) != Array_Owner( array1deep ) &&
			Array_Owner( &array1 ) == Array_Owner( array1shallow ) &&
			Array_Owner( array0 ) != Array_Owner( &array1 ),
			stream, 
			"Failed test 5: Do we trust Array_Owner/Array_IOwn?\n" );
		Journal_Printf( stream, "Passed test 5: Do we trust Array_Owner/Array_IOwn?\n" );
		
		
		/* Test 6: Delete the Array(s) */
		Stg_Class_Delete( array1deep );
		Stg_Class_Delete( array0deep );
		Stg_Class_Delete( &array1 );
		Stg_Class_Delete( array0 );
		Stg_Class_Delete( array1shallow ); /* Purposely do after "owner" has been deleted*/
		Stg_Class_Delete( array0shallow ); /* Purposely do after "owner" has been deleted*/
		Journal_Printf( stream, "Passed test 6: Delete the Array(s)\n" );
	}
	
	BaseContainer_Finalise();
	BaseIO_Finalise();
	BaseFoundation_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}
