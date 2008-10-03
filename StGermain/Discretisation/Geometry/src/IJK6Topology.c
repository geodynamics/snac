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
** $Id: IJK6Topology.c 3882 2006-10-26 04:41:18Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"


#include "units.h"
#include "types.h"
#include "Topology.h"
#include "IJKTopology.h"
#include "IJK6Topology.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


/* Textual name of this class */
const Type IJK6Topology_Type = "IJK6Topology";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

IJK6Topology* IJK6Topology_DefaultNew( Name name )
{
	return _IJK6Topology_New( 
		sizeof(IJK6Topology), 
		IJK6Topology_Type, 
		_IJK6Topology_Delete, 
		_IJK6Topology_Print,
		_IJKTopology_Copy,
                (Stg_Component_DefaultConstructorFunction*)IJK6Topology_DefaultNew,
		_IJK6Topology_Construct,
		_IJK6Topology_Build,
		_IJK6Topology_Initialise,
		_IJK6Topology_Execute,
		_IJK6Topology_Destroy,
		name,
		False,
		_IJK6Topology_NeighbourCount,
		_IJK6Topology_BuildNeighbours,
		NULL,
		NULL,
		NULL );
}

IJK6Topology* IJK6Topology_New_All(
		Name						name,
		Dictionary*					dictionary,
		IJK						size,
		Bool						isPeriodic[3] )
{
	return _IJK6Topology_New( 
		sizeof(IJK6Topology), 
		IJK6Topology_Type, 
		_IJK6Topology_Delete, 
		_IJK6Topology_Print,
		_IJKTopology_Copy,
		(Stg_Component_DefaultConstructorFunction*)IJK6Topology_DefaultNew,
		_IJK6Topology_Construct,
		_IJK6Topology_Build,
		_IJK6Topology_Initialise,
		_IJK6Topology_Execute,
		_IJK6Topology_Destroy,
		name,
		True,
		_IJK6Topology_NeighbourCount,
		_IJK6Topology_BuildNeighbours,
		dictionary,
		size,
		isPeriodic );
}

void IJK6Topology_Init(
		IJK6Topology*					self,
		Name						name,
		Dictionary*					dictionary,
		IJK						size,
		Bool						isPeriodic[3] )
{
	/* General info */
	self->type = IJK6Topology_Type;
	self->_sizeOfSelf = sizeof(IJK6Topology);
	self->_deleteSelf = False;
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->_delete = _IJK6Topology_Delete;
	self->_print = _IJK6Topology_Print;
	self->_copy = _IJKTopology_Copy;
	self->_defaultConstructor = (Stg_Component_DefaultConstructorFunction*)IJK6Topology_DefaultNew;
	self->_construct = _IJK6Topology_Construct;
	self->_build = _IJK6Topology_Build,
	self->_initialise = _IJK6Topology_Initialise;
	self->_execute = _IJK6Topology_Execute;
	self->_destroy = _IJK6Topology_Destroy;
	self->neighbourCount = _IJK6Topology_NeighbourCount;
	self->buildNeighbours = _IJK6Topology_BuildNeighbours;

	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	_Stg_Component_Init( (Stg_Component*)self );
	
	/* IJK6Topology info */
	_Topology_Init( (Topology*)self );
	_IJKTopology_Init( (IJKTopology*)self, size, isPeriodic, False );
	_IJK6Topology_Init( self );
}


IJK6Topology* _IJK6Topology_New(
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
		IJK					size,
		Bool					isPeriodic[3] )
{
	IJK6Topology* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(IJK6Topology) );
	self = (IJK6Topology*)_IJKTopology_New( _sizeOfSelf, type, _delete, _print, _copy, _defaultConstructor,
			_construct, _build, _initialise, _execute, _destroy, name, initFlag, neighbourCount, buildNeighbours,
			dictionary, size, isPeriodic );

	/* General info */
	self->dictionary = dictionary;
	
	/* Virtual info */
	
	/* IJK6Topology info */
	if( initFlag ){
		_IJK6Topology_Init( self );
	}
	
	return self;
}

void _IJK6Topology_Init(
		IJK6Topology*					self )
{
	self->isConstructed = True;
	return;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _IJK6Topology_Delete( void* ijk6Topology ) {
	IJK6Topology* self = (IJK6Topology*)ijk6Topology;
	
	/* Stg_Class_Delete parent */
	_Topology_Delete( self );
}


void _IJK6Topology_Print( void* ijk6Topology, Stream* stream ) {
	IJK6Topology* self = (IJK6Topology*)ijk6Topology;
	
	/* Set the Journal for printing informations */
	Stream* ijk6TopologyStream = Journal_Register( InfoStream_Type, "IJK6TopologyStream" );

	/* Print parent */
	_Topology_Print( self, stream );
	
	/* General info */
	Journal_Printf( ijk6TopologyStream, "IJK6Topology (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* IJK6Topology info */
}

void _IJK6Topology_Construct( void* ijk6Topology, Stg_ComponentFactory* cf, void* data ){
	IJK6Topology *self = (IJK6Topology*)ijk6Topology;

	_IJKTopology_Construct( self, cf, data );
	_IJK6Topology_Init( self );
}

void _IJK6Topology_Build( void* ijk6Topology, void* data ){
	
}
	
void _IJK6Topology_Initialise( void* ijk6Topology, void* data ){
	
}
	
void _IJK6Topology_Execute( void* ijk6Topology, void* data ){
	
}
	
void _IJK6Topology_Destroy( void* ijk6Topology, void* data ){
	
}

NeighbourIndex _IJK6Topology_NeighbourCount( void* ijk6Topology, Index index ) {
	IJK6Topology*   self = (IJK6Topology*)ijk6Topology;
	IJK		max;			/* init later */
	Index		nbrCnt = 6;
	Index		dim_I;
	IJK		refObject;
	int		nbrRelative[3];
	int		sign;
	
	max[0] = self->size[0]-1;
	max[1] = self->size[1]-1;
	max[2] = self->size[2]-1;
	 
	IJK_1DTo3D( self, index, refObject );

	for ( sign = 1; sign >= -1; sign -= 2 ) {
		for (dim_I=I_AXIS; dim_I < 3; dim_I++) {
			if( self->size[dim_I] == 1 ) {
				nbrCnt--;
				continue;
			}	

			if( True == self->dynamicSizes ) {
				nbrRelative[I_AXIS] = nbrRelative[J_AXIS] = nbrRelative[K_AXIS] = 0;
				nbrRelative[dim_I] = sign;

				if ( ( False == self->isPeriodic[dim_I] ) &&
					( (( refObject[dim_I] == 0 ) && (-1 == nbrRelative[dim_I] ) ) ||
					(( refObject[dim_I] == max[dim_I] ) && (1 == nbrRelative[dim_I] )) ) )
				{
					nbrCnt--;
				}
			}	
		}
	}	
	
	return nbrCnt;
}


void _IJK6Topology_BuildNeighbours( void* ijk6Topology, Index index, NeighbourIndex* neighbours )
{
	IJK6Topology*	self = (IJK6Topology*)ijk6Topology;
	IJK		max;			/* init later */
	int		sign;
	Index	dim_I;
	IJK		refObject;
	int		nbrRelative[3];
	Index	pos = 0;
	
	max[0] = self->size[0]-1;
	max[1] = self->size[1]-1;
	max[2] = self->size[2]-1;
 
	IJK_1DTo3D( self, index, refObject );

	for ( sign = 1; sign >= -1; sign -= 2 ) {
		for ( dim_I = I_AXIS; dim_I < 3; dim_I++ ) {
			if ( 0 == max[dim_I] ) continue;
		
			nbrRelative[I_AXIS] = nbrRelative[J_AXIS] = nbrRelative[K_AXIS] = 0;
			nbrRelative[dim_I] = sign;

			if ( ( False == self->isPeriodic[dim_I] ) &&
				( ( ( refObject[dim_I] == 0 ) && (-1 == nbrRelative[dim_I] ) ) ||
				( ( refObject[dim_I] == max[dim_I] ) && (1 == nbrRelative[dim_I] ) ) ) )
			{
				if( False == self->dynamicSizes ) {
					/* Set the object to invalid if using dynamic sizes */
					neighbours[pos] = Topology_Invalid( self );
					pos++;
				}	
			}
			else {
				IJK nbrPos;
				Dimension_Index	innerDim_I;

				for ( innerDim_I=0; innerDim_I < 3; innerDim_I++ ) {
					if ( self->isPeriodic[innerDim_I] && (refObject[innerDim_I] == 0)
						&& (-1 == nbrRelative[innerDim_I] ) )
					{
						nbrPos[innerDim_I] = max[innerDim_I];
					}
					else if ( self->isPeriodic[innerDim_I] && (refObject[innerDim_I] == max[innerDim_I] )
						&& (1 == nbrRelative[innerDim_I] ) )
					{
						nbrPos[innerDim_I] = 0;
					}
					else {
						nbrPos[innerDim_I] = refObject[innerDim_I] + nbrRelative[innerDim_I];
					}
				}	

				IJK_3DTo1D_3( self, 
					nbrPos[I_AXIS],
					nbrPos[J_AXIS],
					nbrPos[K_AXIS],
					&neighbours[pos] );
				pos++;
			}
		}
	}	
}
