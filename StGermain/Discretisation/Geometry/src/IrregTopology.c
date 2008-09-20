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
** $Id: IrregTopology.c 3882 2006-10-26 04:41:18Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"


#include "units.h"
#include "types.h"
#include "Topology.h"
#include "IrregTopology.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


/* Textual name of this class */
const Type IrregTopology_Type = "IrregTopology";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

IrregTopology* IrregTopology_DefaultNew( Name name )
{
	return (IrregTopology*)_IrregTopology_New( 
		sizeof(IrregTopology), 
		IrregTopology_Type, 
		_IrregTopology_Delete, 
		_IrregTopology_Print,
		NULL,
		(Stg_Component_DefaultConstructorFunction*)IrregTopology_DefaultNew,
		_IrregTopology_Construct,
		_IrregTopology_Build,
		_IrregTopology_Initialise,
		_IrregTopology_Execute,
		_IrregTopology_Destroy,
		name,
		False,
		_IrregTopology_NeighbourCount,
		_IrregTopology_BuildNeighbours,
		NULL,
		NULL );
}

IrregTopology* IrregTopology_New(
		Name						name,
		Dictionary*					dictionary,
		Name						listKey )
{
	return _IrregTopology_New( 
		sizeof(IrregTopology), 
		IrregTopology_Type, 
		_IrregTopology_Delete, 
		_IrregTopology_Print,
		NULL,
		(Stg_Component_DefaultConstructorFunction*)IrregTopology_DefaultNew,
		_IrregTopology_Construct,
		_IrregTopology_Build,
		_IrregTopology_Initialise,
		_IrregTopology_Execute,
		_IrregTopology_Destroy,
		name,
		True,
		_IrregTopology_NeighbourCount,
		_IrregTopology_BuildNeighbours,
		dictionary,
		listKey );
}


void IrregTopology_Init(
		IrregTopology*					self,
		Name						name,
		Dictionary*					dictionary,
		Name						listKey )
{
	/* General info */
	self->type = IrregTopology_Type;
	self->_sizeOfSelf = sizeof(IrregTopology);
	self->_deleteSelf = False;
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->_delete = _IrregTopology_Delete;
	self->_print = _IrregTopology_Print;
	self->_copy = NULL;
	self->_defaultConstructor = (Stg_Component_DefaultConstructorFunction*)IrregTopology_DefaultNew;
	self->_construct = _IrregTopology_Construct;
	self->_build = _IrregTopology_Build;
	self->_initialise = _IrregTopology_Initialise;
	self->_execute = _IrregTopology_Execute;
	self->_destroy = _IrregTopology_Destroy;
	self->neighbourCount = _IrregTopology_NeighbourCount;
	self->buildNeighbours = _IrregTopology_BuildNeighbours;
	
	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	_Stg_Component_Init( (Stg_Component*)self );
	
	/* IrregTopology info */
	_IrregTopology_Init( self, listKey );
}


IrregTopology* _IrregTopology_New(
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
		Name						listKey )
{
	IrregTopology* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(IrregTopology) );
	self = (IrregTopology*)_Topology_New( _sizeOfSelf, type, _delete, _print, _copy, _defaultConstructor,
			_construct, _build, _initialise, _execute, _destroy, name, initFlag, neighbourCount, buildNeighbours );
	
	/* General info */
	self->dictionary = dictionary;
	
	/* Virtual info */
	
	/* IrregTopology info */
	if( initFlag ){
		_IrregTopology_Init( self, listKey );
	}
	
	return self;
}

void _IrregTopology_Init(
		IrregTopology*					self,
		Name						listKey )
{
	/* General and Virtual info should already be set */
	
	/* IrregTopology info */
	Dictionary_Entry_Value*		list;
	Dictionary_Entry_Value*		index;
	Dictionary_Entry_Value*		nbr;
	Index				i;
	
	self->isConstructed = True;
	list = Dictionary_Get( self->dictionary, listKey );
	self->indexCount = Dictionary_Entry_Value_GetCount( list );
	self->neighbourCountTbl = Memory_Alloc_Array( Index, self->indexCount, "IrregTopology->neighbourCountTbl" );
	self->neighbourTbl = Memory_Alloc_Array( Index*, self->indexCount, "IrregTopology->neighbourCountTbl" );
	index = Dictionary_Entry_Value_GetFirstElement( list );
	
	for( i = 0; i < self->indexCount; i++ ) {
		Index n_I;
		
		self->neighbourCountTbl[i] = Dictionary_Entry_Value_GetCount( index );
		nbr = Dictionary_Entry_Value_GetFirstElement( index );
		self->neighbourTbl[i] = Memory_Alloc_Array( Index, self->neighbourCountTbl[i], "IrregTopology->neighbourTbl[]" );
		
		for( n_I = 0; n_I < self->neighbourCountTbl[i]; n_I++ ) {
			self->neighbourTbl[i][n_I] = Dictionary_Entry_Value_AsUnsignedInt( nbr );
			nbr = nbr->next;
		}
		
		index = index->next;
	}
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _IrregTopology_Delete( void* irregTopology ) {
	IrregTopology*  self = (IrregTopology*)irregTopology;
	Index		i;
	
	for( i = 0; i < self->indexCount; i++ )
		if( self->neighbourTbl[i] )
			Memory_Free( self->neighbourTbl[i] );
	if( self->neighbourTbl ) {
		Memory_Free( self->neighbourTbl );
		self->neighbourTbl = NULL;
	}
	
	if( self->neighbourCountTbl ) {
		Memory_Free( self->neighbourCountTbl );
		self->neighbourCountTbl = NULL;
	}
	
	/* Stg_Class_Delete parent */
	_Topology_Delete( self );
}


void _IrregTopology_Print( void* irregTopology, Stream* stream ) {
	IrregTopology* self = (IrregTopology*)irregTopology;
	
	/* Set the Journal for printing informations */
	Stream* irregTopologyStream;
	irregTopologyStream = Journal_Register( InfoStream_Type, "IrregTopologyStream" );

	/* Print parent */
	_Topology_Print( self, stream );
	
	/* General info */
	Journal_Printf( stream, "IrregTopology (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* IrregTopology info */
}

void _IrregTopology_Construct( void* irregTopology, Stg_ComponentFactory *cf, void* data ){
	IrregTopology *self = (IrregTopology*)irregTopology;
	Dictionary *componentDict = NULL;
	Dictionary *thisComponentDict = NULL;
	Name listName = NULL;

	self->dictionary = cf->rootDict;
	componentDict = cf->componentDict;
	assert( componentDict );
	thisComponentDict = Dictionary_GetDictionary( componentDict, self->name );
	assert( thisComponentDict );
	
	listName = Dictionary_GetString( thisComponentDict, "ListName" );

	_Topology_Init( (Topology*)self );
	_IrregTopology_Init( self, listName );
}

void _IrregTopology_Build( void* irregTopology, void* data ){
	
}
	
void _IrregTopology_Initialise( void* irregTopology, void* data ){
	
}
	
void _IrregTopology_Execute( void* irregTopology, void* data ){
	
}
	
void _IrregTopology_Destroy( void* irregTopology, void* data ){
	
}


NeighbourIndex _IrregTopology_NeighbourCount( void* irregTopology, Index index ) {
	IrregTopology* self = (IrregTopology*)irregTopology;

	return self->neighbourCountTbl[index];
}


void _IrregTopology_BuildNeighbours( void* irregTopology, Index index, NeighbourIndex* neighbours ) {
	IrregTopology*  self = (IrregTopology*)irregTopology;
	NeighbourIndex  n_I;
	
	for( n_I = 0; n_I < self->neighbourCountTbl[index]; n_I++ )
		neighbours[n_I] = self->neighbourTbl[index][n_I];
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

