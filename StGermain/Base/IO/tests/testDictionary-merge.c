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
**	Tests the dictionary functionality
**
** $Id: testDictionary-merge.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"

#include <stdio.h>
#include <stdlib.h>


int main( int argc, char* argv[] ) {
	
	Stream*				stream = NULL;
	Dictionary*			dictionary;
	Dictionary_Entry_Value*		tmpVal0;
	Dictionary_Entry_Value*		tmpVal1; 
	
	MPI_Init( &argc, &argv );
	BaseFoundation_Init( &argc, &argv );
	BaseIO_Init( &argc, &argv );
	stream = Journal_Register (Info_Type, "myStream");

	dictionary = Dictionary_New();

	tmpVal0 = Dictionary_Entry_Value_NewStruct();
	tmpVal1 = Dictionary_Entry_Value_NewStruct();
	Dictionary_Entry_Value_AddMember( tmpVal1, "name", Dictionary_Entry_Value_FromString( "bill" ) );
	Dictionary_Entry_Value_AddMember( tmpVal1, "value", Dictionary_Entry_Value_FromDouble( 1.0f ) );
	Dictionary_Entry_Value_AddMember( tmpVal0, "inside", tmpVal1 );
	tmpVal1 = Dictionary_Entry_Value_NewList();
	Dictionary_Entry_Value_AddElement( tmpVal1, Dictionary_Entry_Value_FromString( "bottom" ) );
	Dictionary_Entry_Value_AddElement( tmpVal1, Dictionary_Entry_Value_FromString( "top" ) );
	Dictionary_Entry_Value_AddMember( tmpVal0, "list_one", tmpVal1 );
	Dictionary_Add( dictionary, "one", tmpVal0 );
	
	tmpVal0 = Dictionary_Entry_Value_NewStruct();
	tmpVal1 = Dictionary_Entry_Value_NewStruct();
	Dictionary_Entry_Value_AddMember( tmpVal1, "new_name", Dictionary_Entry_Value_FromString( "frank" ) );
	Dictionary_Entry_Value_AddMember( tmpVal1, "value", Dictionary_Entry_Value_FromDouble( 2.0f ) );
	Dictionary_Entry_Value_AddMember( tmpVal0, "inside", tmpVal1 );
	tmpVal1 = Dictionary_Entry_Value_NewList();
	Dictionary_Entry_Value_AddElement( tmpVal1, Dictionary_Entry_Value_FromString( "left" ) );
	Dictionary_Entry_Value_AddElement( tmpVal1, Dictionary_Entry_Value_FromString( "right" ) );
	Dictionary_Entry_Value_AddMember( tmpVal0, "list_one", tmpVal1 );
	Dictionary_AddMerge( dictionary, "one", tmpVal0, Dictionary_MergeType_Append );
	
	Print( dictionary, stream );
	Stg_Class_Delete( dictionary );
	
	dictionary = Dictionary_New();
	
	tmpVal0 = Dictionary_Entry_Value_NewStruct();
	tmpVal1 = Dictionary_Entry_Value_NewStruct();
	Dictionary_Entry_Value_AddMember( tmpVal1, "name", Dictionary_Entry_Value_FromString( "bill" ) );
	Dictionary_Entry_Value_AddMember( tmpVal1, "value", Dictionary_Entry_Value_FromDouble( 1.0f ) );
	Dictionary_Entry_Value_AddMember( tmpVal0, "inside", tmpVal1 );
	tmpVal1 = Dictionary_Entry_Value_NewList();
	Dictionary_Entry_Value_AddElement( tmpVal1, Dictionary_Entry_Value_FromString( "bottom" ) );
	Dictionary_Entry_Value_AddElement( tmpVal1, Dictionary_Entry_Value_FromString( "top" ) );
	Dictionary_Entry_Value_AddMember( tmpVal0, "list_one", tmpVal1 );
	Dictionary_Add( dictionary, "one", tmpVal0 );
	
	tmpVal0 = Dictionary_Entry_Value_NewStruct();
	tmpVal1 = Dictionary_Entry_Value_NewStruct();
	Dictionary_Entry_Value_AddMember( tmpVal1, "new_name", Dictionary_Entry_Value_FromString( "frank" ) );
	Dictionary_Entry_Value_AddMember( tmpVal1, "value", Dictionary_Entry_Value_FromDouble( 2.0f ) );
	Dictionary_Entry_Value_AddMember( tmpVal0, "inside", tmpVal1 );
	tmpVal1 = Dictionary_Entry_Value_NewList();
	Dictionary_Entry_Value_AddElement( tmpVal1, Dictionary_Entry_Value_FromString( "left" ) );
	Dictionary_Entry_Value_AddElement( tmpVal1, Dictionary_Entry_Value_FromString( "right" ) );
	Dictionary_Entry_Value_AddMember( tmpVal0, "list_one", tmpVal1 );
	Dictionary_AddMerge( dictionary, "one", tmpVal0, Dictionary_MergeType_Merge );
	
	Print( dictionary, stream );
	Stg_Class_Delete( dictionary );
	
	dictionary = Dictionary_New();
	
	tmpVal0 = Dictionary_Entry_Value_NewStruct();
	tmpVal1 = Dictionary_Entry_Value_NewStruct();
	Dictionary_Entry_Value_AddMember( tmpVal1, "name", Dictionary_Entry_Value_FromString( "bill" ) );
	Dictionary_Entry_Value_AddMember( tmpVal1, "value", Dictionary_Entry_Value_FromDouble( 1.0f ) );
	Dictionary_Entry_Value_AddMember( tmpVal0, "inside", tmpVal1 );
	tmpVal1 = Dictionary_Entry_Value_NewList();
	Dictionary_Entry_Value_AddElement( tmpVal1, Dictionary_Entry_Value_FromString( "bottom" ) );
	Dictionary_Entry_Value_AddElement( tmpVal1, Dictionary_Entry_Value_FromString( "top" ) );
	Dictionary_Entry_Value_AddMember( tmpVal0, "list_one", tmpVal1 );
	Dictionary_Add( dictionary, "one", tmpVal0 );
	
	tmpVal0 = Dictionary_Entry_Value_NewStruct();
	tmpVal1 = Dictionary_Entry_Value_NewStruct();
	Dictionary_Entry_Value_AddMember( tmpVal1, "new_name", Dictionary_Entry_Value_FromString( "frank" ) );
	Dictionary_Entry_Value_AddMember( tmpVal1, "value", Dictionary_Entry_Value_FromDouble( 2.0f ) );
	Dictionary_Entry_Value_AddMember( tmpVal0, "inside", tmpVal1 );
	tmpVal1 = Dictionary_Entry_Value_NewList();
	Dictionary_Entry_Value_AddElement( tmpVal1, Dictionary_Entry_Value_FromString( "left" ) );
	Dictionary_Entry_Value_AddElement( tmpVal1, Dictionary_Entry_Value_FromString( "right" ) );
	Dictionary_Entry_Value_AddMember( tmpVal0, "list_one", tmpVal1 );
	Dictionary_AddMerge( dictionary, "one", tmpVal0, Dictionary_MergeType_Replace );
	
	Print( dictionary, stream );
	Stg_Class_Delete( dictionary );
	
	BaseIO_Finalise();
	BaseFoundation_Finalise();
	MPI_Finalize();
	
	return EXIT_SUCCESS;
}
