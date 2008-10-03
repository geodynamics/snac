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
** $Id: FieldVariable.c 3884 2006-10-26 05:26:19Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"

#include "types.h"
#include "FieldVariable.h"
#include "DiscretisationContext.h"
#include "FieldVariable_Register.h"

#include <assert.h>

const Type FieldVariable_Type = "FieldVariable";

const char* InterpolationResultToStringMap[4] = {
	"OTHER_PROC",
	"LOCAL",
	"SHADOW",
	"OUTSIDE_GLOBAL"
	};

FieldVariable* FieldVariable_DefaultNew( Name name )
{
		return _FieldVariable_New( 
			sizeof(FieldVariable), 
			FieldVariable_Type, 
			_FieldVariable_Delete, 
			_FieldVariable_Print,
			_FieldVariable_Copy, 
			(Stg_Component_DefaultConstructorFunction*)FieldVariable_DefaultNew,
			_FieldVariable_Construct,
			_FieldVariable_Build, 
			_FieldVariable_Initialise, 
			_FieldVariable_Execute, 
			_FieldVariable_Destroy, 
			name,
			False,
			NULL, 
			NULL,
			NULL, 
			NULL,
			NULL, 
			0,
			0,
			False,
			MPI_COMM_WORLD,
			NULL);
}

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
		FieldVariable_Register*                            fieldVariable_Register ) 
{
	return _FieldVariable_New(
			sizeof(FieldVariable),
			FieldVariable_Type,
			_FieldVariable_Delete,
			_FieldVariable_Print,
			_FieldVariable_Copy, 
			(Stg_Component_DefaultConstructorFunction*)FieldVariable_DefaultNew,
			_FieldVariable_Construct,
			_FieldVariable_Build, 
			_FieldVariable_Initialise, 
			_FieldVariable_Execute, 
			_FieldVariable_Destroy,
			name,
			True,
			_interpolateValueAt,
			_getMinGlobalFieldMagnitude,
			_getMaxGlobalFieldMagnitude,		
			_getMinAndMaxLocalCoords,
			_getMinAndMaxGlobalCoords,
			fieldComponentCount,
			dim,
            isCheckpointedAndReloaded,
			communicator,
			fieldVariable_Register );
}

FieldVariable* _FieldVariable_New(
 		SizeT                                       _sizeOfSelf, 
		Type                                        type,
		Stg_Class_DeleteFunction*                   _delete,
		Stg_Class_PrintFunction*                    _print, 
		Stg_Class_CopyFunction*	                    _copy, 
		Stg_Component_DefaultConstructorFunction*   _defaultConstructor,
		Stg_Component_ConstructFunction*            _construct,
		Stg_Component_BuildFunction*                _build,
		Stg_Component_InitialiseFunction*           _initialise,
		Stg_Component_ExecuteFunction*              _execute,
		Stg_Component_ExecuteFunction*              _destroy,
		Name                                        name,
		Bool                                        initFlag,
		FieldVariable_InterpolateValueAtFunction*   _interpolateValueAt,
		FieldVariable_GetValueFunction*                    _getMinGlobalFieldMagnitude,
		FieldVariable_GetValueFunction*                    _getMaxGlobalFieldMagnitude,		
		FieldVariable_GetCoordFunction*                    _getMinAndMaxLocalCoords,
		FieldVariable_GetCoordFunction*                    _getMinAndMaxGlobalCoords,
		Index                                              fieldComponentCount,
		Dimension_Index                                    dim,
		Bool                                               isCheckpointedAndReloaded,
		MPI_Comm                                           communicator,
		FieldVariable_Register*                            fieldVariable_Register )		
{
	FieldVariable*		self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(FieldVariable) );
	self = (FieldVariable*)_Stg_Component_New( _sizeOfSelf, type, _delete, _print, _copy, _defaultConstructor,
			_construct, _build, _initialise, _execute, _destroy,
		name, NON_GLOBAL );
	
	/* Virtual functions */
	self->_interpolateValueAt         = _interpolateValueAt;
	self->_getMinGlobalFieldMagnitude = _getMinGlobalFieldMagnitude;
	self->_getMaxGlobalFieldMagnitude = _getMaxGlobalFieldMagnitude;
	self->_getMinAndMaxLocalCoords    = _getMinAndMaxLocalCoords;
	self->_getMinAndMaxGlobalCoords   = _getMinAndMaxGlobalCoords;

	/* General info */
	/* FieldVariable info */
	if( initFlag ){
		_FieldVariable_Init( self, fieldComponentCount, dim, isCheckpointedAndReloaded, 
			communicator, fieldVariable_Register );
	}
	
	return self;
}

void _FieldVariable_Delete( void* fieldVariable ) {
	FieldVariable* self = (FieldVariable*) fieldVariable;

	if( self->extensionMgr ) {
		Stg_Class_Delete( self->extensionMgr );
	}
	_Stg_Component_Delete( self );
}

void _FieldVariable_Print( void* _fieldVariable, Stream* stream ) {
	FieldVariable* self = (FieldVariable*) _fieldVariable;

	Journal_Printf( stream, "FieldVariable - '%s'\n", self->name );
	Stream_Indent( stream );
	_Stg_Component_Print( self, stream );

	Journal_PrintPointer( stream, self->_interpolateValueAt );
	Journal_PrintPointer( stream, self->_getMinGlobalFieldMagnitude );
	Journal_PrintPointer( stream, self->_getMaxGlobalFieldMagnitude );
	Journal_PrintPointer( stream, self->_getMinAndMaxLocalCoords );
	Journal_PrintPointer( stream, self->_getMinAndMaxGlobalCoords );

	Journal_PrintValue( stream, self->fieldComponentCount );
	Journal_PrintValue( stream, self->dim );
	Journal_PrintBool( stream, self->isCheckpointedAndReloaded);
	#ifdef LAM_MPI
		Journal_PrintPointer( stream, self->communicator );
	#elif  defined( OPEN_MPI )
		Journal_PrintPointer( stream, self->communicator );
	#else
		Journal_PrintValue( stream, self->communicator );
	#endif
	Journal_PrintPointer( stream, self->fieldVariable_Register );
	Stream_UnIndent( stream );
}

void _FieldVariable_Init( 
		FieldVariable*                                     self, 
		Index                                              fieldComponentCount, 
		Dimension_Index                                    dim,
		Bool                                               isCheckpointedAndReloaded,
		MPI_Comm                                           communicator, 
		FieldVariable_Register*                            fV_Register ) {
	/* Add ourselves to the register for later retrieval by clients */
	self->isConstructed = True;

	self->fieldComponentCount         = fieldComponentCount;
	self->dim                         = dim;
	self->communicator                = communicator;
	self->fieldVariable_Register      = fV_Register;
	//TODO put comment here
	self->isCheckpointedAndReloaded   = isCheckpointedAndReloaded;
	if (self != NULL && fV_Register != NULL)	
		FieldVariable_Register_Add( fV_Register, self );

	self->extensionMgr = ExtensionManager_New_OfExistingObject( self->name, self );
}


void* _FieldVariable_Copy( void* fieldVariable, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	FieldVariable*	self = (FieldVariable*)fieldVariable;
	FieldVariable*	newFieldVariable;
	PtrMap*			map = ptrMap;
	Bool			ownMap = False;
	
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newFieldVariable = _Stg_Component_Copy( self, dest, deep, nameExt, map );
	
	newFieldVariable->_interpolateValueAt        = self->_interpolateValueAt;
	newFieldVariable->_getMinAndMaxLocalCoords   = self->_getMinAndMaxLocalCoords;
	newFieldVariable->_getMinAndMaxGlobalCoords  = self->_getMinAndMaxGlobalCoords;

	newFieldVariable->fieldComponentCount        = self->fieldComponentCount;
	newFieldVariable->dim                        = self->dim;
	newFieldVariable->isCheckpointedAndReloaded  = self->isCheckpointedAndReloaded;
	newFieldVariable->communicator               = self->communicator;
	newFieldVariable->fieldVariable_Register     =  self->fieldVariable_Register;

	newFieldVariable->extensionMgr               = Stg_Class_Copy( self->extensionMgr, NULL, deep, nameExt, map );
	
	if( ownMap ) {
		Stg_Class_Delete( map );
	}
				
	return (void*)newFieldVariable;
}

void _FieldVariable_Construct( void* fieldVariable, Stg_ComponentFactory* cf, void* data ) {
	FieldVariable*	        self         = (FieldVariable*)fieldVariable;
	FieldVariable_Register* fV_Register;
	Dimension_Index         dim;
	Index                   fieldComponentCount;
	Bool                    isCheckpointedAndReloaded;
	
	fV_Register = (FieldVariable_Register*) Stg_ObjectList_Get( cf->registerRegister, "FieldVariable_Register" );
	assert( fV_Register );

	dim = Stg_ComponentFactory_GetRootDictUnsignedInt( cf, "dim", 0 );

	fieldComponentCount = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, "fieldComponentCount", 0 );
	/* DiscretisationContext will be setting isCheckpointedAndReloaded later
		depending on the contents of the list "FieldVariablesToCheckpoint". 
		May want to change this later so it is set individually in each
		FeVariable's component xml entry.
		--PatrickSunter & KathleenHumble, 27/09/06	*/
	isCheckpointedAndReloaded = True;
	_FieldVariable_Init( self, fieldComponentCount, dim, isCheckpointedAndReloaded, 
		MPI_COMM_WORLD, fV_Register );
	
}

void _FieldVariable_Build( void* fieldVariable, void* data ) {

}

void _FieldVariable_Initialise( void* fieldVariable, void* data ) {

}

void _FieldVariable_Execute( void* fieldVariable, void* data ) {

}

void _FieldVariable_Destroy( void* fieldVariable, void* data ) {

}

InterpolationResult FieldVariable_InterpolateValueAt( void* fieldVariable, Coord coord, double* value ) {
	FieldVariable*	self = (FieldVariable*)fieldVariable;

	return self->_interpolateValueAt( self, coord, value );
}

double FieldVariable_GetMinGlobalFieldMagnitude( void* fieldVariable ) {
	FieldVariable*	self = (FieldVariable*)fieldVariable;
	return self->_getMinGlobalFieldMagnitude( self );
}


double FieldVariable_GetMaxGlobalFieldMagnitude( void* fieldVariable ) {
	FieldVariable*	self = (FieldVariable*)fieldVariable;
	return self->_getMaxGlobalFieldMagnitude( self );
}


void FieldVariable_GetMinAndMaxLocalCoords( void* fieldVariable, Coord min, Coord max ) {
	FieldVariable*	self = (FieldVariable*)fieldVariable;

	self->_getMinAndMaxLocalCoords( self, min, max );
}

void FieldVariable_GetMinAndMaxGlobalCoords( void* fieldVariable, Coord min, Coord max ) {
	FieldVariable*	self = (FieldVariable*)fieldVariable;

	self->_getMinAndMaxGlobalCoords( self, min, max );
}

void _FieldVariable_GetMinAndMaxGlobalCoords( void* fieldVariable, Coord globalMin, Coord globalMax ) {
	FieldVariable*	self = (FieldVariable*)fieldVariable;
	Coord localMin, localMax;

	self->_getMinAndMaxLocalCoords( self, localMin, localMax );

	MPI_Allreduce( localMin, globalMin, self->dim, MPI_DOUBLE, MPI_MIN, self->communicator );
	MPI_Allreduce( localMax, globalMax, self->dim, MPI_DOUBLE, MPI_MAX, self->communicator );
}
