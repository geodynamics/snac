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
** $Id: RefinedRegionsGeometry.c 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"


#include "units.h"
#include "types.h"
#include "GeometryClass.h"
#include "RefinedRegionsGeometry.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>


/* Textual name of this class */
const Type RefinedRegionsGeometry_Type = "RefinedRegionsGeometry";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

void* RefinedRegionsGeometry_DefaultNew( Name name )
{
	return (void*) _RefinedRegionsGeometry_New(
		sizeof(RefinedRegionsGeometry), 
		RefinedRegionsGeometry_Type, 
		_RefinedRegionsGeometry_Delete, 
		_RefinedRegionsGeometry_Print,
		_RefinedRegionsGeometry_Copy,
		RefinedRegionsGeometry_DefaultNew,
		_RefinedRegionsGeometry_Construct,
		_RefinedRegionsGeometry_Build,
		_RefinedRegionsGeometry_Initialise,
		_RefinedRegionsGeometry_Execute,
		_RefinedRegionsGeometry_Destroy,
		name,
		False,
		_RefinedRegionsGeometry_BuildPoints,
		_RefinedRegionsGeometry_PointAt,
		NULL );
}

RefinedRegionsGeometry* RefinedRegionsGeometry_New(
		Name name,
		Dictionary*					dictionary )
{
	return _RefinedRegionsGeometry_New( 
		sizeof(RefinedRegionsGeometry), 
		RefinedRegionsGeometry_Type, 
		_RefinedRegionsGeometry_Delete, 
		_RefinedRegionsGeometry_Print,
		_RefinedRegionsGeometry_Copy,
		RefinedRegionsGeometry_DefaultNew,
		_RefinedRegionsGeometry_Construct,
		_RefinedRegionsGeometry_Build,
		_RefinedRegionsGeometry_Initialise,
		_RefinedRegionsGeometry_Execute,
		_RefinedRegionsGeometry_Destroy,
		name,
		True,
		_RefinedRegionsGeometry_BuildPoints,
		_RefinedRegionsGeometry_PointAt,
		dictionary );
}


void RefinedRegionsGeometry_Init(
		RefinedRegionsGeometry*					self,
		Name						name,
		Dictionary*					dictionary )
{
	/* General info */
	self->type = RefinedRegionsGeometry_Type;
	self->_sizeOfSelf = sizeof(RefinedRegionsGeometry);
	self->_deleteSelf = False;
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->_delete = _RefinedRegionsGeometry_Delete;
	self->_print = _RefinedRegionsGeometry_Print;
	self->_copy = _RefinedRegionsGeometry_Copy;
	self->_defaultConstructor = RefinedRegionsGeometry_DefaultNew;
	self->_construct = _RefinedRegionsGeometry_Construct;
	self->_build = _RefinedRegionsGeometry_Build;
	self->_initialise = _RefinedRegionsGeometry_Initialise;
	self->_execute = _RefinedRegionsGeometry_Execute;
	self->_destroy = _RefinedRegionsGeometry_Destroy;
	self->buildPoints = _RefinedRegionsGeometry_BuildPoints;
	self->pointAt = _RefinedRegionsGeometry_PointAt;
	
	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	_Stg_Component_Init( (Stg_Component*)self );
	
	_Geometry_Init( (Geometry*)self );
	
	/* RefinedRegionsGeometry info */
	_RefinedRegionsGeometry_Init( self, NULL );
}


RefinedRegionsGeometry* _RefinedRegionsGeometry_New(
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
		Geometry_BuildPointsFunction*			buildPoints,
		Geometry_PointAtFunction*			pointAt,
		Dictionary*					dictionary )
{
	RefinedRegionsGeometry* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(RefinedRegionsGeometry) );
	self = (RefinedRegionsGeometry*)_Geometry_New(
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
		buildPoints,
		pointAt,
		dictionary );
	
	/* General info */
	
	/* Virtual info */
	
	/* RefinedRegionsGeometry info */
	if( initFlag ){
		_RefinedRegionsGeometry_Init( self, NULL );
	}
	
	return self;
}

void _RefinedRegionsGeometry_Init( RefinedRegionsGeometry* self, IJK size ) {
	Dimension_Index		dim_I;
	Dictionary_Entry_Value*	regionsList = NULL;

	/* General and Virtual info should already be set */
	
	/* RefinedRegionsGeometry info */
	self->isConstructed = False;

	self->min[ I_AXIS ] = Dictionary_GetDouble_WithDefault( self->dictionary, "minX", 0.0f );
	self->min[ J_AXIS ] = Dictionary_GetDouble_WithDefault( self->dictionary, "minY", 0.0f );
	self->min[ K_AXIS ] = Dictionary_GetDouble_WithDefault( self->dictionary, "minZ", 0.0f );

	self->max[ I_AXIS ] = Dictionary_GetDouble_WithDefault( self->dictionary, "maxX", 1.0f );
	self->max[ J_AXIS ] = Dictionary_GetDouble_WithDefault( self->dictionary, "maxY", 1.0f );
	self->max[ K_AXIS ] = Dictionary_GetDouble_WithDefault( self->dictionary, "maxZ", 1.0f );

	if ( size ) {
		memcpy( self->countPerDim, size, sizeof(IJK) );
	}
	else {
		self->countPerDim[ I_AXIS ] = Dictionary_GetUnsignedInt_WithDefault( self->dictionary, "meshSizeI", 2 );
		self->countPerDim[ J_AXIS ] = Dictionary_GetUnsignedInt_WithDefault( self->dictionary, "meshSizeJ", 2 );
		self->countPerDim[ K_AXIS ] = Dictionary_GetUnsignedInt_WithDefault( self->dictionary, "meshSizeK", 2 );
	}

	for ( dim_I = 0; dim_I < 3; dim_I++ ) {
		self->refinedRegionDeltas[dim_I] = 4;
	}
	
	/* Now Read in the refined regions */
	regionsList = Dictionary_Get( self->dictionary, "RefinedRegions" );	
	if ( regionsList ) {
		Index				entryCount = Dictionary_Entry_Value_GetCount( regionsList );
		Index				entry_I = 0;
		Dictionary_Entry_Value*		regionEntry;
		Dictionary*			regionDict;
		Dimension_Index			dim = 0;
		double				regionStart = 0;
		double				regionEnd = 0;
		unsigned int			refinementFactor = 1;
		
		for( entry_I = 0; entry_I < entryCount; entry_I++ ) {
			regionEntry = Dictionary_Entry_Value_GetElement( regionsList, entry_I );
			regionDict = Dictionary_Entry_Value_AsDictionary( regionEntry );
			dim = Dictionary_GetUnsignedInt_WithDefault( regionDict, "dim", 0 );
			regionStart = Dictionary_GetDouble_WithDefault( regionDict, "regionStart", 0.0 );
			regionEnd = Dictionary_GetDouble_WithDefault( regionDict, "regionEnd", 1.0 );
			refinementFactor = Dictionary_GetUnsignedInt_WithDefault( regionDict, "refinementFactor", 2 );
			_RefinedRegionsGeometry_AddRefinedRegion( self, dim, regionStart, regionEnd, refinementFactor );
		}	
	}
	
	self->pointCount = self->countPerDim[I_AXIS] * self->countPerDim[J_AXIS] *
		self->countPerDim[K_AXIS];
	
	assert( self->pointCount );

}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _RefinedRegionsGeometry_Delete( void* refinedRegionsGeometry ) {
	RefinedRegionsGeometry* self = (RefinedRegionsGeometry*)refinedRegionsGeometry;
	
	/* Stg_Class_Delete parent */
	_Geometry_Delete( self );
}


void _RefinedRegionsGeometry_Print( void* refinedRegionsGeometry, Stream* stream ) {
	RefinedRegionsGeometry* self = (RefinedRegionsGeometry*)refinedRegionsGeometry;
	
	/* Print parent */
	_Geometry_Print( self, stream );
	
	/* General info */
	Journal_Printf( stream, "RefinedRegionsGeometry (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* RefinedRegionsGeometry info */
}

void _RefinedRegionsGeometry_Construct( void* refinedRegionsGeometry, Stg_ComponentFactory *cf, void* data ){
	RefinedRegionsGeometry*  self   = (RefinedRegionsGeometry*)refinedRegionsGeometry;
	IJK             size;
	int             shift;
	Dimension_Index dim;
	Dimension_Index dim_I;

	self->dictionary =  Dictionary_GetDictionary( cf->componentDict, self->name );

	/* Get Size from Dictionary */
	dim = Stg_ComponentFactory_GetRootDictUnsignedInt( cf, "dim", 0 );
		
	size[ I_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, "sizeI", 1 );  
	size[ J_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, "sizeJ", 1 );
	size[ K_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, "sizeK", 1 );
	if ( dim == 2 )
		size[ K_AXIS ] = 1;

	/* Shift the size if nessesary */
	shift = Stg_ComponentFactory_GetInt( cf, self->name, "sizeShift", 0 );
	for ( dim_I = I_AXIS ; dim_I < dim ; dim_I++ )
		size[ dim_I ] += shift;
	
	_Geometry_Init( (Geometry*)self );
	_RefinedRegionsGeometry_Init( self, size );
}
	
void _RefinedRegionsGeometry_Build( void* refinedRegionsGeometry, void *data ){
	
}
	
void _RefinedRegionsGeometry_Initialise( void* refinedRegionsGeometry, void *data ){
	
}
	
void _RefinedRegionsGeometry_Execute( void* refinedRegionsGeometry, void *data ){
	
}
	
void _RefinedRegionsGeometry_Destroy( void* refinedRegionsGeometry, void *data ){
	
}


void* _RefinedRegionsGeometry_Copy( void* refinedRegionsGeometry, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	RefinedRegionsGeometry*	self = (RefinedRegionsGeometry*)refinedRegionsGeometry;
	RefinedRegionsGeometry*	newRefinedRegionsGeometry;
	Index		dim_I;
	
	newRefinedRegionsGeometry = (RefinedRegionsGeometry*)_Geometry_Copy( self, dest, deep, nameExt, ptrMap );
	
	for( dim_I = 0; dim_I < 3; dim_I++ ) {
		newRefinedRegionsGeometry->countPerDim[dim_I] = 
			self->countPerDim[dim_I];
		newRefinedRegionsGeometry->min[dim_I] = self->min[dim_I];
		newRefinedRegionsGeometry->max[dim_I] = self->max[dim_I];
		newRefinedRegionsGeometry->refinedRegionCounts[dim_I] = 
			self->refinedRegionCounts[dim_I];
		newRefinedRegionsGeometry->refinedRegionSizes[dim_I] = 
			self->refinedRegionSizes[dim_I];
		newRefinedRegionsGeometry->refinedRegionDeltas[dim_I] = 
			self->refinedRegionDeltas[dim_I];
		memcpy( newRefinedRegionsGeometry->refinedRegions[dim_I],
			self->refinedRegions[dim_I], 
			sizeof(RefinedRegion) * self->refinedRegionCounts[dim_I] );
	}
	
	return (void*)newRefinedRegionsGeometry;
}


void _RefinedRegionsGeometry_AddRefinedRegion( void* refinedRegionsGeometry, Index dim,
		double regionStart, double regionEnd, unsigned int refinementFactor )
{
	RefinedRegionsGeometry*	self = (RefinedRegionsGeometry*)refinedRegionsGeometry;

	if ( self->refinedRegionCounts[dim] == self->refinedRegionSizes[dim] )
	{
		self->refinedRegionSizes[dim] += self->refinedRegionDeltas[dim];
		self->refinedRegions[dim] = Memory_Realloc( 
			self->refinedRegions[dim],
			self->refinedRegionSizes[dim] * sizeof(RefinedRegion) );
	}
	self->refinedRegions[dim][self->refinedRegionCounts[dim]].regionStart = regionStart;
	self->refinedRegions[dim][self->refinedRegionCounts[dim]].regionEnd = regionEnd;
	self->refinedRegions[dim][self->refinedRegionCounts[dim]].refinementFactor = refinementFactor;
	/* Following 2 values are set to zero until real vals calculated later */
	self->refinedRegions[dim][self->refinedRegionCounts[dim]].numElements = 0;
	self->refinedRegions[dim][self->refinedRegionCounts[dim]].elementLength = 0.0;
	self->refinedRegionCounts[dim]++;
}





void _RefinedRegionsGeometry_BuildPoints( void* refinedRegionsGeometry, Coord_List points )
{
	RefinedRegionsGeometry*  self = (RefinedRegionsGeometry*)refinedRegionsGeometry;
	Dimension_Index	dim_I = 0;
	Index		currRegion_I[3] = { 0, 0, 0 };
	double		elementLengthNormal[3];
	Index		numElementsNormal[3];
	RefinedRegion*	currRefinedRegion = NULL;
	Index		i;
	double		increment = 0;
	IJK		ijk;
	IJK		prevIJK = {0,0,0};
	Coord		prevPos = {0,0,0};
	Bool		withinRefined[3] = { False, False, False };
	
	_RefinedRegionsGeometry_CalculateElementLengths( refinedRegionsGeometry, numElementsNormal, elementLengthNormal );

	for( i = 0; i < self->pointCount; i++ ) {
	
		ijk[0] = i % self->countPerDim[0];
		ijk[1] = (i / self->countPerDim[0]) % self->countPerDim[1];
		ijk[2] = i / (self->countPerDim[0] * self->countPerDim[1]);
	
		for ( dim_I = 0; dim_I < 3; dim_I++ ) {
			if ( ijk[dim_I] == 0 ) {
				/* Axis reset to 0 - also reset the region iterator*/
				points[i][dim_I] = self->min[dim_I];
				currRegion_I[dim_I] = 0;
			}
			else if ( ijk[dim_I] == prevIJK[dim_I] ) {
				/* We haven't moved in this direction this step */
				points[i][dim_I] = prevPos[dim_I];
			}
			else if ( withinRefined[dim_I] ) {
				currRefinedRegion = &self->refinedRegions[dim_I][currRegion_I[dim_I]];
				/* We are within a refined block */
				increment = currRefinedRegion->elementLength;
				points[i][dim_I] =  prevPos[dim_I] + increment;
				if ( points[i][dim_I] > ( currRefinedRegion->regionEnd - 1e-6 ) ) {
					withinRefined[dim_I] = False;
					(currRegion_I[dim_I])++;
				}
			}
			else if ( ( self->refinedRegions[dim_I] ) &&
				( currRegion_I[dim_I] < self->refinedRegionCounts[dim_I] ) &&
				(prevPos[dim_I] > self->refinedRegions[dim_I][currRegion_I[dim_I]].regionStart - 1e-6 ) )
			{
				currRefinedRegion = &self->refinedRegions[dim_I][currRegion_I[dim_I]];
				/* We are entering a refined block */
				withinRefined[dim_I] = True;
				increment = currRefinedRegion->elementLength;
				points[i][dim_I] = prevPos[dim_I] + increment;
			}	
			else {
				/* The "normal" case- not in refined area */
				points[i][dim_I] = prevPos[dim_I] + elementLengthNormal[dim_I];
			}

			prevPos[dim_I] = points[i][dim_I];
			prevIJK[dim_I] = ijk[dim_I];
		}
	}
}

void _RefinedRegionsGeometry_PointAt( void* refinedRegionsGeometry, Index index, Coord point )
{
	RefinedRegionsGeometry* self = (RefinedRegionsGeometry*)refinedRegionsGeometry;
	
	if( index < self->pointCount ) {
		Index		i;
		Bool		withinRefined[3] = { False, False, False };
		Index		currRegion_I[3] = { 0, 0, 0 };
		RefinedRegion*	currRefinedRegion = NULL;
		Dimension_Index	dim_I = 0;
		double		increment = 0;
		IJK		ijk;
		IJK		prevIJK = {0,0,0};
		Coord		prevPos = {0,0,0};
		double		elementLengthNormal[3];
		Index		numElementsNormal[3];
		
		_RefinedRegionsGeometry_CalculateElementLengths( refinedRegionsGeometry, numElementsNormal, elementLengthNormal );
		
		for( i = 0; i <= index; i++ ) {
		
			ijk[0] = i % self->countPerDim[0];
			ijk[1] = (i / self->countPerDim[0]) % self->countPerDim[1];
			ijk[2] = i / (self->countPerDim[0] * self->countPerDim[1]);
		
			for ( dim_I = 0; dim_I < 3; dim_I++ ) {
				if ( ijk[dim_I] == 0 ) {
					/* Axis reset to 0 - also reset the region iterator*/
					point[dim_I] = self->min[dim_I];
					currRegion_I[dim_I] = 0;
				}
				else if ( ijk[dim_I] == prevIJK[dim_I] ) {
					/* We haven't moved in this direction this step */
					point[dim_I] = prevPos[dim_I];
				}
				else if ( withinRefined[dim_I] ) {
					currRefinedRegion = &self->refinedRegions[dim_I][currRegion_I[dim_I]];
					/* We are within a refined block */
					increment = currRefinedRegion->elementLength;
					point[dim_I] =  prevPos[dim_I] + increment;
					if ( point[dim_I] > ( currRefinedRegion->regionEnd - 1e-6 ) ) {
						withinRefined[dim_I] = False;
						(currRegion_I[dim_I])++;
					}
				}
				else if ( ( self->refinedRegions[dim_I] ) &&
					( currRegion_I[dim_I] < self->refinedRegionCounts[dim_I] ) &&
					( prevPos[dim_I] > self->refinedRegions[dim_I][currRegion_I[dim_I]].regionStart - 1e-6) )
				{
					currRefinedRegion = &self->refinedRegions[dim_I][currRegion_I[dim_I]];
					/* We are entering a refined block */
					withinRefined[dim_I] = True;
					increment = currRefinedRegion->elementLength;
					point[dim_I] =  prevPos[dim_I] + increment;
				}	
				else {
					/* The "normal" case- not in refined area */
					point[dim_I] = prevPos[dim_I] + elementLengthNormal[dim_I];
				}

				prevPos[dim_I] = point[dim_I];
				prevIJK[dim_I] = ijk[dim_I];
			}
		}	
	}
	else {
		point[0] = HUGE_VAL;
		point[1] = HUGE_VAL;
		point[2] = HUGE_VAL;
	}
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

void _RefinedRegionsGeometry_CalculateElementLengths(
		RefinedRegionsGeometry*	self,
		Index			numElementsNormal[3],
		double			elementLengthNormal[3] )
{	
	Dimension_Index	dim_I = 0;
	Index		currRegion_I = 0;
	double		remainderLength;
	RefinedRegion*	currRefinedRegion = NULL;
	double		summation = 0;
	double		elLengthBase = 0;
	double		regionLength;

	for ( dim_I = 0; dim_I < 3; dim_I++ ) {
		remainderLength = self->max[dim_I] - self->min[dim_I];

		for ( currRegion_I = 0; currRegion_I < self->refinedRegionCounts[dim_I]; currRegion_I++ ) {
			currRefinedRegion = &self->refinedRegions[dim_I][currRegion_I];
			remainderLength -= ( currRefinedRegion->regionEnd - currRefinedRegion->regionStart );
		}
		
		summation = 0.0;
		for ( currRegion_I = 0; currRegion_I < self->refinedRegionCounts[dim_I]; currRegion_I++ ) {
			currRefinedRegion = &self->refinedRegions[dim_I][currRegion_I];
			summation += ( currRefinedRegion->regionEnd - currRefinedRegion->regionStart ) *
				currRefinedRegion->refinementFactor;
		}	
		summation += remainderLength * 1;
			
		elLengthBase = 1.0 / (self->countPerDim[dim_I] - 1) * summation;	
		
		/* Now we have the base element length, calculate the rounded values for each section */
		for ( currRegion_I = 0; currRegion_I < self->refinedRegionCounts[dim_I]; currRegion_I++ ) {
			currRefinedRegion = &self->refinedRegions[dim_I][currRegion_I];
			regionLength = currRefinedRegion->regionEnd - currRefinedRegion->regionStart;
			currRefinedRegion->numElements = floor( regionLength / elLengthBase *
				currRefinedRegion->refinementFactor + 0.5 );
			currRefinedRegion->elementLength = regionLength / currRefinedRegion->numElements;	
		}	

		/* Do a check to make sure the total number of elements is correct */
		numElementsNormal[dim_I] = self->countPerDim[dim_I] - 1;
		for ( currRegion_I = 0; currRegion_I < self->refinedRegionCounts[dim_I]; currRegion_I++ ) {
			currRefinedRegion = &self->refinedRegions[dim_I][currRegion_I];
			numElementsNormal[dim_I] -= currRefinedRegion->numElements;
		}	
		elementLengthNormal[dim_I] = remainderLength / numElementsNormal[dim_I];
	}
}
