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
** $Id: IJK26Topology.c 3882 2006-10-26 04:41:18Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"


#include "units.h"
#include "types.h"
#include "Topology.h"
#include "IJKTopology.h"
#include "IJK26Topology.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>


/* Textual name of this class */
const Type IJK26Topology_Type = "IJK26Topology";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

IJK26Topology* IJK26Topology_DefaultNew( Name name )
{
	return (IJK26Topology*)_IJKTopology_New( 
		sizeof(IJK26Topology), 
		IJK26Topology_Type, 
		_IJK26Topology_Delete, 
		_IJK26Topology_Print,
		_IJKTopology_Copy,
		(Stg_Component_DefaultConstructorFunction*)IJK26Topology_DefaultNew,
		_IJK26Topology_Construct,
		_IJK26Topology_Build,
		_IJK26Topology_Initialise,
		_IJK26Topology_Execute,
		_IJK26Topology_Destroy,
		name,
		False,
		_IJK26Topology_NeighbourCount,
		_IJK26Topology_BuildNeighbours,
		NULL,
		NULL,
		NULL );
}

IJK26Topology* IJK26Topology_New_All(
		Name						name,
		Dictionary*					dictionary,
		IJK						size,
		Bool						isPeriodic[3] )
{
	return _IJK26Topology_New( 
		sizeof(IJK26Topology), 
		IJK26Topology_Type, 
		_IJK26Topology_Delete, 
		_IJK26Topology_Print,
		_IJKTopology_Copy,
		(Stg_Component_DefaultConstructorFunction*)IJK26Topology_DefaultNew,
		_IJK26Topology_Construct,
		_IJK26Topology_Build,
		_IJK26Topology_Initialise,
		_IJK26Topology_Execute,
		_IJK26Topology_Destroy,
		name,
		True,
		_IJK26Topology_NeighbourCount,
		_IJK26Topology_BuildNeighbours,
		dictionary,
		size,
		isPeriodic );
}


void IJK26Topology_Init(
		IJK26Topology*					self,
		Name						name,
		Dictionary*					dictionary,
		IJK						size,
		Bool						isPeriodic[3] )
{
	/* General info */
	self->type = IJK26Topology_Type;
	self->_sizeOfSelf = sizeof(IJK26Topology);
	self->_deleteSelf = False;
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->_delete = _IJK26Topology_Delete;
	self->_print = _IJK26Topology_Print;
	self->_copy = _IJKTopology_Copy;
	self->_defaultConstructor = (Stg_Component_DefaultConstructorFunction*)IJK26Topology_DefaultNew;
	self->_construct = _IJK26Topology_Construct;
	self->_build = _IJK26Topology_Build;
	self->_initialise = _IJK26Topology_Initialise;
	self->_execute = _IJK26Topology_Execute;
	self->_destroy = _IJK26Topology_Destroy;
	self->neighbourCount = _IJK26Topology_NeighbourCount;
	self->buildNeighbours = _IJK26Topology_BuildNeighbours;
	
	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	_Stg_Component_Init( (Stg_Component*)self );

	/* IJK26Topology info */
	_Topology_Init( (Topology*)self );
	_IJKTopology_Init( (IJKTopology*)self, size, isPeriodic, False );
	_IJK26Topology_Init( self, True );
}


IJK26Topology* _IJK26Topology_New(
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
	IJK26Topology* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(IJK26Topology) );
	self = (IJK26Topology*)_IJKTopology_New( _sizeOfSelf, type, _delete, _print, _copy, _defaultConstructor,
			_construct, _build, _initialise, _execute, _destroy, name, initFlag, neighbourCount, buildNeighbours,
			dictionary, size, isPeriodic );
	
	/* General info */
	self->dictionary = dictionary;
	
	/* Virtual info */
	
	/* IJK26Topology info */
	if( initFlag ){
		_IJK26Topology_Init( self, True );
	}
	
	return self;
}


void _IJK26Topology_Init( IJK26Topology* self, Bool shiftNegOne )
{
	Index i;

	if (self->dictionary) {
		self->isConstructed = True;

		/* Since the default is for elements, do a slight adjustment */
		for (i=I_AXIS; i < 3; i++ ) {
			if (shiftNegOne)
				self->size[i] -= 1;

			if ( self->size[i] == 0 )
				self->size[i] = 1;
		}
		
		self->indexCount = self->size[I_AXIS] * self->size[J_AXIS] * self->size[K_AXIS];
		assert( self->indexCount );
	}

	return;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _IJK26Topology_Delete( void* ijk26Topology ) {
	IJK26Topology* self = (IJK26Topology*)ijk26Topology;
	
	/* Stg_Class_Delete parent */
	_IJKTopology_Delete( self );
}


void _IJK26Topology_Print( void* ijk26Topology, Stream* stream ) {
	IJK26Topology* self = (IJK26Topology*)ijk26Topology;
	
	/* Set the Journal for printing informations */
	Stream* ijk26TopologyStream = Journal_Register( InfoStream_Type, "IJK26TopologyStream" );

	/* Print parent */
	_Topology_Print( self, stream );
	
	/* General info */
	Journal_Printf( ijk26TopologyStream, "IJK26Topology (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* IJK26Topology info */
}

void _IJK26Topology_Construct( void* ijk26Topology, Stg_ComponentFactory* cf, void* data ){
	IJK26Topology *self = (IJK26Topology*)ijk26Topology;

	_IJKTopology_Construct( self, cf, data );

	/* No shifting sizes of topology because no assumption is made about elements */
	_IJK26Topology_Init( self, False );
}

void _IJK26Topology_Build( void* ijk26Topology, void* data ){
	
}
	
void _IJK26Topology_Initialise( void* ijk26Topology, void* data ){
	
}
	
void _IJK26Topology_Execute( void* ijk26Topology, void* data ){
	
}
	
void _IJK26Topology_Destroy( void* ijk26Topology, void* data ){
	
}

/* See header file for docs */
NeighbourIndex _IJK26Topology_NeighbourCount( void* ijk26Topology, Index index ) {
	IJK26Topology*  self = (IJK26Topology*)ijk26Topology;
	IJK		max;		/* init later */
	IJK		nbrPerDim = { 3, 3, 3 };
	IJK		refObject;
	Index	i;
	
	max[0] = self->size[I_AXIS] - 1;
	max[1] = self->size[J_AXIS] - 1;
	max[2] = self->size[K_AXIS] - 1;
	 
	IJK_1DTo3D( self, index, refObject );
	
	for( i=I_AXIS; i < 3; i++ ) {

		if( 0 == max[i] )
			nbrPerDim[i] = 1;

		/* Ok given there is actually more than one item in this dim:
		only if we are using "dynamic sizes" ie just want the good values to
		be returned from BuildNeighbours and don't want entries in the array with
		an INVALID tag, do we need to see if the size should be reduced */
		else if( self->dynamicSizes ) {
			if ( ( False == self->isPeriodic[i] )
				&& (refObject[i] == 0 || refObject[i] == max[i]) )
			{
				nbrPerDim[i]--;
			}	
		}	
	}	
	
	return nbrPerDim[I_AXIS] * nbrPerDim[J_AXIS] * nbrPerDim[K_AXIS] - 1;
}


void _IJK26Topology_BuildNeighbours( void* ijk26Topology, Index index, NeighbourIndex* neighbours )
{
	IJK26Topology*	self = (IJK26Topology*)ijk26Topology;
	IJK		max; 		/* init later */
	IJK		refObject;
	Index		pos = 0;
	int		nbrRelative[3];
	Index	dim_I;
	Bool	nbrExists = True;

	max[0] = self->size[0] - 1;
	max[1] = self->size[1] - 1;
	max[2] = self->size[2] - 1;

	IJK_1DTo3D( self, index, refObject );

	/* For each neighbour position relative to the reference object */
	for ( nbrRelative[K_AXIS] = -1; nbrRelative[K_AXIS] <= 1; nbrRelative[K_AXIS] ++ ) {
		if ( (0 == max[K_AXIS]) && (nbrRelative[K_AXIS] != 0) ) continue; 
		
		for ( nbrRelative[J_AXIS] = -1; nbrRelative[J_AXIS] <= 1; nbrRelative[J_AXIS]++ ) {
			if ( (0 == max[J_AXIS]) && (nbrRelative[J_AXIS] != 0) ) continue; 
		
			for ( nbrRelative[I_AXIS] = -1; nbrRelative[I_AXIS] <= 1; nbrRelative[I_AXIS]++ ) {
				if ( (0 == max[I_AXIS]) && (nbrRelative[I_AXIS] != 0) ) continue; 

				if ( ( 0 == nbrRelative[I_AXIS] ) && ( 0 == nbrRelative[J_AXIS] ) && ( 0 == nbrRelative[K_AXIS] )  ) {
					continue;
				}
				
				nbrExists = True;
				/* Test if the neigbour exists or not, and modify the index appropriately */
				for ( dim_I=0; dim_I < 3; dim_I++ ) {
					if ( (False == self->isPeriodic[dim_I] ) &&
						( (( refObject[dim_I] == 0 ) && (-1 == nbrRelative[dim_I] )) ||
						(( refObject[dim_I] == max[dim_I] ) && (1 == nbrRelative[dim_I] )) ) )
					{
						nbrExists = False;
						break;
					}
				}	
				if (nbrExists) {
					IJK     nbrPos;
					Dimension_Index	innerDim_I;
				
					/* Find out the object's number */

					for ( innerDim_I=0; innerDim_I < 3; innerDim_I++ ) {
						if ( self->isPeriodic[innerDim_I] && (refObject[innerDim_I] == 0)
							&& (-1 == nbrRelative[innerDim_I] ) )
						{
							nbrPos[innerDim_I] = max[innerDim_I];
						}
						else if ( self->isPeriodic[innerDim_I]
							&& (refObject[innerDim_I] == max[innerDim_I] )
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
				else {
					if( False == self->dynamicSizes ) {
						/* Set the object to invalid if using dynamic sizes */
						neighbours[pos] = Topology_Invalid( self );
						pos++;
					}	
				}
			}	
		}	
	}
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void IJK26Topology_PrintNeighboursOfIndex( IJK26Topology* self, Index refIndex, Stream* stream ) {
	NeighbourIndex nbrCount;
	NeighbourIndex* neighbours;

	nbrCount = Topology_NeighbourCount( self, refIndex );
	neighbours = Memory_Alloc_Array( NeighbourIndex, nbrCount, "neighbours" );
	Topology_BuildNeighbours( self, refIndex, neighbours );
	
	IJK26Topology_PrintNeighbourOfIndexFromArray( self, refIndex, neighbours, nbrCount, stream );
	Memory_Free( neighbours );
}


void IJK26Topology_PrintNeighbourOfIndexFromArray(
		IJK26Topology* self,
		Index refIndex,
		NeighbourIndex* neighbours,
		NeighbourIndex nbrCount,
		Stream* stream )
{		
	if ( True == self->dynamicSizes )
	{
		Index nbr_I;

		Journal_Printf( stream, "[%d] = {", nbrCount );
		for ( nbr_I=0; nbr_I < nbrCount; nbr_I++ ) {
			Journal_Printf( stream, "%d", neighbours[nbr_I] );
			if ( neighbours[nbr_I] == Topology_Invalid( self ) ) {
				Journal_Printf( stream, "(Inv)");
			}
			Journal_Printf( stream, ", ");
		}
		Journal_Printf( stream, "}\n" );
	} 
	else {
		Dimension_Index  dimension = 0;
		Dimension_Index  dim_I=0;

		for ( dim_I=0; dim_I < 3; dim_I++ ) {
			if (self->size[dim_I] > 1) {
				dimension++;
			}
		}
		if ( 0 == dimension ) {
			Journal_Printf( stream, "|" );
			Journal_Printf( stream, "(%3d  )", refIndex );
			Journal_Printf( stream, "|" );
			Journal_Printf( stream, "\n" );
		}
		else if ( 1 == dimension ) {
			IJK26Topology_PrintRowSeparator( self, stream );
			IJK26Topology_PrintMiddleRow( self, neighbours, 0, refIndex, 1, stream );
			IJK26Topology_PrintRowSeparator( self, stream );
		}
		else if ( 2 == dimension ) {
			IJK26Topology_PrintRowSeparator( self, stream );
			IJK26Topology_PrintNormalRow( self, neighbours, 5, 6, 7, stream );
			IJK26Topology_PrintRowSeparator( self, stream );
			IJK26Topology_PrintMiddleRow( self, neighbours, 3, refIndex, 4, stream );
			IJK26Topology_PrintRowSeparator( self, stream );
			IJK26Topology_PrintNormalRow( self, neighbours, 0, 1, 2, stream );
			IJK26Topology_PrintRowSeparator( self, stream );
		}
		else {
			/* 3D: do it in 3 separate goes: */
			Journal_Printf( stream, "-1 in 3rd dim plane\n" );
			Stream_Indent( stream );
			IJK26Topology_PrintRowSeparator( self, stream );
			IJK26Topology_PrintNormalRow( self, neighbours, 6, 7, 8, stream );
			IJK26Topology_PrintRowSeparator( self, stream );
			IJK26Topology_PrintNormalRow( self, neighbours, 3, 4, 5, stream );
			IJK26Topology_PrintRowSeparator( self, stream );
			IJK26Topology_PrintNormalRow( self, neighbours, 0, 1, 2, stream );
			IJK26Topology_PrintRowSeparator( self, stream );
			Stream_UnIndent( stream );
			Journal_Printf( stream, "ref item's plane in 3rd dim\n" );
			Stream_Indent( stream );
			IJK26Topology_PrintRowSeparator( self, stream );
			IJK26Topology_PrintNormalRow( self, neighbours, 14, 15, 16, stream );
			IJK26Topology_PrintRowSeparator( self, stream );
			IJK26Topology_PrintMiddleRow( self, neighbours, 12, refIndex, 13, stream );
			IJK26Topology_PrintRowSeparator( self, stream );
			IJK26Topology_PrintNormalRow( self, neighbours, 9, 10, 11, stream );
			IJK26Topology_PrintRowSeparator( self, stream );
			Stream_UnIndent( stream );
			Journal_Printf( stream, "+1 3rd dim\n" );
			Stream_Indent( stream );
			IJK26Topology_PrintRowSeparator( self, stream );
			IJK26Topology_PrintNormalRow( self, neighbours, 23, 24, 25, stream );
			IJK26Topology_PrintRowSeparator( self, stream );
			IJK26Topology_PrintNormalRow( self, neighbours, 20, 21, 22, stream );
			IJK26Topology_PrintRowSeparator( self, stream );
			IJK26Topology_PrintNormalRow( self, neighbours, 17, 18, 19, stream );
			IJK26Topology_PrintRowSeparator( self, stream );
			Stream_UnIndent( stream );
		}
	}
}	


void IJK26Topology_PrintRowSeparator( IJK26Topology* self, Stream* stream ) {  	
	Journal_Printf( stream, "-------------------------\n" );
}


void IJK26Topology_PrintNeighbour( IJK26Topology* topology, Index itemIndex, Stream* stream ) {  	
	if ( itemIndex == Topology_Invalid( topology ) ) {
		Journal_Printf( stream, "   X   ", itemIndex );
	}
	else {	
		Journal_Printf( stream, "%4d   ", itemIndex );
	}
}	


void IJK26Topology_PrintNormalRow( IJK26Topology* topology, NeighbourIndex* neighbours,
		Index nbr1, Index nbr2, Index nbr3, Stream* stream )
{
	/* Do middle row */
	Journal_Printf( stream, "|" );
	IJK26Topology_PrintNeighbour( topology, neighbours[nbr1], stream );
	Journal_Printf( stream, "|" );
	IJK26Topology_PrintNeighbour( topology, neighbours[nbr2], stream );
	Journal_Printf( stream, "|" );
	IJK26Topology_PrintNeighbour( topology, neighbours[nbr3], stream );
	Journal_Printf( stream, "|" );
	Journal_Printf( stream, "\n" );
}


void IJK26Topology_PrintMiddleRow( IJK26Topology* topology, NeighbourIndex* neighbours,
		Index leftNbr, Index refPos, Index rightNbr, Stream* stream )
{
	/* Do middle row */
	Journal_Printf( stream, "|" );
	IJK26Topology_PrintNeighbour( topology, neighbours[leftNbr], stream );
	Journal_Printf( stream, "|" );
	/* represents the current one */
	Journal_Printf( stream, "(%3d  )", refPos );
	Journal_Printf( stream, "|" );
	IJK26Topology_PrintNeighbour( topology, neighbours[rightNbr], stream );
	Journal_Printf( stream, "|" );
	Journal_Printf( stream, "\n" );
}

/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

