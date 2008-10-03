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
** $Id: IJKTopology.c 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"


#include "units.h"
#include "types.h"
#include "Topology.h"
#include "IJKTopology.h"

#include "Geometry.h"
#include "BlockGeometry.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>


/* Textual name of this class */
const Type IJKTopology_Type = "IJKTopology";

const char IJKTopology_DimNumToDimLetter[3] = { 'I', 'J', 'K' };

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

IJKTopology* _IJKTopology_New(
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print,
		Stg_Class_CopyFunction*				_copy, 
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
		Stg_Component_ConstructFunction*			_construct,
		Stg_Component_BuildFunction*		_build,
		Stg_Component_InitialiseFunction*		_initialise,
		Stg_Component_ExecuteFunction*		_execute,
		Stg_Component_DestroyFunction*		_destroy,
		Name							name,
		Bool							initFlag,
		Topology_NeighbourCountFunction*		neighbourCount,
		Topology_BuildNeighboursFunction*		buildNeighbours,
		Dictionary*					dictionary,
		IJK						size,
		Bool						isPeriodic[3] )
{
	IJKTopology* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(IJKTopology) );
	self = (IJKTopology*)_Topology_New( _sizeOfSelf, type, _delete, _print, _copy, _defaultConstructor,
			_construct, _build, _initialise, _execute, _destroy, name, initFlag, neighbourCount, buildNeighbours );
	
	/* General info */
	self->dictionary = dictionary;
	
	/* Virtual info */
	
	/* IJKTopology info */
	if( initFlag ){
		_IJKTopology_Init( self, size, isPeriodic, False );
	}
	
	return self;
}

void _IJKTopology_Construct( void* ijkTopology, Stg_ComponentFactory* cf, void* data ){
	IJKTopology*    self   = (IJKTopology*)ijkTopology;
	IJK             size;
	int             shift;
	Dimension_Index dim;
	Dimension_Index dim_I;
	Bool		isPeriodic[3];
	Bool		dynamic;

	self->dictionary = cf->rootDict;
	dim = Stg_ComponentFactory_GetRootDictUnsignedInt( cf, "dim", 0 );
	
	/* TODO _Topology_Construct( self, cf ); */
	_Topology_Init( (Topology*)self );

	size[ I_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, "sizeI", 1 );  
	size[ J_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, "sizeJ", 1 );  
	size[ K_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, "sizeK", 1 );  
	if ( dim == 2 )
		size[ K_AXIS ] = 1;

	isPeriodic[ I_AXIS ] = Stg_ComponentFactory_GetRootDictBool( cf, "isPeriodicI", False );  
	isPeriodic[ J_AXIS ] = Stg_ComponentFactory_GetRootDictBool( cf, "isPeriodicJ", False );  
	isPeriodic[ K_AXIS ] = Stg_ComponentFactory_GetRootDictBool( cf, "isPeriodicK", False );  

	dynamic = Stg_ComponentFactory_GetBool( cf, self->name, "dynamic", False );
	
	/* Shift the topology if nessesary */
	shift = Stg_ComponentFactory_GetInt( cf, self->name, "sizeShift", 0 );
	for ( dim_I = I_AXIS ; dim_I < dim ; dim_I++ )
		size[ dim_I ] += shift;

	_IJKTopology_Init( self, size, isPeriodic, dynamic );
}


void _IJKTopology_Init(
		IJKTopology*					self,
		IJK						size,
		Bool						isPeriodic[3], 
		Bool						dynamic )
{
	Index i = I_AXIS;

	/* General and Virtual info should already be set */
	
	/* IJKTopology info */
	self->isConstructed = True;
	if ( size ) {
		for (i=I_AXIS; i < 3; i++ ) {
			self->size[i] = size[i];
		}
	}
	else if( self->dictionary ) {
		char* keyBase = "meshSize";
		char* key = Memory_Alloc_Array_Unnamed( char, (strlen( keyBase ) + 2 ) );

		for (i=I_AXIS; i < 3; i++ ) {
			sprintf ( key, "%s%c", keyBase, IJKTopology_DimNumToDimLetter[i] );
			self->size[i] = Dictionary_Entry_Value_AsUnsignedInt( Dictionary_GetDefault(
				self->dictionary, key, Dictionary_Entry_Value_FromUnsignedInt( 1 ) ) );
		}
		Memory_Free( key );
	}
	else {
		fprintf( stderr, "Error - %s: either a valid Dictionary or size input parameter must be given.\n",
			__func__ );
		exit( EXIT_FAILURE );	
	}
	
	if ( isPeriodic ) {
		for (i=I_AXIS; i < 3; i++ ) {
			self->isPeriodic[i] = isPeriodic[i];
		}
	}
	else if( self->dictionary ) {
		self->isPeriodic[I_AXIS] = Dictionary_GetBool_WithDefault( self->dictionary, "isPeriodicI", False );
		self->isPeriodic[J_AXIS] = Dictionary_GetBool_WithDefault( self->dictionary, "isPeriodicJ", False );
		self->isPeriodic[K_AXIS] = Dictionary_GetBool_WithDefault( self->dictionary, "isPeriodicK", False );
	}
	else {
		fprintf( stderr, "Error - %s: either a valid Dictionary or \"isPeriodic\" input parameter must be given.\n",
			__func__ );
		exit( EXIT_FAILURE );	
	}	

	for (i=I_AXIS; i < 3; i++ ) {
		if ( self->size[i] == 0 ) self->size[i] = 1;
	}
	
	self->indexCount = self->size[I_AXIS] * self->size[J_AXIS] * self->size[K_AXIS];
	assert( self->indexCount );
	
	self->dynamicSizes = dynamic;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _IJKTopology_Delete( void* ijk26Topology ) {
	IJKTopology* self = (IJKTopology*)ijk26Topology;
	
	/* Stg_Class_Delete parent */
	_Topology_Delete( self );
}


void _IJKTopology_Print( void* ijk26Topology, Stream* stream ) {
	IJKTopology* self = (IJKTopology*)ijk26Topology;
	
	/* Print parent */
	_Topology_Print( self, stream );
	
	 IJKTopology_PrintConcise( self, stream );
}


void IJKTopology_PrintConcise( IJKTopology* self, Stream* stream ) {
	/* General info */
	Journal_Printf( stream, "IJKTopology (ptr): (%p)\n", self );
	
	/* IJKTopology info */
	Stream_Indent( stream );
	Journal_Printf( stream, "size : (%u,%u,%u)\n", self->size[0], self->size[1], self->size[2] );
	Journal_Printf( stream, "isPeriodic : (%s,%s,%s)\n",
		StG_BoolToStringMap[self->isPeriodic[I_AXIS]],
		StG_BoolToStringMap[self->isPeriodic[J_AXIS]],
		StG_BoolToStringMap[self->isPeriodic[K_AXIS]] );
	Journal_Printf( stream, "dynamicSizes : %s\n", StG_BoolToStringMap[self->dynamicSizes] );
	Stream_UnIndent( stream );
}


void* _IJKTopology_Copy( void* ijkTopology, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	IJKTopology*	self = (IJKTopology*)ijkTopology;
	IJKTopology*	newIJKTopology;
	Dimension_Index	dim_I=0;
	
	newIJKTopology = (IJKTopology*)_Topology_Copy( self, dest, deep, nameExt, ptrMap );
	
	newIJKTopology->dictionary = self->dictionary;
	for ( dim_I = 0; dim_I < 3; dim_I++ ) {
		newIJKTopology->size[dim_I] = self->size[dim_I];
		newIJKTopology->isPeriodic[dim_I] = self->isPeriodic[dim_I];
	}	
	newIJKTopology->dynamicSizes = self->dynamicSizes;
	
	return (void*)newIJKTopology;
}
