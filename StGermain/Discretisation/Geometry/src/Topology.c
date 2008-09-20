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
** $Id: Topology.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"


#include "units.h"
#include "types.h"
#include "Topology.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


/* Textual name of this class */
const Type Topology_Type = "Topology";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

Topology* _Topology_New(
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
		Topology_BuildNeighboursFunction*		buildNeighbours )
{
	Topology* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(Topology) );
	self = (Topology*)_Stg_Component_New( _sizeOfSelf, type, _delete, _print, _copy, _defaultConstructor, _construct, _build, 
			_initialise, _execute, _destroy, name, NON_GLOBAL );
	
	/* General info */
	
	/* Virtual info */
	self->neighbourCount = neighbourCount;
	self->buildNeighbours = buildNeighbours;
	
	/* Topology info */
	if( initFlag ){
		_Topology_Init( self );
	}
	
	return self;
}

void _Topology_Init(
		Topology*					self )
{
	/* General and Virtual info should already be set */
	
	/* Topology info */
	self->isConstructed = True;
	self->indexCount = 0;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _Topology_Delete( void* topology ) {
	Topology* self = (Topology*)topology;
	
	/* Stg_Class_Delete parent */
	_Stg_Component_Delete( self );
}


void _Topology_Print( void* topology, Stream* stream ) {
	Topology* self = (Topology*)topology;
	
	/* Set the Journal for printing informations */
	Stream* topologyStream;
	topologyStream = Journal_Register( InfoStream_Type, "TopologyStream" );

	/* Print parent */
	_Stg_Component_Print( self, stream );
	
	/* General info */
	Journal_Printf( stream, "Topology (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* Topology info */
}


void* _Topology_Copy( void* topology, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	Topology*	self = (Topology*)topology;
	Topology*	newTopology;
	
	newTopology = (Topology*)_Stg_Component_Copy( self, dest, deep, nameExt, ptrMap );
	
	/* Virtual methods */
	newTopology->neighbourCount = self->neighbourCount;
	newTopology->buildNeighbours = self->buildNeighbours;
	
	newTopology->indexCount = self->indexCount;
	
	return (void*)newTopology;
}


NeighbourIndex Topology_NeighbourCount( void* topology, Index index ) {
	Topology* self = (Topology*)topology;
	assert( topology );
	return self->neighbourCount( self, index );
}

void Topology_BuildNeighbours( void* topology, Index index, NeighbourIndex* neighbours ) {
	Topology* self = (Topology*)topology;
	assert( topology );
	self->buildNeighbours( self, index, neighbours );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

