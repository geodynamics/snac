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
**
**
** Assumptions:
**
** Comments:
**
** $Id: Swarm_Register.c 2745 2005-05-10 08:12:18Z RaquibulHassan $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"
#include "Discretisation/Utils/Utils.h"

#include "types.h"
#include "shortcuts.h"
#include "Swarm_Register.h"

#include "ParticleLayout.h"
#include "SwarmClass.h"
#include "SwarmDump.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


/* Textual name of this class */
const Type Swarm_Register_Type = "Swarm_Register";
Swarm_Register* stgSwarm_Register = NULL;

Swarm_Register* _Swarm_Register_New( 
		SizeT					_sizeOfSelf, 
		Type					type,
		Stg_Class_DeleteFunction*		_delete,
		Stg_Class_PrintFunction*		_print, 
		Stg_Class_CopyFunction*			_copy )
{
	Swarm_Register* self = NULL;

	assert( _sizeOfSelf >= sizeof(Swarm_Register) );
	self = (Swarm_Register*) _Stg_Class_New( _sizeOfSelf, type, _delete, _print, _copy );
	
	return self;
}
	
Swarm_Register* Swarm_Register_New(  )
{
	Swarm_Register* self = NULL;

	self = _Swarm_Register_New( 
			sizeof(Swarm_Register), 
			Swarm_Register_Type,
			_Swarm_Register_Delete,
			_Swarm_Register_Print,
			NULL);

	Swarm_Register_Init( self );

	if ( !stgSwarm_Register ) {
		stgSwarm_Register = self;
	}
	return self;
}

void _Swarm_Register_Init( Swarm_Register* self )
{
}
	
void Swarm_Register_Init( Swarm_Register* self )
{
	assert( self );

	self->swarmList = Stg_ObjectList_New( );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _Swarm_Register_Delete( void* swarmRegister )
{
	Swarm_Register* self = (Swarm_Register *) swarmRegister;

	assert( self );

	/** Actually, don't delete all swarms as they have already been deleted by the
	live component register : or else leave it up to the user */
	/* Stg_ObjectList_DeleteAllObjects( self->swarmList ); */

	Stg_Class_Delete( self->swarmList );
	if ( stgSwarm_Register == self ) {
		stgSwarm_Register = NULL;
	}
	_Stg_Class_Delete( self );
}

void _Swarm_Register_Print( void* swarmRegister, Stream* stream )
{
	Swarm_Register* self = (Swarm_Register *) swarmRegister;

	assert( self );
	
	/* General info */
	Journal_Printf( (void*) stream, "Swarm_Register (ptr): %p\n", self );
	
	/* Print parent class */
	_Stg_Class_Print( self, stream );
	
	Journal_Printf( (void*) stream, "swarmList (ptr): %p\n", self->swarmList );
	Stg_Class_Print( self->swarmList, stream );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Functions
*/

Index Swarm_Register_Add( Swarm_Register* self, void* swarm )
{
	assert( self );
	return Stg_ObjectList_Append( self->swarmList, swarm );
}
	
Swarm* Swarm_Register_Get( Swarm_Register* self, Name name )
{
	assert( self );
	
	return (Swarm*)Stg_ObjectList_Get( self->swarmList, name );
}

Swarm* Swarm_Register_At( void* swarmRegister, Index index ) {
	Swarm_Register* self      = (Swarm_Register *) swarmRegister;
	assert( self );

	return (Swarm*) Stg_ObjectList_At( self->swarmList, index );
}
	
unsigned int Swarm_Register_GetCount( Swarm_Register* self )
{
	assert( self );

	return self->swarmList->count;
}

Index Swarm_Register_GetIndexFromPointer( Swarm_Register* self, void* ptr ) {
	Index i;

	for ( i = 0; i < self->swarmList->count; ++i ) {
		if ( (void*)self->swarmList->data[i] == ptr ) {
			return i;
		}
	}

	return (Index)-1;
}


Swarm_Register* Swarm_Register_GetSwarm_Register() {
	return stgSwarm_Register;
}


void Swarm_Register_SaveAllRegisteredSwarms( Swarm_Register* self, void* context ) {
	Index                     swarmCount = self->swarmList->count;
	Swarm**                   swarmList = NULL;
	Index                     swarm_I;
	Index                     swarmsToDumpCount = 0;
	SwarmDump*                swarmDumper;
	Swarm*                    swarm;
	Stream*                   info = Journal_Register( Info_Type, self->type );

	if ( swarmCount == 0 ) {
		return;
	}	
	
	Journal_Printf( info, "In %s(): about to save the swarms to disk:\n", __func__ );

	swarmList = Memory_Alloc_Array_Unnamed( Swarm*, swarmCount ); 

	for ( swarm_I=0 ; swarm_I < swarmCount; swarm_I++ ) {
		swarm = Swarm_Register_At( self, swarm_I );
		if ( swarm->isSwarmTypeToCheckPointAndReload == True ) {
			swarmList[swarmsToDumpCount++] = swarm;
		}
	}
	
	if ( swarmsToDumpCount == 0 ) {
		Journal_Printf( info, "found 0 swarms that need to be checkpointed -> nothing to do\n",
			swarmsToDumpCount );
	}
	else {
		Journal_Printf( info, "\t(found %u swarms that need to be checkpointed)\n",
			swarmsToDumpCount );

		/* Create new SwarmDump object to check point the swarms
		 * We're not passing in the 'context' as the second argument because we don't need the swarm dumper to 
		 * add a hook on the Save entrypoint - contexts can manually execute this function */ 
		swarmDumper = SwarmDump_New( "swarmDumper", NULL, swarmList, swarmsToDumpCount, True );
		/* Passing the context through allows the swarmDumper to check requested strings etc */
		SwarmDump_Execute( swarmDumper, context );
		Stg_Class_Delete( swarmDumper );
	}

	Memory_Free( swarmList );
	Journal_Printf( info, "%s: saving of swarms completed.\n", __func__ );
}

