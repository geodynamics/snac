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
** $Id: testLiveComponentRegister.c 2136 2005-05-10 02:47:13Z RaquibulHassan $
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

/*
 * A
 * |
 * +------>C
 * |       |
 * +-->B<--+
 */

struct Stg_ComponentA;
struct Stg_ComponentB;
struct Stg_ComponentC;

typedef struct Stg_ComponentA Stg_ComponentA;
typedef struct Stg_ComponentB Stg_ComponentB;
typedef struct Stg_ComponentC Stg_ComponentC;

Type Stg_ComponentA_Type = "Stg_ComponentA";
Type Stg_ComponentB_Type = "Stg_ComponentB";
Type Stg_ComponentC_Type = "Stg_ComponentC";

const char* Stg_ComponentA_Type_GetMetadata() { return  ""; }
const char* Stg_ComponentB_Type_GetMetadata() { return  ""; }
const char* Stg_ComponentC_Type_GetMetadata() { return  ""; }


#define __Stg_ComponentA \
	__Stg_Component \
	Stg_ComponentB* b; \
	Stg_ComponentC* c;
struct Stg_ComponentA { __Stg_ComponentA };

void* Stg_ComponentA_NewDefault( Name name );
void Stg_ComponentA_Delete( void* class );
void Stg_ComponentA_Print( void* class, Stream* s );
void* Stg_ComponentA_Copy( void* self, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
void Stg_ComponentA_Construct( void* component, Stg_ComponentFactory* cf, void* data );
void Stg_ComponentA_Build( void* component, void* data );
void Stg_ComponentA_Initialise( void* component, void* data );
void Stg_ComponentA_Execute( void* component, void* data );
void Stg_ComponentA_Destroy( void* component, void* data );

void* Stg_ComponentA_NewDefault( Name name )
{
	return _Stg_Component_New(
		sizeof(Stg_ComponentA),
		"Stg_ComponentA",
		Stg_ComponentA_Delete,
		Stg_ComponentA_Print,
		Stg_ComponentA_Copy,
		Stg_ComponentA_NewDefault,
		Stg_ComponentA_Construct,
		Stg_ComponentA_Build,
		Stg_ComponentA_Initialise,
		Stg_ComponentA_Execute,
		Stg_ComponentA_Destroy,
		name,
		NON_GLOBAL );
}
void Stg_ComponentA_Delete( void* class ) {
	Stg_ComponentA* self = (Stg_ComponentA*)class;
	
	Stg_Class_Delete( self->c );
	Stg_Class_Delete( self->b );

	_Stg_Component_Delete( self );
}
void Stg_ComponentA_Print( void* class, Stream* s ) {
	Stg_ComponentA* self = (Stg_ComponentA*)class;

	Journal_Printf(
		s,
		"%s %s %d,\n", self->name, self->type, Memory_CountGet( self ) );
	Print( self->b, s );
	Print( self->c, s );
}
void* Stg_ComponentA_Copy( void* class, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	Stg_ComponentA* self = (Stg_ComponentA*)class;
	Stg_ComponentA* newCopy;
	
	newCopy = _Stg_Component_Copy( self, dest, deep, nameExt, ptrMap );
	PtrMap_Append( ptrMap, self, newCopy );
	
	newCopy->b = Stg_Class_Copy( self->b, dest, deep, nameExt, ptrMap );
	newCopy->c = Stg_Class_Copy( self->c, dest, deep, nameExt, ptrMap );
	
	return newCopy;
}
void Stg_ComponentA_Construct( void* component, Stg_ComponentFactory* cf, void* data ) {
	Stg_ComponentA* self = (Stg_ComponentA*)component;

	self->b =  Stg_ComponentFactory_ConstructByName(  cf,  "b", Stg_ComponentB,  True, data  ) ;
	self->c =  Stg_ComponentFactory_ConstructByName(  cf,  "c", Stg_ComponentC,  True, data  ) ;
}
void Stg_ComponentA_Build( void* component, void* data ) {
	Stg_ComponentA* self = (Stg_ComponentA*)component;
	
	Build( self->b, data, False );
	Build( self->c, data, False );
}
void Stg_ComponentA_Initialise( void* component, void* data ) {

}
void Stg_ComponentA_Execute( void* component, void* data ) {

}
void Stg_ComponentA_Destroy( void* component, void* data ) {

}

#define __Stg_ComponentB \
	__Stg_Component
struct Stg_ComponentB { __Stg_ComponentB };

void* Stg_ComponentB_NewDefault( Name name );
void Stg_ComponentB_Delete( void* class );
void Stg_ComponentB_Print( void* class, Stream* s );
void* Stg_ComponentB_Copy( void* self, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
void Stg_ComponentB_Construct( void* component, Stg_ComponentFactory* cf, void* data );
void Stg_ComponentB_Build( void* component, void* data );
void Stg_ComponentB_Initialise( void* component, void* data );
void Stg_ComponentB_Execute( void* component, void* data );
void Stg_ComponentB_Destroy( void* component, void* data );

void* Stg_ComponentB_NewDefault( Name name )
{
	return _Stg_Component_New(
		sizeof(Stg_ComponentB),
		"Stg_ComponentB",
		Stg_ComponentB_Delete,
		Stg_ComponentB_Print,
		Stg_ComponentB_Copy,
		Stg_ComponentB_NewDefault,
		Stg_ComponentB_Construct,
		Stg_ComponentB_Build,
		Stg_ComponentB_Initialise,
		Stg_ComponentB_Execute,
		Stg_ComponentB_Destroy,
		name,
		NON_GLOBAL );
}
void Stg_ComponentB_Delete( void* class ) {
	Stg_ComponentB* self = (Stg_ComponentB*)class;
	
	_Stg_Component_Delete( self );
}
void Stg_ComponentB_Print( void* class, Stream* s ) {
	Stg_ComponentB* self = (Stg_ComponentB*)class;

	Journal_Printf(
		s,
		"%s %s %d,\n", self->name, self->type, Memory_CountGet( self ) );
}
void* Stg_ComponentB_Copy( void* class, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	Stg_ComponentB* self = (Stg_ComponentB*)class;
	Stg_ComponentB* newCopy;
	
	newCopy = _Stg_Component_Copy( self, dest, deep, nameExt, ptrMap );
	PtrMap_Append( ptrMap, self, newCopy );
	
	return newCopy;
}
void Stg_ComponentB_Construct( void* component, Stg_ComponentFactory* cf, void* data ) {
}
void Stg_ComponentB_Build( void* component, void* data ) {
}
void Stg_ComponentB_Initialise( void* component, void* data ) {
}
void Stg_ComponentB_Execute( void* component, void* data ) {
}
void Stg_ComponentB_Destroy( void* component, void* data ) {
}


#define __Stg_ComponentC \
	__Stg_Component \
	Stg_ComponentB* b;
struct Stg_ComponentC { __Stg_ComponentC };

void* Stg_ComponentC_NewDefault( Name name );
void Stg_ComponentC_Delete( void* class );
void Stg_ComponentC_Print( void* class, Stream* s );
void* Stg_ComponentC_Copy( void* self, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
void Stg_ComponentC_Construct( void* component, Stg_ComponentFactory* cf, void* data );
void Stg_ComponentC_Build( void* component, void* data );
void Stg_ComponentC_Initialise( void* component, void* data );
void Stg_ComponentC_Execute( void* component, void* data );
void Stg_ComponentC_Destroy( void* component, void* data );

void* Stg_ComponentC_NewDefault( Name name )
{
	return _Stg_Component_New(
		sizeof(Stg_ComponentC),
		"Stg_ComponentC",
		Stg_ComponentC_Delete,
		Stg_ComponentC_Print,
		Stg_ComponentC_Copy,
		Stg_ComponentC_NewDefault,
		Stg_ComponentC_Construct,
		Stg_ComponentC_Build,
		Stg_ComponentC_Initialise,
		Stg_ComponentC_Execute,
		Stg_ComponentC_Destroy,
		name,
		NON_GLOBAL );
}
void Stg_ComponentC_Delete( void* class ) {
	Stg_ComponentC* self = (Stg_ComponentC*)class;
	
	Stg_Class_Delete( self->b );

	_Stg_Component_Delete( self );
}
void Stg_ComponentC_Print( void* class, Stream* s ) {
	Stg_ComponentC* self = (Stg_ComponentC*)class;

	Journal_Printf(
		s,
		"%s %s %d,\n", self->name, self->type, Memory_CountGet( self ) );
	Print( self->b, s );
}
void* Stg_ComponentC_Copy( void* class, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	Stg_ComponentC* self = (Stg_ComponentC*)class;
	Stg_ComponentC* newCopy;
	
	newCopy = _Stg_Component_Copy( self, dest, deep, nameExt, ptrMap );
	PtrMap_Append( ptrMap, self, newCopy );

	newCopy->b = Stg_Class_Copy( self->b, dest, deep, nameExt, ptrMap );

	return newCopy;
}
void Stg_ComponentC_Construct( void* component, Stg_ComponentFactory* cf, void* data ) {
	Stg_ComponentC* self = (Stg_ComponentC*)component;

	self->b =  Stg_ComponentFactory_ConstructByName(  cf,  "b", Stg_ComponentB,  True, data  ) ;
}
void Stg_ComponentC_Build( void* component, void* data ) {
	Stg_ComponentC* self = (Stg_ComponentC*)component;
	
	Build( self->b, data, False );
}
void Stg_ComponentC_Initialise( void* component, void* data ) {

}
void Stg_ComponentC_Execute( void* component, void* data ) {

}
void Stg_ComponentC_Destroy( void* component, void* data ) {

}





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

	RegressionTest_Init( "Base/Automation/Stg_Component" );
	
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
		Stg_ComponentA* a;
		Stg_ComponentA* aCopy;
		XML_IO_Handler* ioHandler;
		Dictionary* dictionary;
		Dictionary* componentDictionary;

		Stg_ComponentFactory* cf;

		Stg_ComponentRegister_Add( 
			Stg_ComponentRegister_Get_ComponentRegister(), 
			Stg_ComponentA_Type, 
			"0", 
			(Stg_Component_DefaultConstructorFunction*)Stg_ComponentA_NewDefault );

		Stg_ComponentRegister_Add( 
			Stg_ComponentRegister_Get_ComponentRegister(), 
			Stg_ComponentB_Type,
			"0",
			(Stg_Component_DefaultConstructorFunction*)Stg_ComponentB_NewDefault );

		Stg_ComponentRegister_Add( 
			Stg_ComponentRegister_Get_ComponentRegister(), 
			Stg_ComponentC_Type,
			"0",
			(Stg_Component_DefaultConstructorFunction*)Stg_ComponentC_NewDefault );

		/* Create the application's dictionary */
		dictionary = Dictionary_New();
                                                                                                                                    
		/* Read input */
		ioHandler = XML_IO_Handler_New();
		IO_Handler_ReadAllFromFile( ioHandler, "data/copy.xml", dictionary );
                                                                                                                                    
		Journal_ReadFromDictionary( dictionary );

		componentDictionary = Dictionary_GetDictionary( dictionary, "components" );
		if ( componentDictionary ) {
			cf = Stg_ComponentFactory_New( dictionary, componentDictionary, Stg_ObjectList_New() );

			Stg_ComponentFactory_CreateComponents( cf );
			Stg_ComponentFactory_ConstructComponents( cf, 0 /* dummy */ );

			a = (Stg_ComponentA*)LiveComponentRegister_Get( cf->LCRegister, "a" );
			
			RegressionTest_Check(
				a != NULL &&
				a->b != NULL &&
				a->c != NULL &&
				a->b == a->c->b,
				stream,
				"Stg_Components creation",
				"Can we create the components from dictionary" );

			if ( a ) {
				Print( a, stream );
				
				aCopy = Stg_Class_Copy( a, NULL, True, "_dup", NULL );

				/* Disown the copy. Why? Because main() doesn't own the original neither.
				 * The instance counting then matches up.
				 */
				Stg_Component_Disown( aCopy );

				RegressionTest_Check(
					aCopy != NULL &&
					aCopy->b != NULL &&
					aCopy->c != NULL &&
					aCopy->b == aCopy->c->b,
					stream,
					"Stg_Components copy",
					"Can we copy the components and subcomponents correctly" );

				if ( aCopy ) {
					Print( aCopy, stream );
				}

				RegressionTest_Check(
					Memory_CountGet( a ) == Memory_CountGet( aCopy ) &&
					Memory_CountGet( a->b ) == Memory_CountGet( aCopy->b ) &&
					Memory_CountGet( a->c ) == Memory_CountGet( aCopy->c ),
					stream,
					"Instance counter",
					"Are the instance counters correct" );
				
				Print( cf->LCRegister, stream );
			}

		}

		Stg_Class_Delete( ioHandler );
		Stg_Class_Delete( dictionary );
	}

	
	BaseAutomation_Finalise();
	BaseContainer_Finalise();
	BaseIO_Finalise();

	RegressionTest_Finalise();
	
	BaseFoundation_Finalise();

	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}

