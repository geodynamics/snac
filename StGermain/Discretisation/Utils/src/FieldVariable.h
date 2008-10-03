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
**	Represents a Variable that is a discretisation of a field-like physical property over a spatial domain.
**
** Assumptions:
**	The function interfaces assume the spatially disc. variable is stored as a double
**	(it can't be an int because its an approximation to a continuous variable right?)
**
** Comments:
**	Abstract class that defines an interface to use when accessing spatially discretised
**	field variables.
**
**	This means that e.g. visualisation code can be written to use this class,
**	and doesn't have to worry exactly how the variable is discretised - that will be
**	done by the back-end implementation of this class.
**
**	The name comes from the definition of "field" in the physics domain: A region of space
**	characterized by a physical property, such as gravitational or electromagnetic force or
**	fluid pressure, having a determinable value at every point in the region.
**
**	TODO: should it have a ptr to the Variable its based on at this level?
**	doesn't make sense at the moment as the FeVariable used a \
**	doflayout rather than a variable -> but may in future... 
**
**	$Id: FieldVariable.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Utils_FieldVariable_h__
#define __Discretisation_Utils_FieldVariable_h__

	/** Textual name of this class */
	extern const Type FieldVariable_Type;
	
	/** Enumerated type to allow the user to know the result of an effort to interpolate the value of a
	 * FieldVariable at a particular point.
	 * Note that the order in the enum has been chosen for backward-compatibility for functions that now
	 * return one of these instead of a Bool: 0 (false) meant other proc, 1 (true) meant local
	 */
	typedef enum InterpolationResult {
		OTHER_PROC = 0,	/**< The value wasn't interpolated, as the requested co-ord is on another processor */
		LOCAL = 1,	/**< The value was successfully interpolated, in local space. */
		SHADOW,		/**< The value was successfully interpolated, in shadow space. */
		OUTSIDE_GLOBAL	/**< The value wasn't interpolated, as the requested co-ord is outside the entire field */
	} InterpolationResult;
	
	/** Mapping to convert an interpolation result to a string for debug purposes */
	extern const char* InterpolationResultToStringMap[4];
	
	typedef InterpolationResult	(FieldVariable_InterpolateValueAtFunction) ( void* fieldVariable, Coord coord, double* value );
	typedef double	(FieldVariable_GetValueFunction) ( void* fieldVariable );
	typedef void	(FieldVariable_GetCoordFunction) ( void* fieldVariable, Coord min, Coord max );
	
	/** FieldVariable contents */
	#define __FieldVariable \
		/* General info */ \
		__Stg_Component \
		\
		/* Virtual info */ \
		FieldVariable_InterpolateValueAtFunction*          _interpolateValueAt; \
		FieldVariable_GetValueFunction*                    _getMinGlobalFieldMagnitude; \
		FieldVariable_GetValueFunction*                    _getMaxGlobalFieldMagnitude; \
		FieldVariable_GetCoordFunction*                    _getMinAndMaxLocalCoords;	\
		FieldVariable_GetCoordFunction*                    _getMinAndMaxGlobalCoords;	\
		\
		/* Member info */ \
		ExtensionManager*                                  extensionMgr;                \
		Index                                              fieldComponentCount;         \
		Dimension_Index                                    dim;                         \
		MPI_Comm                                           communicator;                \
		FieldVariable_Register*                            fieldVariable_Register;      \
		Bool                                               isCheckpointedAndReloaded;    \

	struct FieldVariable { __FieldVariable };	

	/** General Virtual Functions */
	#define FieldVariable_Copy( self ) \
		(FieldVariable*)Stg_Class_Copy( self, NULL, False, NULL, NULL )

	/** Creation implementation */
	FieldVariable* FieldVariable_DefaultNew( Name name );

	FieldVariable* FieldVariable_New(		
		Name                                               name,
		FieldVariable_InterpolateValueAtFunction*          _interpolateValueAt,
		FieldVariable_GetValueFunction*                    _getMinGlobalFieldMagnitude,
		FieldVariable_GetValueFunction*                    _getMaxGlobalFieldMagnitude,		
		FieldVariable_GetCoordFunction*                    _getMinAndMaxLocalCoords,
		FieldVariable_GetCoordFunction*                    _getMinAndMaxGlobalCoords,
		Index                                              fieldComponentCount,
		Dimension_Index                                    dim,
		Bool                                               isCheckpointedAndReloaded,
		MPI_Comm                                           communicator,
		FieldVariable_Register*                            fieldVariable_Register ) ;
	
	FieldVariable* _FieldVariable_New(
		SizeT							_sizeOfSelf, 
		Type							type,
		Stg_Class_DeleteFunction*					_delete,
		Stg_Class_PrintFunction*					_print, 
		Stg_Class_CopyFunction*					_copy, 
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
		Stg_Component_ConstructFunction*			_construct,
		Stg_Component_BuildFunction*				_build,
		Stg_Component_InitialiseFunction*				_initialise,
		Stg_Component_ExecuteFunction*				_execute,
		Stg_Component_DestroyFunction*				_destroy,
		Name									name,
		Bool									initFlag,
		FieldVariable_InterpolateValueAtFunction*          _interpolateValueAt,
		FieldVariable_GetValueFunction*                    _getMinGlobalFieldMagnitude,
		FieldVariable_GetValueFunction*                    _getMaxGlobalFieldMagnitude,		
		FieldVariable_GetCoordFunction*                    _getMinAndMaxLocalCoords,
		FieldVariable_GetCoordFunction*                    _getMinAndMaxGlobalCoords,
		Index                                              fieldComponentCount,
		Dimension_Index                                    dim,
		Bool                                               isCheckpointedAndReloaded,
		MPI_Comm                                           communicator,
		FieldVariable_Register*                            fieldVariable_Register );

	/** Member initialisation implementation */
	void _FieldVariable_Init( 
			FieldVariable*                  self, 
			Index                           fieldComponentCount, 
			Dimension_Index                 dim,
			Bool                            isCheckpointedAndReloaded,
			MPI_Comm                        communicator, 
			FieldVariable_Register*         fV_Register ) ;
	
	void _FieldVariable_Delete( void* fieldVariable ) ;
	void _FieldVariable_Print( void* _fieldVariable, Stream* stream ) ;
	void _FieldVariable_Construct( void* fieldVariable, Stg_ComponentFactory* cf, void* data ) ;
	void _FieldVariable_Build( void* fieldVariable, void* data ) ;
	void _FieldVariable_Execute( void* fieldVariable, void* data ) ;
	void _FieldVariable_Destroy( void* fieldVariable, void* data ) ;
	void _FieldVariable_Initialise( void* fieldVariable, void* data ) ;
	
	/* Copy */
	void* _FieldVariable_Copy( void* fieldVariable, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/** Interface to determine the value of the field at a specific co-ordinate in space.
	Return status determines whether operation was successful - i.e. whether the given
	coordinate was valid, and within the processor's local domain*/
	InterpolationResult FieldVariable_InterpolateValueAt( void* fieldVariable, Coord coord, double* value );

	/** Interface for finding the minimum field value */
	double FieldVariable_GetMinGlobalFieldMagnitude( void* fieldVariable );

	/** Interface for finding the maximum field value */
	double FieldVariable_GetMaxGlobalFieldMagnitude( void* fieldVariable );

	/* Interface for finding the boundary of the spatial region this processor is holding info on */
	void FieldVariable_GetMinAndMaxLocalCoords( void* fieldVariable, Coord min, Coord max ) ;
	void FieldVariable_GetMinAndMaxGlobalCoords( void* fieldVariable, Coord min, Coord max ) ;
	void _FieldVariable_GetMinAndMaxGlobalCoords( void* fieldVariable, Coord min, Coord max ) ;


#endif /* __Discretisation_Utils_FieldVariable_h__ */
