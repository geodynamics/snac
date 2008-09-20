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
** $Id: IrregEL.c 3883 2006-10-26 05:00:23Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"

#include "types.h"
#include "shortcuts.h"
#include "ElementLayout.h"
#include "IrregEL.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <mpi.h>



/* Textual name of this class */
const Type IrregEL_Type = "IrregEL";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

IrregEL* IrregEL_DefaultNew( Name name )
{
	return (IrregEL*)_IrregEL_New( 
		sizeof(IrregEL), 
		IrregEL_Type, 
		_IrregEL_Delete, 
		_IrregEL_Print,
		NULL,
		(Stg_Component_DefaultConstructorFunction*)IrregEL_DefaultNew,
		_IrregEL_Construct,
		_IrregEL_ComponentBuild,
		_IrregEL_Initialise,
		_IrregEL_Execute,
		_IrregEL_Destroy,
		name,
		False,
		_ElementLayout_Build,
		_IrregEL_BuildCornerIndices,
		_IrregEL_CornerElementCount,
		_IrregEL_BuildCornerElements,
		_IrregEL_BuildEdgeIndices,
		_IrregEL_EdgeElementCount,
		_IrregEL_BuildEdgeElements,
		_IrregEL_EdgeAt,
		_ElementLayout_GetStaticMinAndMaxLocalCoords,
		_ElementLayout_GetStaticMinAndMaxLocalCoords,		
		_IrregEL_ElementWithPoint,
		NULL,
		NULL,
		NULL,
		NULL );
}

IrregEL* IrregEL_New(
		Name						name,
		Dictionary*					dictionary,
		Geometry*					geometry,
		Topology*					topology,
		Name						listKey )
{
	return _IrregEL_New( 
		sizeof(IrregEL), 
		IrregEL_Type, 
		_IrregEL_Delete, 
		_IrregEL_Print,
		NULL,
		(Stg_Component_DefaultConstructorFunction*)IrregEL_DefaultNew,
		_IrregEL_Construct,
		_IrregEL_ComponentBuild,
		_IrregEL_Initialise,
		_IrregEL_Execute,
		_IrregEL_Destroy,
		name,
		True,
		_ElementLayout_Build,
		_IrregEL_BuildCornerIndices,
		_IrregEL_CornerElementCount,
		_IrregEL_BuildCornerElements,
		_IrregEL_BuildEdgeIndices,
		_IrregEL_EdgeElementCount,
		_IrregEL_BuildEdgeElements,
		_IrregEL_EdgeAt,
		_ElementLayout_GetStaticMinAndMaxLocalCoords,
		_ElementLayout_GetStaticMinAndMaxLocalCoords,		
		_IrregEL_ElementWithPoint,
		dictionary,
		geometry,
		topology,
		listKey );
}


void IrregEL_Init(
		IrregEL*					self,
		Name						name,
		Dictionary*					dictionary,
		Geometry*					geometry,
		Topology*					topology,
		Name						listKey )
{
	/* General info */
	self->type = IrregEL_Type;
	self->_sizeOfSelf = sizeof(IrregEL);
	self->_deleteSelf = False;
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->_delete = _IrregEL_Delete;
	self->_print = _IrregEL_Print;
	self->_copy = NULL;
	self->_defaultConstructor = (Stg_Component_DefaultConstructorFunction*)IrregEL_DefaultNew;
	self->_construct = _IrregEL_Construct;
	self->_build = _IrregEL_ComponentBuild;
	self->_initialise = _IrregEL_Initialise;
	self->_execute = _IrregEL_Execute;
	self->_destroy = _IrregEL_Destroy;
	self->build = _ElementLayout_Build;
	self->buildCornerIndices = _IrregEL_BuildCornerIndices;
	self->cornerElementCount = _IrregEL_CornerElementCount;
	self->buildCornerElements = _IrregEL_BuildCornerElements;
	self->buildEdgeIndices = _IrregEL_BuildEdgeIndices;
	self->edgeElementCount = _IrregEL_EdgeElementCount;
	self->buildEdgeElements = _IrregEL_BuildEdgeElements;
	self->edgeAt = _IrregEL_EdgeAt;
	self->getStaticMinAndMaxLocalCoords = _ElementLayout_GetStaticMinAndMaxLocalCoords;
	self->getStaticMinAndMaxGlobalCoords = _ElementLayout_GetStaticMinAndMaxGlobalCoords;	
	self->elementWithPoint = _IrregEL_ElementWithPoint;

	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	_Stg_Component_Init( (Stg_Component*)self );
	_ElementLayout_Init( (ElementLayout*)self, geometry, topology );
	
	/* IrregEL info */
	_IrregEL_Init( self, listKey );
}


IrregEL* _IrregEL_New(
		SizeT							_sizeOfSelf, 
		Type							type,
		Stg_Class_DeleteFunction*					_delete,
		Stg_Class_PrintFunction*					_print,
		Stg_Class_CopyFunction*					_copy, 
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
		Stg_Component_ConstructFunction*			_construct,
		Stg_Component_BuildFunction*				_build,
		Stg_Component_InitialiseFunction*			_initialise,
		Stg_Component_ExecuteFunction*				_execute,
		Stg_Component_DestroyFunction*				_destroy,
		Name									name,
		Bool									initFlag,
		ElementLayout_BuildFunction*  				build,		
		ElementLayout_BuildCornerIndicesFunction*       	buildCornerIndices,
		ElementLayout_CornerElementCountFunction*       	cornerElementCount,
		ElementLayout_BuildCornerElementsFunction*      	buildCornerElements,
		ElementLayout_BuildEdgeIndicesFunction*			buildEdgeIndices,
		ElementLayout_EdgeElementCountFunction*			edgeElementCount,
		ElementLayout_BuildEdgeElementsFunction*		buildEdgeElements,
		ElementLayout_EdgeAtFunction*				edgeAt,
		ElementLayout_GetStaticMinAndMaxLocalCoordsFunction*	getStaticMinAndMaxLocalCoords,
		ElementLayout_GetStaticMinAndMaxGlobalCoordsFunction*	getStaticMinAndMaxGlobalCoords,				
		ElementLayout_ElementWithPointFunction*			elementWithPoint,
		Dictionary*						dictionary,
		Geometry*						geometry,		
		Topology*						topology,
		Name							listKey )
{
	IrregEL* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(IrregEL) );
	self = (IrregEL*)_ElementLayout_New(
		_sizeOfSelf,
		type,
		_delete,
		_print,
		_copy,
		_defaultConstructor,
		_construct,
		_build,
		_initialise,
		_execute,
		_destroy,
		name,
		initFlag,
		build,
		buildCornerIndices,
		cornerElementCount,
		buildCornerElements,
		buildEdgeIndices,
		edgeElementCount,
		buildEdgeElements,
		edgeAt,
		getStaticMinAndMaxLocalCoords,
		getStaticMinAndMaxGlobalCoords,		
		elementWithPoint,
		dictionary,
		geometry,
		topology );
	
	/* General info */
	
	/* Virtual info */
	
	/* IrregEL info */
	if ( initFlag ){
		_IrregEL_Init( self, listKey );
	}
	
	return self;
}

void _IrregEL_Init(
		IrregEL*					self,
		Name						listKey )
{
	/* General and Virtual info should already be set */
	
	/* IrregEL info */
	Dictionary_Entry_Value*		list;
	Dictionary_Entry_Value*		elementList;
	Dictionary_Entry_Value*		element;
	Dictionary_Entry_Value*		point;
	Index				e_I;
	
	self->isConstructed = True;
	list = Dictionary_Get( self->dictionary, listKey );
	self->elementCornerCount = Dictionary_Entry_Value_AsUnsignedInt( Dictionary_Entry_Value_GetMember( list, "elementCornerCount" ) );
	if ( 0 == self->elementCornerCount ) {
		fprintf( stderr, "Error - %s: Required Dictionary entry for this element layout\" elementCornerCount\" "
			"not found. Aborting.\n", __func__ );
		MPI_Finalize();
		exit(EXIT_FAILURE);
	}
	
	elementList = Dictionary_Entry_Value_GetMember( list, "elementList" );
	self->elementCount = Dictionary_Entry_Value_GetCount( elementList );
	
/* TEMPORARY. Until test are verified the following #ifdef will be used */
#ifdef USE_2D_CODE

	self->elementTbl = Memory_Alloc_2DArray( Index, self->elementCount, self->elementCornerCount, "IrregEL->elementTbl" );
		
	element = Dictionary_Entry_Value_GetFirstElement( elementList );		
	for( e_I = 0; e_I < self->elementCount; e_I++) {
		Index p_I;
		
		point = Dictionary_Entry_Value_GetFirstElement( element );
		for( p_I = 0; p_I < self->elementCornerCount; p_I++ ) {
			self->elementTbl[e_I][p_I] = Dictionary_Entry_Value_AsUnsignedInt( point );
			point = point->next;
		}
		
		element = element->next;	
	}

#else
	
	self->elementTbl = Memory_Alloc_Array( Index*, self->elementCount, "IrregEL->elementTbl" );

	element = Dictionary_Entry_Value_GetFirstElement( elementList );			
	for( e_I = 0; e_I < self->elementCount; e_I++ ) {
		Index p_I;
		
		point = Dictionary_Entry_Value_GetFirstElement( element );
		self->elementTbl[e_I] = Memory_Alloc_Array( Index, self->elementCornerCount, "IrregEL->elementTbl[]" );
		
		for( p_I = 0; p_I < self->elementCornerCount; p_I++ ) {
			self->elementTbl[e_I][p_I] = Dictionary_Entry_Value_AsUnsignedInt( point );
			point = point->next;
		}
		
		element = element->next;
	}
#endif

	self->cornerCount = self->geometry->pointCount;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _IrregEL_Delete( void* irregEL ) {
	IrregEL*		self = (IrregEL*)irregEL;
	Element_GlobalIndex     e_I;

#ifdef USE_2D_CODE

	Memory_Free( self->elementTbl );
	self->elementTbl = NULL;

#else
	
	for( e_I = 0; e_I < self->elementCount; e_I++ )
		if( self->elementTbl[e_I] )
			Memory_Free( self->elementTbl[e_I] );
	if( self->elementTbl ) {
		Memory_Free( self->elementTbl );
		self->elementTbl = NULL;
	}

#endif
	
	/* Stg_Class_Delete parent */
	_ElementLayout_Delete( self );
}


void _IrregEL_Print( void* irregEL, Stream* stream ) {
	IrregEL* self = (IrregEL*)irregEL;
	
	/* Set the Journal for printing informations */
	Stream* irregELStream;
	irregELStream = Journal_Register( InfoStream_Type, "IrregELStream" );

	/* Print parent */
	_ElementLayout_Print( self, stream );
	
	/* General info */
	Journal_Printf( stream, "IrregEL (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* IrregEL info */
}

void _IrregEL_Construct( void* irregEL, Stg_ComponentFactory* cf, void* data ){
	IrregEL *self = (IrregEL*)irregEL;
	Name listName = NULL;
	Topology *topology = NULL;
	Geometry *geometry = NULL;

	self->dictionary = cf->rootDict;
	listName = Stg_ComponentFactory_GetString( cf, self->name, "ListName", "" ); 
	
	topology =  Stg_ComponentFactory_ConstructByKey(  cf,  self->name,  Topology_Type, Topology,  True, data  ) ;
	geometry =  Stg_ComponentFactory_ConstructByKey(  cf,  self->name,  Geometry_Type, Geometry,  True, data ) ;

	_ElementLayout_Init( (ElementLayout*)self, geometry, topology );
	_IrregEL_Init( self, listName );
}
	
void _IrregEL_ComponentBuild( void* irregEL, void* data ){
	
}
	
void _IrregEL_Initialise( void* irregEL, void* data ){
	
}
	
void _IrregEL_Execute( void* irregEL, void* data ){
	
}
	
void _IrregEL_Destroy( void* irregEL, void* data ){
	
}

void _IrregEL_BuildCornerIndices( void* irregEL, Element_GlobalIndex globalIndex, Index* points ) {
	IrregEL*	self = (IrregEL*)irregEL;
	Index		p_I;
	
	for( p_I = 0; p_I < self->elementCornerCount; p_I++ )
		points[p_I] = self->elementTbl[globalIndex][p_I];
}


Element_GlobalIndex _IrregEL_CornerElementCount( void* irregEL, Index corner ) {
	IrregEL*		self = (IrregEL*)irregEL;
	Topology*		topology = self->topology;
	Index			e_I;
	Element_GlobalIndex     eNbrCnt;
	Element_Neighbours      eNbrs;
	Element_GlobalIndex     eCnt = 1;
	
	for( e_I = 0; e_I < self->elementCount; e_I++ ) {
		Index p_I;
		
		for( p_I = 0; p_I < self->elementCornerCount; p_I++ )
			if( self->elementTbl[e_I][p_I] == corner )
				break;
		
		if( p_I < self->elementCornerCount )
			break;
	}
	
	if( e_I == self->elementCount )
		assert( 0 );
	
	eNbrCnt = topology->neighbourCount( topology, e_I );
	eNbrs = Memory_Alloc_Array( Element_GlobalIndex, eNbrCnt, "IrregEL" );
	topology->buildNeighbours( topology, e_I, eNbrs );
	
	for( e_I = 0; e_I < eNbrCnt; e_I++ ) {
		Index p_I;
		
		for( p_I = 0; p_I < self->elementCornerCount; p_I++ )
			if( self->elementTbl[eNbrs[e_I]][p_I] == corner ) {
				eCnt++;
				break;
			}
	}
	
	return eCnt;
}


void _IrregEL_BuildCornerElements( void* irregEL, Index corner, Element_GlobalIndex* elements ) {
	IrregEL*		self = (IrregEL*)irregEL;
	Topology*		topology = self->topology;
	Index			e_I;
	Element_GlobalIndex     eCnt = 1;
	Element_GlobalIndex     eNbrCnt;
	Element_Neighbours      eNbrs;
	
	for( e_I = 0; e_I < self->elementCount; e_I++ ) {
		Index p_I;
		
		for( p_I = 0; p_I < self->elementCornerCount; p_I++ )
			if( self->elementTbl[e_I][p_I] == corner )
				break;
		
		if( p_I < self->elementCornerCount )
			break;
	}
	
	if( e_I == self->elementCount )
		assert( 0 );
	
	eNbrCnt = topology->neighbourCount( topology, e_I );
	eNbrs = Memory_Alloc_Array( Element_GlobalIndex, eNbrCnt, "IrregEL" );
	topology->buildNeighbours( topology, e_I, eNbrs );
	
	elements[0] = e_I;
	
	for( e_I = 0; e_I < eNbrCnt; e_I++ ) {
		Index p_I;
		
		for( p_I = 0; p_I < self->elementCornerCount; p_I++ )
			if( self->elementTbl[eNbrs[e_I]][p_I] == corner ) {
				elements[eCnt++] = eNbrs[e_I];
				break;
			}
	}
}


void _IrregEL_BuildEdgeIndices( void* irregEL, Element_GlobalIndex globalIndex, Index* edges ) {
	assert( 0 );
}


Element_GlobalIndex _IrregEL_EdgeElementCount( void* irregEL, Index edge ) {
	assert( 0 );
	return 0;
}


void _IrregEL_BuildEdgeElements( void* irregEL, Index edge, Element_GlobalIndex* elements ) {
	assert( 0 );
}


void _IrregEL_EdgeAt( void* irregEL, Index index, Edge edge ) {
	assert( 0 );
}


Element_GlobalIndex _IrregEL_ElementWithPoint( void* irregEL, void* decomp, Coord point, void* mesh, 
					       PartitionBoundaryStatus boundaryStatus, unsigned nHints, unsigned* hints )
{		
	/* Not yet supported */
	assert( 0 );
	return 0;
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/
