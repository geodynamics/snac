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
*/
/** \file
**  Role:
**	This class allows the user to specify a regular geometry, but with refinements
**	in special regions.
**
** Assumptions:
**
** Comments:
**
** $Id: RefinedRegionsGeometry.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Geometry_RefinedRegionsGeometry_h__
#define __Discretisation_Geometry_RefinedRegionsGeometry_h__

	/* Virtual function types */
	
	/* Textual name of this class */
	extern const Type RefinedRegionsGeometry_Type;

	typedef struct RefinedRegion {
		double		regionStart;
		double		regionEnd;
		unsigned int	refinementFactor;
		Index		numElements;
		double		elementLength;
	} RefinedRegion;

	/* RefinedRegionsGeometry information */
	#define __RefinedRegionsGeometry \
		/* General info */ \
		__Geometry \
		\
		/* Virtual info */ \
		\
		/* RefinedRegionsGeometry info ... */ \
		IJK				countPerDim; \
		XYZ				min; \
		XYZ				max; \
		Index				refinedRegionCounts[3]; \
		Index				refinedRegionSizes[3]; \
		Index				refinedRegionDeltas[3]; \
		RefinedRegion*			refinedRegions[3]; \
		
	struct RefinedRegionsGeometry { __RefinedRegionsGeometry };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/* Create a RefinedRegionsGeometry */
	void* RefinedRegionsGeometry_DefaultNew( Name name );
	
	RefinedRegionsGeometry* RefinedRegionsGeometry_New(
		Name						name,
		Dictionary*					dictionary );
	
	/* Initialise a RefinedRegionsGeometry */
	void RefinedRegionsGeometry_Init(
		RefinedRegionsGeometry*					self,
		Name						name,
		Dictionary*					dictionary );
	
	/* Creation implementation */
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
		Dictionary*					dictionary );
	
	/* Initialisation implementation functions */
	void _RefinedRegionsGeometry_Init( RefinedRegionsGeometry* self, IJK size );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/* Stg_Class_Delete RefinedRegionsGeometry implementation */
	void _RefinedRegionsGeometry_Delete( void* blockGeometry );
	
	/* Print RefinedRegionsGeometry implementation */
	void _RefinedRegionsGeometry_Print( void* blockGeometry, Stream* stream );
	
	void _RefinedRegionsGeometry_Construct( void* blockGeometry, Stg_ComponentFactory *cf, void* data );
	
	void _RefinedRegionsGeometry_Build( void* blockGeometry, void *data );
	
	void _RefinedRegionsGeometry_Initialise( void* blockGeometry, void *data );
	
	void _RefinedRegionsGeometry_Execute( void* blockGeometry, void *data );
	
	void _RefinedRegionsGeometry_Destroy( void* blockGeometry, void *data );
	
	/* Copy */
	#define RefinedRegionsGeometry_Copy( self ) \
		(RefinedRegionsGeometry*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define RefinedRegionsGeometry_DeepCopy( self ) \
		(RefinedRegionsGeometry*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _RefinedRegionsGeometry_Copy( void* geometry, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	void _RefinedRegionsGeometry_AddRefinedRegion( void* blockGeometry, Index dimension, double regionStart, double regionEnd, unsigned int refinementFactor );

	void _RefinedRegionsGeometry_BuildPoints( void* blockGeometry, Coord_List points );
	
	void _RefinedRegionsGeometry_PointAt( void* blockGeometry, Index index, Coord point );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public member functions
	*/
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	/** Calculates element lengths and numbers of elements in each region, including the "normal"
	unrefined region. A first iteration at the normal length is calculated using:
	X0 = 1 / globalRes * ( sigma ( Fn * Rn) ) 
	Where:
	X0 = unrefined element length, 
	globalRes = global number of elements in this dimension
	Fn = requested refinement factor of nth region
	Rn = Region length of nth region

	We then calculate the number of elements in the refined regions, _rounding off_ to ensure
	even division:
	resN = round( Rn / X0 * Fn)
	where resN = the number of elements in the nth region, others as before.

	And the element length can be calculated as:
	elLengthN = Rn / resN.
	
	The number of elements in the unrefined region is then recovered by subtracting all the
	other element totals, and the element length calcualted as above.
	*/
	void _RefinedRegionsGeometry_CalculateElementLengths(
		RefinedRegionsGeometry*	self,
		Index			numElementsNormal[3],
		double			elementLengthNormal[3] );

	
#endif /* __Discretisation_Geometry_RefinedRegionsGeometry_h__ */
