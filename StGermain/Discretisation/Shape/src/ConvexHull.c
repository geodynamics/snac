/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003, Victorian Partnership for Advanced Computing (VPAC) Ltd, 110 Victoria Street, Melbourne, 3053, Australia.
**
** Authors:
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**	Patrick D. Sunter, Software Engineer, VPAC. (pds@vpac.org)
**	Luke J. Hodkinson, Computational Engineer, VPAC. (lhodkins@vpac.org)
**	Siew-Ching Tan, Software Engineer, VPAC. (siew@vpac.org) ) 
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
** $Id: ConvexHull.c 3869 2006-10-16 13:42:59Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <Base/Base.h>
#include <Discretisation/Geometry/Geometry.h>

#include "types.h"
#include "ShapeClass.h"
#include "ConvexHull.h"

#include <assert.h>
#include <string.h>
#include <math.h>

/* Textual name of this class */
const Type ConvexHull_Type = "ConvexHull";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

ConvexHull* ConvexHull_New(
		Name                                  name,
		Dimension_Index                       dim,
		XYZ                                   centre, 
		double                                alpha,
		double                                beta,
		double                                gamma,
		Coord_List                            vertexList,
		Index                                 vertexCount
		)
{
	ConvexHull* self = (ConvexHull*)_ConvexHull_DefaultNew( name );

	ConvexHull_InitAll( 
		self, 
		dim,
		centre,
		alpha,
		beta,
		gamma,
		vertexList,
		vertexCount);
	return self;
}

ConvexHull* _ConvexHull_New(
		SizeT                                 _sizeOfSelf, 
		Type                                  type,
		Stg_Class_DeleteFunction*             _delete,
		Stg_Class_PrintFunction*              _print,
		Stg_Class_CopyFunction*               _copy, 
		Stg_Component_DefaultConstructorFunction* _defaultConstructor,
		Stg_Component_ConstructFunction*      _construct,
		Stg_Component_BuildFunction*          _build,
		Stg_Component_InitialiseFunction*     _initialise,
		Stg_Component_ExecuteFunction*        _execute,
		Stg_Component_DestroyFunction*        _destroy,		
		Stg_Shape_IsCoordInsideFunction*      _isCoordInside,
		Stg_Shape_CalculateVolumeFunction*    _calculateVolume,
		Name                                  name )
{
	ConvexHull* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(ConvexHull) );
	self = (ConvexHull*)_Stg_Shape_New( 
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
			_isCoordInside,
			_calculateVolume,
			name );
	
	/* General info */

	/* Virtual Info */
	
	return self;
}

void _ConvexHull_Init( void* convexHull, Coord_List vertexList, Index vertexCount) {
	ConvexHull* self = (ConvexHull*)convexHull;
	Index       numberOfFaces;
	Index       dimensions; // this probably doesn't need to be here
	Index       vertex_I;
	
	double        tmpVector1[3];
	double        tmpVector2[3];
	double        tmpVector3[3];
	
	dimensions = 3;
	/* Copy vertexCount */
	self->vertexCount = vertexCount;
	/* Copy vertexList */
	self->vertexList = Memory_Alloc_Array( Coord, vertexCount, "vertexList" );
	memcpy( self->vertexList , vertexList, sizeof(Coord) * vertexCount );
	
	/* Now Construct normal and store them in self->facesList */
	
	// 1st allocate memory	
	if(vertexCount == 3) numberOfFaces = 3; 
	else                 numberOfFaces = 4; 
	
	self->facesList = Memory_Alloc_Array( XYZ, numberOfFaces, "facesList" );

	// 2nd Calculate Normals on faces
	// in 2-D
	if( numberOfFaces == 3 ) {
	printf("THE VERTEXCOUNT IS %d\n", vertexCount);
		for( vertex_I = 0; vertex_I < vertexCount ; vertex_I++ ) {
			
			StGermain_VectorSubtraction(tmpVector1, vertexList[ vertex_I % 3 ], vertexList[ (vertex_I+1) % 3 ], dimensions);
			StGermain_VectorSubtraction(tmpVector2, vertexList[ vertex_I % 3 ], vertexList[ (vertex_I+2) % 3 ], dimensions);
				
			tmpVector3[0] = tmpVector1[1];
			tmpVector3[1] = -1 * tmpVector1[0];
			tmpVector3[2] = 0;
//			Now Vec3 is possibly normal vector
				if( StGermain_VectorDotProduct(tmpVector3, tmpVector2, dimensions) < 0 )
				{ tmpVector3[0] = -1*tmpVector3[0]; tmpVector3[1] = -1 * tmpVector3[1]; }
			
			printf("vector normal = (%g, %g, %g)\n", tmpVector3[0], tmpVector3[1],  tmpVector3[2] ); 
			memcpy( self->facesList[ vertex_I ], tmpVector3, sizeof(XYZ) );
		}
	}
	else {
		for( vertex_I = 0 ; vertex_I < vertexCount ; vertex_I++ ) {
			StGermain_NormalToPlane( tmpVector3, vertexList[ vertex_I % 4], vertexList[ (vertex_I+1) % 4], vertexList[ (vertex_I+2) % 4]);
			StGermain_VectorSubtraction( tmpVector2, vertexList[ vertex_I % 4 ], vertexList[ (vertex_I+3) % 4 ], dimensions);

			if( StGermain_VectorDotProduct(tmpVector3, tmpVector2, dimensions) < 0 ) 
				{ Vector_Div( tmpVector3, tmpVector3, -1.0 ); }
			printf("vector normal = (%g, %g, %g)\n", tmpVector3[0], tmpVector3[1],  tmpVector3[2] ); 
			memcpy( self->facesList[ vertex_I ], tmpVector3, sizeof(XYZ) );

		}
	}
}
	
void ConvexHull_InitAll( 
		void*                                 convexHull, 
		Dimension_Index                       dim, 
		Coord                                 centre,
		double                                alpha,
		double                                beta,
		double                                gamma,
		Coord_List                            vertexList,
		Index                                 vertexCount
		)
{
	ConvexHull* self = (ConvexHull*)convexHull;

	Stg_Shape_InitAll( self, dim, centre, alpha, beta, gamma);
	_ConvexHull_Init( self, vertexList, vertexCount);
}
	

/*------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _ConvexHull_Delete( void* convexHull ) {
	ConvexHull*       self       = (ConvexHull*)convexHull;
	Coord_List        vertexList = self->vertexList;
	XYZ*              facesList  = self->facesList;
	

	Memory_Free( vertexList );
	Memory_Free( facesList );
	
	/* Delete parent */
	_Stg_Shape_Delete( self );
}


void _ConvexHull_Print( void* convexHull, Stream* stream ) {
	ConvexHull* self = (ConvexHull*)convexHull;
	
	/* Print parent */
	_Stg_Shape_Print( self, stream );
}



void* _ConvexHull_Copy( void* convexHull, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	ConvexHull*	self = (ConvexHull*)convexHull;
	ConvexHull*	newConvexHull;
	
	newConvexHull = (ConvexHull*)_Stg_Shape_Copy( self, dest, deep, nameExt, ptrMap );

	newConvexHull->vertexList = Memory_Alloc_Array( Coord, self->vertexCount, "vertexList" );
	memcpy( newConvexHull->vertexList , self->vertexList, sizeof(Coord) * self->vertexCount );

	newConvexHull->facesList = Memory_Alloc_Array( XYZ, self->vertexCount, "facesList" );
	memcpy( newConvexHull->facesList, self->facesList, sizeof(XYZ) * self->vertexCount );

	newConvexHull->vertexList  = self->vertexList;
	newConvexHull->vertexCount = self->vertexCount;
	newConvexHull->facesList   = self->facesList;
	
	return (void*)newConvexHull;
}

void* _ConvexHull_DefaultNew( Name name ) {
	return (void*) _ConvexHull_New(
			sizeof(ConvexHull),
			ConvexHull_Type,
			_ConvexHull_Delete,
			_ConvexHull_Print,
			_ConvexHull_Copy,
			_ConvexHull_DefaultNew,
			_ConvexHull_Construct,
			_ConvexHull_Build,
			_ConvexHull_Initialise,
			_ConvexHull_Execute,
			_ConvexHull_Destroy,
			_ConvexHull_IsCoordInside,
			_ConvexHull_CalculateVolume,
			name );
}


void _ConvexHull_Construct( void* convexHull, Stg_ComponentFactory* cf, void* data ) {
	ConvexHull*             self       = (ConvexHull*)convexHull;
	Index                   vertexCount;
	Index                   vertex_I;
	Coord_List              vertexList;
	double*                 coord;
	Dictionary_Entry_Value* optionSet;
	Dictionary_Entry_Value* optionsList;
	Dictionary*             dictionary = Dictionary_GetDictionary( cf->componentDict, self->name );
	Stream*                 stream     = cf->infoStream;

	
	_Stg_Shape_Construct( self, cf, data );

	optionsList = Dictionary_Get( dictionary, "verticies" );
	Journal_Firewall( optionsList != NULL, 
		Journal_Register( Error_Type, self->type ),
		"In func %s: The list 'verticies' specifying the convexHull is NULL.\n", __func__);

	vertexCount = Dictionary_Entry_Value_GetCount(optionsList);
	Journal_Firewall( ( self->dim == 2 && vertexCount < 4 ) || ( self->dim == 3 && vertexCount < 5 ),
		Journal_Register( Error_Type, self->type ),
		"In func %s: Sorry, but we got lazy, you can only specify 3 (2D) or 4 (3D) points. " 
		"Please feel free to hassle developers for this feature.\n", __func__);

	/* Allocate space */
	vertexList = Memory_Alloc_Array( Coord , vertexCount, "Vertex Array" );
	memset( vertexList, 0, vertexCount * sizeof(Coord) );
	
	Stream_Indent( stream );
	for ( vertex_I = 0 ; vertex_I < vertexCount ; vertex_I++) { 
		optionSet = Dictionary_Entry_Value_GetElement(optionsList, vertex_I );
		coord = vertexList[vertex_I];

		/* Read Vertex */
		coord[ I_AXIS ] = Dictionary_Entry_Value_AsDouble( Dictionary_Entry_Value_GetMember( optionSet, "x"));
		coord[ J_AXIS ] = Dictionary_Entry_Value_AsDouble( Dictionary_Entry_Value_GetMember( optionSet, "y"));
		
		coord[ K_AXIS ] = Dictionary_Entry_Value_AsDouble( Dictionary_Entry_Value_GetMember( optionSet, "z"));
		Journal_PrintfL( stream, 2, "(%0.3g, %0.3g, %0.3g)\n", coord[ I_AXIS ], coord[ J_AXIS ], coord[ K_AXIS ] );
		optionSet = optionSet->next;
	}
	Stream_UnIndent( stream );


	_ConvexHull_Init( self, vertexList, vertexCount);
}

void _ConvexHull_Build( void* convexHull, void* data ) {
	ConvexHull*	self = (ConvexHull*)convexHull;

	_Stg_Shape_Build( self, data );
}
void _ConvexHull_Initialise( void* convexHull, void* data ) {
	ConvexHull*	self = (ConvexHull*)convexHull;
	
	_Stg_Shape_Initialise( self, data );
}
void _ConvexHull_Execute( void* convexHull, void* data ) {
	ConvexHull*	self = (ConvexHull*)convexHull;
	
	_Stg_Shape_Execute( self, data );
}
void _ConvexHull_Destroy( void* convexHull, void* data ) {
	ConvexHull*	self = (ConvexHull*)convexHull;
	
	_Stg_Shape_Destroy( self, data );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/
	
/*---------------------------------------------------------------------------------------------------------------------
** Private Member functions
*/
Bool _ConvexHull_IsCoordInside( void* convexHull, Coord point ) {
	ConvexHull*     self                 = (ConvexHull*)convexHull;
	Index           vertex_I;
	XYZ             tmpVector;
	//Stream*                 stream     = cf->infoStream;


	// for the particle to be inside the shape, the dot product of its position vector with the shape face normal's must be non-positive
	for( vertex_I = 0 ; vertex_I < self->vertexCount ; vertex_I++ ) {
		StGermain_VectorSubtraction(tmpVector, point, self->vertexList[ vertex_I ], self->dim );
		if( StGermain_VectorDotProduct(self->facesList[ vertex_I ], tmpVector, self->dim ) > 0 ) {
			return False;
		}
	}
	return True;
}

double _ConvexHull_CalculateVolume( void* convexHull ) {
	assert( 0 );
	return 0.0;
}


