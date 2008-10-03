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
** $Id: HexaMD.c 3883 2006-10-26 05:00:23Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"

#include "types.h"
#include "shortcuts.h"
#include "MeshDecomp.h"
#include "HexaMD.h"
#include "ElementLayout.h"
#include "NodeLayout.h"
#include "CornerNL.h"
#include "BodyNL.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>

/* Textual name of this class */
const Type HexaMD_Type = "HexaMD";
const Name HexaMDIJK26TopologyName = "defaultHexaMDIJK26TopologyName";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

HexaMD* HexaMD_DefaultNew( Name name )
{
	return (HexaMD*)_HexaMD_New( 
		sizeof(HexaMD), 
		HexaMD_Type, 
		_HexaMD_Delete, 
		_HexaMD_Print,
		_HexaMD_Copy,
		(Stg_Component_DefaultConstructorFunction*)HexaMD_DefaultNew,
		_HexaMD_Construct,
		_HexaMD_Build,
		_HexaMD_Initialise,
		_HexaMD_Execute,
		(Stg_Component_DestroyFunction*)_HexaMD_Destroy,
		name,
		False,
		// TODO: these should all be over-ridden with fast specific versions using the existing macros,
		//	which are maybe in regMeshUtils ?
		_MeshDecomp_Node_LocalToGlobal1D,
		_MeshDecomp_Node_DomainToGlobal1D,
		_MeshDecomp_Node_ShadowToGlobal1D, 
		_MeshDecomp_Node_GlobalToLocal1D, 
		_MeshDecomp_Node_GlobalToDomain1D, 
		_MeshDecomp_Node_GlobalToShadow1D,
		_MeshDecomp_Element_LocalToGlobal1D,
		_MeshDecomp_Element_DomainToGlobal1D,
		_MeshDecomp_Element_ShadowToGlobal1D, 
		_MeshDecomp_Element_GlobalToLocal1D, 
		_MeshDecomp_Element_GlobalToDomain1D, 
		_MeshDecomp_Element_GlobalToShadow1D,
		_HexaMD_Shadow_ProcCount,
		_HexaMD_Shadow_BuildProcs,
		_MeshDecomp_Shadow_ProcElementCount,
		_MeshDecomp_Proc_WithElement,
		NULL,
		MPI_COMM_WORLD,
		NULL,
		NULL,
		0 );
}

HexaMD* HexaMD_New_All(
		Name							name,
		Dictionary*					dictionary,
		MPI_Comm					communicator,
		ElementLayout*					eLayout,
		NodeLayout*					nLayout,
		unsigned char					numPartitionedDims )
{
	return _HexaMD_New( 
		sizeof(HexaMD), 
		HexaMD_Type, 
		_HexaMD_Delete, 
		_HexaMD_Print,
		_HexaMD_Copy,
		(Stg_Component_DefaultConstructorFunction*)HexaMD_DefaultNew,
		_HexaMD_Construct,
		_HexaMD_Build,
		_HexaMD_Initialise,
		_HexaMD_Execute,
		(Stg_Component_DestroyFunction*)_HexaMD_Destroy,
		name,
		True,
		_MeshDecomp_Node_LocalToGlobal1D,
		_MeshDecomp_Node_DomainToGlobal1D,
		_MeshDecomp_Node_ShadowToGlobal1D, 
		_MeshDecomp_Node_GlobalToLocal1D, 
		_MeshDecomp_Node_GlobalToDomain1D, 
		_MeshDecomp_Node_GlobalToShadow1D,
		_MeshDecomp_Element_LocalToGlobal1D,
		_MeshDecomp_Element_DomainToGlobal1D,
		_MeshDecomp_Element_ShadowToGlobal1D, 
		_MeshDecomp_Element_GlobalToLocal1D, 
		_MeshDecomp_Element_GlobalToDomain1D, 
		_MeshDecomp_Element_GlobalToShadow1D,
		_HexaMD_Shadow_ProcCount,
		_HexaMD_Shadow_BuildProcs,
		_MeshDecomp_Shadow_ProcElementCount,
		_MeshDecomp_Proc_WithElement,
		dictionary,
		communicator,
		eLayout,
		nLayout,
		numPartitionedDims );
}

void HexaMD_Init(
		HexaMD*						self,
		Name							name,
		Dictionary*					dictionary,
		MPI_Comm					communicator,
		ElementLayout*					eLayout,
		NodeLayout*					nLayout,
		unsigned char					numPartitionedDims )
{
	/* General info */
	self->type = HexaMD_Type;
	self->_sizeOfSelf = sizeof(HexaMD);
	self->_deleteSelf = False;
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->_delete = _HexaMD_Delete;
	self->_print = _HexaMD_Print;
	self->_copy = _HexaMD_Copy;
	self->_defaultConstructor = (Stg_Component_DefaultConstructorFunction*)HexaMD_DefaultNew;
	self->_construct = _HexaMD_Construct;
	self->_build = _HexaMD_Build;
	self->_initialise = _HexaMD_Initialise;
	self->_execute = _HexaMD_Execute;
	self->_destroy = (Stg_Component_DestroyFunction*)_HexaMD_Destroy;
	// TODO: these should all be over-ridden with fast specific versions using the existing macros,
	//	which are maybe in regMeshUtils ?
	self->nodeMapLocalToGlobal = _MeshDecomp_Node_LocalToGlobal1D;
	self->nodeMapDomainToGlobal = _MeshDecomp_Node_DomainToGlobal1D;
	self->nodeMapShadowToGlobal = _MeshDecomp_Node_ShadowToGlobal1D;
	self->nodeMapGlobalToLocal = _MeshDecomp_Node_GlobalToLocal1D;
	self->nodeMapGlobalToDomain = _MeshDecomp_Node_GlobalToDomain1D;
	self->nodeMapGlobalToShadow = _MeshDecomp_Node_GlobalToShadow1D;
	self->elementMapLocalToGlobal = _MeshDecomp_Element_LocalToGlobal1D;
	self->elementMapDomainToGlobal = _MeshDecomp_Element_DomainToGlobal1D;
	self->elementMapShadowToGlobal = _MeshDecomp_Element_ShadowToGlobal1D;
	self->elementMapGlobalToLocal = _MeshDecomp_Element_GlobalToLocal1D;
	self->elementMapGlobalToDomain = _MeshDecomp_Element_GlobalToDomain1D;
	self->elementMapGlobalToShadow = _MeshDecomp_Element_GlobalToShadow1D;
	self->shadowProcCount = _HexaMD_Shadow_ProcCount;
	self->shadowBuildProcs = _HexaMD_Shadow_BuildProcs;
	self->shadowProcElementCount = _MeshDecomp_Shadow_ProcElementCount;
	self->procWithElement = _MeshDecomp_Proc_WithElement;


	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	_Stg_Component_Init( (Stg_Component*)self );
	_MeshDecomp_Init( (MeshDecomp*)self, communicator, eLayout, nLayout, NULL );
	
	/* HexaMD info */
	_HexaMD_Init( self, numPartitionedDims );
}


HexaMD* _HexaMD_New(
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print,
		Stg_Class_CopyFunction*				_copy, 
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
		Stg_Component_ConstructFunction*			_construct,
		Stg_Component_BuildFunction*				_build,
		Stg_Component_InitialiseFunction*			_initialise,
		Stg_Component_ExecuteFunction*				_execute,
		Stg_Component_DestroyFunction*				_destroy,
		Name									name,
		Bool									initFlag,
		MeshDecomp_Node_MapLocalToGlobalFunction*       nodeMapLocalToGlobal,
		MeshDecomp_Node_MapDomainToGlobalFunction*      nodeMapDomainToGlobal,
		MeshDecomp_Node_MapShadowToGlobalFunction*      nodeMapShadowToGlobal,
		MeshDecomp_Node_MapGlobalToLocalFunction*       nodeMapGlobalToLocal,
		MeshDecomp_Node_MapGlobalToDomainFunction*      nodeMapGlobalToDomain,
		MeshDecomp_Node_MapGlobalToShadowFunction*      nodeMapGlobalToShadow,
		MeshDecomp_Element_MapLocalToGlobalFunction*	elementMapLocalToGlobal,
		MeshDecomp_Element_MapDomainToGlobalFunction*	elementMapDomainToGlobal,
		MeshDecomp_Element_MapShadowToGlobalFunction*	elementMapShadowToGlobal,
		MeshDecomp_Element_MapGlobalToLocalFunction*	elementMapGlobalToLocal,
		MeshDecomp_Element_MapGlobalToDomainFunction*	elementMapGlobalToDomain,
		MeshDecomp_Element_MapGlobalToShadowFunction*	elementMapGlobalToShadow,
		MeshDecomp_Shadow_ProcCountFunction*		shadowProcCount,
		MeshDecomp_Shadow_BuildProcsFunction*		shadowBuildProcs,
		MeshDecomp_Shadow_ProcElementCountFunction*     shadowProcElementCount,
		MeshDecomp_Proc_WithElementFunction*		procWithElement,
		Dictionary*					dictionary,
		MPI_Comm					communicator,
		ElementLayout*					eLayout,
		NodeLayout*					nLayout,
		unsigned char					numPartitionedDims )
{
	HexaMD* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(HexaMD) );
	self = (HexaMD*)_MeshDecomp_New(
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
		nodeMapLocalToGlobal, 
		nodeMapDomainToGlobal,
		nodeMapShadowToGlobal,
		nodeMapGlobalToLocal,
		nodeMapGlobalToDomain,
		nodeMapGlobalToShadow, 
		elementMapLocalToGlobal,
		elementMapDomainToGlobal,
		elementMapShadowToGlobal, 
		elementMapGlobalToLocal,
		elementMapGlobalToDomain, 
		elementMapGlobalToShadow,
		shadowProcCount,
		shadowBuildProcs,
		shadowProcElementCount,
		procWithElement,
		dictionary, 
		communicator,
		eLayout,
		nLayout );
	
	/* General info */
	
	/* Virtual info */
	
	/* HexaMD info */
	if( initFlag ){
		_HexaMD_Init( self, numPartitionedDims );
	}
	
	return self;
}

#define HMD_E_3DTo1D( self, i, j, k ) \
	(((self)->elementGlobal3DCounts[2] ? (k) * ((self)->elementGlobal3DCounts[0] ? (self)->elementGlobal3DCounts[0] : 1) * \
		((self)->elementGlobal3DCounts[1] ? (self)->elementGlobal3DCounts[1] : 1) : 0) + \
		((self)->elementGlobal3DCounts[1] ? (j) * ((self)->elementGlobal3DCounts[0] ? \
		(self)->elementGlobal3DCounts[0] : 1) : 0) + \
		((self)->elementGlobal3DCounts[0] ? (i) : 0))


void _HexaMD_Init(
		HexaMD*						self,
		unsigned char					numPartitionedDims )
{
	/* General and Virtual info should already be set */
	
	/* HexaMD info */
	Partition_Index		proc_I;
	Index			axis;
	unsigned char		axisIJK[3];
	Element_GlobalIndex*	localSizes[3];
	Element_GlobalIndex*	localOffsets[3];
	Index			dim_I;
	Stream*			debug;
	Stream*			error;
	SizeT			candidateSize;
	IJK*			candidate;
	IJK*			candidateIJK;
	Index			candidateCount;
	Index*			candidateResidual;
	IJK			ijk;
	Index			candidate_I;
	IndexSet*		filter;
	Index			filterCount;
	IndexSet*		bestFilter;
	Index			bestCount;
	Index			lowest;
	Index			highest;
	Element_GlobalIndex	ecI[3]; /* frequently accessed... cached on stack */
	double			ecD[3]; /* frequently accessed... cached on stack */
	Index			bestCandidate_I;
	double			lowestD;

	/* Assign function pointers in case the decomp is serial */
	MeshDecomp_OverrideMethodsIfSerial(self);
	
	debug = Journal_Register( DebugStream_Type, HexaMD_Type );
	error = Journal_Register( ErrorStream_Type, HexaMD_Type );
	
	self->isConstructed = True;
	self->numPartitionedDims = (numPartitionedDims <= 3) ? numPartitionedDims : 3;
	
	Journal_Firewall( 
		self->nodeLayout->nodeCount > 1,
		error,
		"Error: %s component \"%s\" asked to decompose mesh based on NodeLayout \"%s\", but "
		"provided NodeLayout has only 1 node. Did you specify a valid input file? Did you specify "
		"the number of elements/nodes in your input file?\n",
		self->type, self->name, self->nodeLayout->name );

	self->nodeGlobalCount = self->nodeLayout->nodeCount;
	self->elementGlobalCount = self->elementLayout->elementCount;
	
	/* Start Steve 20040103 */
	
	/* Changed by Patrick Sunter, 22/7/2004, so Body meshes work properly
	(which have same number of elements and nodes in each axis ) */
	for (dim_I = 0; dim_I < 3; dim_I++) {
		self->nodeGlobal3DCounts[dim_I] = ((IJKTopology*)self->nodeLayout->topology)->size[dim_I];
		/* element global counts stored as an integer */
		ecI[dim_I] = self->elementGlobal3DCounts[dim_I] = ((IJKTopology*)self->elementLayout->topology)->size[dim_I];
		/* element global counts stored as a double */
		ecD[dim_I] = (double)ecI[dim_I];
	}
	Journal_Firewall( 
		self->nodeGlobal3DCounts[I_AXIS] * self->nodeGlobal3DCounts[J_AXIS] * self->nodeGlobal3DCounts[K_AXIS],
		error,
		"Error: you specified node Global counts of { %u, %u, %u }, but mesh must have at least 1 node in each dimension "
		"(If you want a 2d mesh set node count to 1 in one of the dimensions.\n",
		self->nodeGlobal3DCounts[0], 
		self->nodeGlobal3DCounts[1], 
		self->nodeGlobal3DCounts[2] );
	Journal_Printf( debug, "nodeGlobalCount: %u, elementGlobalCount: %u\n", self->nodeGlobalCount, self->elementGlobalCount );
	Journal_Printf( 
		debug, 
		"nodeGlobal3DCount: { %u, %u, %u }, elementGlobal3DCount: { %u, %u, %u }\n",
		self->nodeGlobal3DCounts[0], 
		self->nodeGlobal3DCounts[1], 
		self->nodeGlobal3DCounts[2], 
		ecI[0],
		ecI[1],
		ecI[2] );
	
	/* Phase 1:Find all combinations of i, j, k that multiply to nproc. We do this by trying all i,j,k = 0->nproc combinations.
	   Once the combination is > nproc, we can forfiet the rest of the loop */
	candidateSize = self->nproc; /* initial guess */
	candidate = Memory_Alloc_Array( IJK, candidateSize, "candidate" );
	candidateCount = 0;
	for( ijk[2] = 1; ijk[2] <= self->nproc; ijk[2]++ ) {
		for( ijk[1] = 1; ijk[1] <= self->nproc; ijk[1]++ ) {
			for( ijk[0] = 1; ijk[0] <= self->nproc; ijk[0]++ ) {
				Index total = ijk[0] * ijk[1] * ijk[2];
				
				if( total == self->nproc ) {
					Index			dim_I;
					
					/* If list is full... increase array size */
					if( candidateCount == candidateSize ) {
						candidateSize += self->nproc;
						candidate = Memory_Realloc_Array( candidate, IJK, candidateSize );
					}
					
					for( dim_I = 0; dim_I < 3; dim_I++ ) {
						candidate[candidateCount][dim_I] = ijk[dim_I];
					}
					candidateCount++;
					break;
				}
				else if( total > self->nproc ) {
					break;
				}
			}
		
		}
	}
	#ifdef DEBUG
	{
		Index			candidate_I;
		
		Journal_Printf( debug, "Decomposition candidates count (phase 1): %u\n", candidateCount );
		Journal_Printf( debug, "Decomposition candidates (phase 1): { " );
		for( candidate_I = 0; candidate_I < candidateCount; candidate_I++ ) {
			Journal_Printf( 
				debug, 
				"{ %u, %u, %u } ", 
				candidate[candidate_I][0], 
				candidate[candidate_I][1], 
				candidate[candidate_I][2] );
		}
		Journal_Printf( debug, "}\n" );
	}
	#endif
	
	/* Phase 2: Cull out candidates of an order of decomposition greater than asked for. */
	filter = IndexSet_New( candidateCount );
	filterCount = 0;
	for( candidate_I = 0; candidate_I < candidateCount; candidate_I++ ) {
		Bool			decomposeAxis[3];
		Index			decomposeCount;

		decomposeCount = 0;
		for( dim_I = 0; dim_I < 3; dim_I++ ) {
			decomposeAxis[dim_I] = candidate[candidate_I][dim_I] > 1 ? True : False;
			decomposeCount += decomposeAxis[dim_I];
		}
		
		if( decomposeCount <= self->numPartitionedDims ) {
			IndexSet_Add( filter, candidate_I );
			filterCount++;
		}
	}
	#ifdef DEBUG
	{
		Index			candidate_I;
		
		Journal_Printf( debug, "Decomposition candidates count (phase 2): %u\n", filterCount );
		Journal_Printf( debug, "Decomposition candidates (phase 2): { " );
		for( candidate_I = 0; candidate_I < candidateCount; candidate_I++ ) {
			if( IndexSet_IsMember( filter, candidate_I ) ) {
				Journal_Printf( 
					debug, 
					"{ %u, %u, %u } ", 
					candidate[candidate_I][0], 
					candidate[candidate_I][1], 
					candidate[candidate_I][2] );
			}
		}
		Journal_Printf( debug, "}\n" );
	}
	#endif
	
	/* Phase 3: Make sure there are enough elements to decompose across, then calculate
	    the residual elements.  Note that residual processors have not yet been
		    considered. */
	candidateIJK = Memory_Alloc_Array( IJK, candidateCount, "candidateIJK" );
	candidateResidual = Memory_Alloc_Array( Index, candidateCount, "candidateResidual" );
	for( candidate_I = 0; candidate_I < candidateCount; candidate_I++ ) {
		Index   dim_I;
		Index   residual[3];
		
		for( dim_I = 0; dim_I < 3; dim_I++ ) {
			candidateIJK[candidate_I][dim_I] = candidate[candidate_I][dim_I] <= ecI[dim_I] ?
				candidate[candidate_I][dim_I] : ecI[dim_I];
			residual[dim_I] = 
				ecI[dim_I] - 
				ecI[dim_I] / candidateIJK[candidate_I][dim_I] * 
				candidateIJK[candidate_I][dim_I];
		}
		
		if( residual[0] || residual[1] || residual[2] ) {
			candidateResidual[candidate_I] = 
				residual[0] ? residual[0] : 1 * 
				residual[1] ? residual[1] : 1 * 
				residual[2] ? residual[2] : 1;
		}
		else {
			candidateResidual[candidate_I] = 0;
		}
	}

/* This formula could be useful for direct calculation of the decomposition, so I thought it best
   not to delete it. */
#if 0
	/* Phase 3 (a): Work out the element counts...
	   fi * fj * fk = pi * pj * pk = fPrime, where px > 1, else fx = ex = px = 1
	                  --   --   --
	                  ei   ej   ek
	   fx = proc to element fraction in dir, px = proc count in dir, ex = element count in dir
	   
	   Logically px     = fx
	             ------   ------
	             p(x+1)   f(x+1)
	*/
	candidateIJK = Memory_Alloc_Array( IJK, candidateCount, "candidateIJK" );
	candidateResidual = Memory_Alloc_Array( Index, candidateCount, "candidateResidual" );
	for( candidate_I = 0; candidate_I < candidateCount; candidate_I++ ) {
		/* Only work out legitimate values for real candidates */
		if( IndexSet_IsMember( filter, candidate_I ) ) {
			Bool			decomposeAxis[3];
			Index			decomposeCount;
			Index			dim_I;
			double			f[3];
			Index			residual[3];
			
			/* if decomposed in a dimension, calculate inverse element count, store in f[dim_I] */
			decomposeCount = 0;
			for( dim_I = 0; dim_I < 3; dim_I++ ) {
				decomposeAxis[dim_I] = candidate[candidate_I][dim_I] > 1 ? True : False;
				decomposeCount += decomposeAxis[dim_I];
				f[dim_I] = (double)1.0f / ecD[dim_I];
			}
			
			if( decomposeCount == 0 ) {
				/* Do nothing... no decomposition */
			}
			else if( decomposeCount == 1 ) {
				Index			dim_I1 = decomposeAxis[0] ? 0 : decomposeAxis[1] ? 1 : 2;
				
				f[dim_I1] = (double)candidate[candidate_I][dim_I1] / ecD[dim_I1];
			}
			else if( decomposeCount == 2 ) {
				Index			dim_I1 = decomposeAxis[0] ? 0 : 1;
				Index			dim_I2 = dim_I1 == 0 ? (decomposeAxis[1] ? 1 : 2) : 2;
				double			fPrime;
				double			ratio;
				
				fPrime = 
					(double)candidate[candidate_I][dim_I1] * 
					(double)candidate[candidate_I][dim_I2] / 
					ecD[dim_I1] /
					ecD[dim_I2];
				
				ratio = (double)candidate[candidate_I][dim_I1] / (double)candidate[candidate_I][dim_I2];
				
				f[dim_I2] = pow( fPrime / ratio, 1.0f / 2.0f );
				f[dim_I1] = ratio * f[dim_I2];
			}
			else if( decomposeCount == 3 ) {
				double			fPrime;
				double			ratio1;
				double			ratio2;
				
				fPrime = 
					(double)candidate[candidate_I][0] * 
					(double)candidate[candidate_I][1] * 
					(double)candidate[candidate_I][2] / 
					ecD[0] /
					ecD[1] /
					ecD[2];
				
				ratio1 = (double)candidate[candidate_I][0] / (double)candidate[candidate_I][1];
				ratio2 = (double)candidate[candidate_I][1] / (double)candidate[candidate_I][2];
				
				f[2] = pow( fPrime / ratio1 / ratio2 / ratio2, 1.0f / 3.0f );
				f[1] = ratio2 * f[2];
				f[0] = ratio1 * f[1];
			}
			else {
				Journal_Firewall( 
					0, 
					error,
					"Illegal number of decomposition axis.\n" );
			}
			
			for( dim_I = 0; dim_I < 3; dim_I++ ) {
				candidateIJK[candidate_I][dim_I] = rint( f[dim_I] * ecD[dim_I] );
				residual[dim_I] = 
					ecI[dim_I] - 
					ecI[dim_I] / candidateIJK[candidate_I][dim_I] * 
					candidateIJK[candidate_I][dim_I];
			}
			if( residual[0] || residual[1] || residual[2] ) {
				candidateResidual[candidate_I] = 
					residual[0] ? residual[0] : 1 * 
					residual[1] ? residual[1] : 1 * 
					residual[2] ? residual[2] : 1;
			}
			else {
				candidateResidual[candidate_I] = 0;
			}
		}
		else {
			Index			dim_I;
			
			for( dim_I = 0; dim_I < 3; dim_I++ ) {
				candidateIJK[candidate_I][dim_I] = 0;
			}
			candidateResidual[candidate_I] = -1;
		}
	}
#endif
	#ifdef DEBUG
	{
		Index			candidate_I;
		
		Journal_Printf( debug, "Decomposition candidates element counts & residual (phase 3): { " );
		for( candidate_I = 0; candidate_I < candidateCount; candidate_I++ ) {
			if( IndexSet_IsMember( filter, candidate_I ) ) {
				Journal_Printf( 
					debug, 
					"{ %u, %u, %u (%u) } ", 
					candidateIJK[candidate_I][0], 
					candidateIJK[candidate_I][1], 
					candidateIJK[candidate_I][2],
					candidateResidual[candidate_I] );
			}
		}
		Journal_Printf( debug, "}\n" );
	}
	#endif
	
	
	/* Phase 3 (b): Cull out those with residuals (unless allowUnsed or allowUnBalanced is enabled), or are no longer the 
	   processor count*/
	if( self->allowUnusedCPUs || self->allowUnbalancing ) {
		/* Do nothing... code further on will deal with the residuals */
	}
	else {
		Index			count;
		
		count = 0;
		for( candidate_I = 0; candidate_I < candidateCount; candidate_I++ ) {
			/* Only for those that we have considered so far as real candidates */
			if( IndexSet_IsMember( filter, candidate_I ) ) {
				/* If it has a residual, it is no longer a candidate */
				if( candidateResidual[candidate_I] ) {
					IndexSet_Remove( filter, candidate_I );
					filterCount--;
				}
				else if( 
					( candidateIJK[candidate_I][0] * 
					  candidateIJK[candidate_I][1] * 
					  candidateIJK[candidate_I][2] ) != self->nproc ) 
				{
					IndexSet_Remove( filter, candidate_I );
					filterCount--;
				}
				else {
					count++;
				}
			}
		}
		
		/* If there are no more candidates left, inform and error */
		if( !count ) {
			Journal_Firewall( 
				0, 
				error,
				"Cannot decompose mesh of dimensions %u %u %u by %u processors allowing for upto %uD decomposition (unbalancing and/or unused cpus disallowed).\n",
				ecI[0],
				ecI[1],
				ecI[2],
				self->nproc,
				self->numPartitionedDims );
		}
	}
	#ifdef DEBUG
	{
		Index			candidate_I;
		
		Journal_Printf( debug, "Decomposition candidates count (phase 3): %u\n", filterCount );
		Journal_Printf( debug, "Decomposition candidates (phase 3): { " );
		for( candidate_I = 0; candidate_I < candidateCount; candidate_I++ ) {
			if( IndexSet_IsMember( filter, candidate_I ) ) {
				Journal_Printf( 
					debug, 
					"{ %u, %u, %u } ", 
					candidateIJK[candidate_I][0], 
					candidateIJK[candidate_I][1], 
					candidateIJK[candidate_I][2] );
			}
		}
		Journal_Printf( debug, "}\n" );
	}
	#endif
	
	/* Phase 4a. Find the highest decomposition count. */
	highest = 0;
	for( candidate_I = 0; candidate_I < candidateCount; candidate_I++ ) {
		if( IndexSet_IsMember( filter, candidate_I ) ) {
			Bool			decomposeAxis[3];
			Index			decomposeCount;
			
			decomposeCount = 0;
			for( dim_I = 0; dim_I < 3; dim_I++ ) {
				decomposeAxis[dim_I] = candidateIJK[candidate_I][dim_I] > 1 ? True : False;
				decomposeCount += decomposeAxis[dim_I];
			}
			
			if( decomposeCount > highest ) {
				highest = decomposeCount;
			}
		}
	}
	/* TODO: Steve, this firewall causes it to always exit when using 1 processor...please fix*/
	/*Journal_Firewall( highest != 0, error, "Highest decomposition count not found... something is wrong!\n" );*/
	Journal_DPrintf( debug, "Decomposition count highest value: %u\n", highest );
	
	/* Phase 4b. Choose the best decomposition(s) based on using the highest decomposition count. */
	bestFilter = IndexSet_New( candidateCount );
	bestCount = 0;
	for( candidate_I = 0; candidate_I < candidateCount; candidate_I++ ) {
		if( IndexSet_IsMember( filter, candidate_I ) ) {
			Bool			decomposeAxis[3];
			Index			decomposeCount;
			
			decomposeCount = 0;
			for( dim_I = 0; dim_I < 3; dim_I++ ) {
				decomposeAxis[dim_I] = candidateIJK[candidate_I][dim_I] > 1 ? True : False;
				decomposeCount += decomposeAxis[dim_I];
			}
			
			if( decomposeCount == highest ) {
				IndexSet_Add( bestFilter, candidate_I );
				bestCount++;
			}
		}
	}
	#ifdef DEBUG
	{
		Index			candidate_I;
		
		Journal_Printf( debug, "Decomposition best decomposition candidates count (phase 4): %u\n", bestCount );
		Journal_Printf( debug, "Decomposition best decomposition candidates (phase 4): { " );
		for( candidate_I = 0; candidate_I < candidateCount; candidate_I++ ) {
			if( IndexSet_IsMember( bestFilter, candidate_I ) ) {
				Journal_Printf( 
					debug, 
					"{ %u, %u, %u } ", 
					candidateIJK[candidate_I][0], 
					candidateIJK[candidate_I][1], 
					candidateIJK[candidate_I][2] );
			}
		}
		Journal_Printf( debug, "}\n" );
	}
	#endif
	
	/* Phase 5a. Find the lowest surface boundary area proxy value. */
	lowest = (unsigned)-1;
	for( candidate_I = 0; candidate_I < candidateCount; candidate_I++ ) {
		if( IndexSet_IsMember( bestFilter, candidate_I ) ) {
			Index count = 
				ecI[0] / 
				candidateIJK[candidate_I][0] + 
				ecI[1] /
				candidateIJK[candidate_I][1] + 
				ecI[2] /
				candidateIJK[candidate_I][2];
			
			if( count < lowest ) {
				lowest = count;
			}
		}
	}
	Journal_Firewall( lowest != (unsigned)-1, error, "Lowest surface area proxy not found... something is wrong!\n" );
	#ifdef DEBUG
		Journal_Printf( debug, "Decomposition lowest surface area proxy value: %u\n", lowest );
	#endif
	
	/* Phase 5b. Choose the best decomposition(s) based on the lowest surface boundary area. */
	for( candidate_I = 0; candidate_I < candidateCount; candidate_I++ ) {
		if( IndexSet_IsMember( bestFilter, candidate_I ) ) {
			Index count = 
				ecI[0] / 
				candidateIJK[candidate_I][0] + 
				ecI[1] /
				candidateIJK[candidate_I][1] + 
				ecI[2] /
				candidateIJK[candidate_I][2];
			
			if( count != lowest ) {
				IndexSet_Remove( bestFilter, candidate_I );
				bestCount--;
			}
		}
	}
	#ifdef DEBUG
	{
		Index			candidate_I;
		
		Journal_Printf( debug, "Decomposition best surface area candidates count (phase 5): %u\n", bestCount );
		Journal_Printf( debug, "Decomposition best surface area candidates (phase 5): { " );
		for( candidate_I = 0; candidate_I < candidateCount; candidate_I++ ) {
			if( IndexSet_IsMember( bestFilter, candidate_I ) ) {
				Journal_Printf( 
					debug, 
					"{ %u, %u, %u } ", 
					candidateIJK[candidate_I][0], 
					candidateIJK[candidate_I][1], 
					candidateIJK[candidate_I][2] );
			}
		}
		Journal_Printf( debug, "}\n" );
	}
	#endif
	
	/* Phase 6: Choose the candidate that provides the most work per cpu */
	bestCandidate_I = 0;
	
	lowestD = ecD[0] * ecD[1] * ecD[2];
	bestCandidate_I = 0;
	for( candidate_I = 0; candidate_I < candidateCount; candidate_I++ ) {
		if( IndexSet_IsMember( bestFilter, candidate_I ) ) {
			double			average;
			Index			dim_I;
			
			average = 0;
			for( dim_I = 0; dim_I < 3; dim_I++ ) {
				average += ecD[dim_I] / (double)candidateIJK[candidate_I][dim_I];
			}
			average /= 3;
			
			if( average < lowestD ) {
				lowestD = average;
				bestCandidate_I = candidate_I;
			}
		}
	}
	Journal_Firewall( 
		lowestD != ecD[0] * ecD[1] * ecD[2], 
		error, 
		"Couldn't choose out of the best... something is wrong!\n" );
	for( dim_I = 0; dim_I < 3; dim_I++) {
		self->partitionedAxis[dim_I] = candidate[bestCandidate_I][dim_I] == 1 ? 0 : 1;
		self->partition3DCounts[dim_I] = candidateIJK[bestCandidate_I][dim_I];
	}
	Journal_Printf( 
		debug, 
		"Out of %u valid decomposition candidates, %u are best surface area candidates. Best work rate is: { %u, %u, %u }\n", 
		filterCount,
		bestCount,
		self->partition3DCounts[0],
		self->partition3DCounts[1],
		self->partition3DCounts[2] );
	
	/* End Steve 20040103 */
	
	/* Decompose each dimension */
	self->procsInUse = 1;
	for( axis = 0; axis < 3; axis++ ) {
		localSizes[axis] = Memory_Alloc_Array( Element_GlobalIndex, self->partition3DCounts[axis],
			"HexaMD ... localSizes" );
		localOffsets[axis] = Memory_Alloc_Array( Element_GlobalIndex, self->partition3DCounts[axis],
			"HexaMD ... localOffsets" );
		
		if( self->partitionedAxis[axis] ) {
			_HexaMD_DecomposeDimension(
				self,
				self->elementGlobal3DCounts[axis],
				self->partition3DCounts[axis],
				localSizes[axis],
				localOffsets[axis] );
		}
		else {
			for( proc_I = 0; proc_I < self->partition3DCounts[axis]; proc_I++ ) {
				localSizes[axis][proc_I] = self->elementGlobal3DCounts[axis];
				localOffsets[axis][proc_I] = 0;
			}
		}
		
		proc_I = 0;
		while( proc_I < self->partition3DCounts[axis] && localSizes[axis][proc_I] )
			proc_I++;
		
		self->partition3DCounts[axis] = proc_I;
		if( self->partition3DCounts[axis] == 1 )
			self->partitionedAxis[axis] = False;
		
		self->procsInUse *= proc_I;
	}
	
	if( self->rank < self->procsInUse ) {
		/* processors being used now need to fill in all counts */
		Partition_Index		procCnt;
		Partition_Index*	procs;
		Bool			isPeriodic[3];
		
		/* Allocate enough memory for everything */
		self->elementLocal3DCounts = Memory_Alloc_Array( Element_LocalIJK, self->procsInUse, "HexaMD" );
		self->_elementOffsets = Memory_Alloc_Array( Element_GlobalIJK, self->procsInUse, "HexaMD" );
		self->nodeLocal3DCounts = Memory_Alloc_Array( Node_LocalIJK, self->procsInUse, "HexaMD" );
		self->_nodeOffsets = Memory_Alloc_Array( Node_GlobalIJK, self->procsInUse, "HexaMD" );
		
		proc_I = 0;
		for( axisIJK[2] = 0; axisIJK[2] < self->partition3DCounts[2]; axisIJK[2]++ ) {
			for( axisIJK[1] = 0; axisIJK[1] < self->partition3DCounts[1]; axisIJK[1]++ ) {
				for( axisIJK[0] = 0; axisIJK[0] < self->partition3DCounts[0]; axisIJK[0]++ ) {

					for ( dim_I=0; dim_I < 3; dim_I++ ) {
						self->elementLocal3DCounts[proc_I][dim_I] = localSizes[dim_I][axisIJK[dim_I]];
						self->_elementOffsets[proc_I][dim_I] = localOffsets[dim_I][axisIJK[dim_I]];
					
						Journal_Firewall( self->elementLocal3DCounts[proc_I][dim_I]
							<= self->elementGlobal3DCounts[dim_I], error,
							"Error - in %s(), proc %d: elementLocal3DCounts[%d]=%d is > than "
							"elementGlobalCounts[%d]=%d for proc %d.\n", __func__,
							self->rank, dim_I, self->elementLocal3DCounts[proc_I][dim_I],
							dim_I, self->elementGlobal3DCounts[dim_I], self->rank ); 
					}	
		
					for ( dim_I=0; dim_I < 3; dim_I++ ) {

						/* Fixed by Patrick Sunter, 15 May 2006: now calculates node counts
						 * correctly for body node layouts */
						self->nodeLocal3DCounts[proc_I][dim_I] = NodeLayout_CalcNodeCountInDimFromElementCount(
							self->nodeLayout, localSizes[dim_I][axisIJK[dim_I]] );

						/* Correct for special case of 2D meshes : node count always 1, not 2 */
						if ( self->nodeGlobal3DCounts[dim_I] == 1 ) {
							self->nodeLocal3DCounts[proc_I][dim_I] = 1;
						}
						self->_nodeOffsets[proc_I][dim_I] = localOffsets[dim_I][axisIJK[dim_I]];

						Journal_Firewall( self->nodeLocal3DCounts[proc_I][dim_I]
							<= self->nodeGlobal3DCounts[dim_I], error,
							"Error - in %s(), proc %d: nodeLocal3DCounts[%d]=%d is > than "
							"nodeGlobalCounts[%d]=%d for proc %d.\n", __func__,
							self->rank, dim_I, self->nodeLocal3DCounts[proc_I][dim_I],
							dim_I, self->nodeGlobal3DCounts[dim_I], self->rank ); 
					}	

					proc_I++;
				}
			}
		}
		
		for( axis = 0; axis < 3; axis++ ) {
			if( localSizes[axis] )
				Memory_Free( localSizes[axis] );
			
			if( localOffsets[axis] )
				Memory_Free( localOffsets[axis] );
		}
		
		isPeriodic[I_AXIS] = Dictionary_GetBool_WithDefault( self->dictionary, "isPeriodicI", False );
		isPeriodic[J_AXIS] = Dictionary_GetBool_WithDefault( self->dictionary, "isPeriodicJ", False );
		isPeriodic[K_AXIS] = Dictionary_GetBool_WithDefault( self->dictionary, "isPeriodicK", False );
		
		/* Construct processor topology */
		self->procTopology = (Topology*)IJK26Topology_New_All( HexaMDIJK26TopologyName, NULL,
			self->partition3DCounts, isPeriodic );
		((IJK26Topology*)self->procTopology)->dynamicSizes = True;
		
		/* Build local element IndexSets */
		self->localElementSets = Memory_Alloc_Array( IndexSet*, self->procsInUse, "HexaMD->localElementSets" );
		for( proc_I = 0; proc_I < self->procsInUse; proc_I++ )
			self->localElementSets[proc_I] = NULL;
		
		if( self->storage == StoreAll ) {
			procCnt = self->procsInUse;
			procs = Memory_Alloc_Array( Partition_Index, procCnt, "HexaMD" );
			for( proc_I = 0; proc_I < self->procsInUse; proc_I++ )
				procs[proc_I] = proc_I;
		}
		else if( self->storage == StoreSelf ) {
			procCnt = 1;
			procs = Memory_Alloc_Array( Partition_Index, procCnt, "HexaMD" );
			procs[0] = self->rank;
		}
		else /*( self->storage == StoreNeighbours )*/ {
			procCnt = self->shadowProcCount( self ) + 1;
			procs = Memory_Alloc_Array( Partition_Index, procCnt, "HexaMD" );
			procs[0] = self->rank;
			self->shadowBuildProcs( self, &procs[1] );
		}
		
		for( proc_I = 0; proc_I < procCnt; proc_I++ ) {
			IJK		max = { 1, 1, 1 };
			Index		sizeI, sizeJ, sizeK;
			Dimension_Index	dim_I;

			for ( dim_I=0; dim_I < 3; dim_I++ ) {
				if ( self->elementLocal3DCounts[procs[proc_I]][dim_I] ) {
					max[dim_I] = self->elementLocal3DCounts[procs[proc_I]][dim_I];
				}	
			}
			
			self->localElementSets[procs[proc_I]] = IndexSet_New( self->elementGlobalCount );
			
			for( sizeK = 0; sizeK < max[2]; sizeK++ ) {
				for( sizeJ = 0; sizeJ < max[1]; sizeJ++ ) {
					for( sizeI = 0; sizeI < max[0]; sizeI++ ) {
						IndexSet_Add( self->localElementSets[procs[proc_I]], HMD_E_3DTo1D( self, 
							self->_elementOffsets[procs[proc_I]][0] + sizeI, 
							self->_elementOffsets[procs[proc_I]][1] + sizeJ, 
							self->_elementOffsets[procs[proc_I]][2] + sizeK ) );
					}
				}
			}
		}
		
		/* Decompose local nodes based on element decomp */
		_MeshDecomp_DecomposeNodes( self );
		
		/* Calculate local totals */
		self->elementLocalCount = IndexSet_UpdateMembersCount( self->localElementSets[self->rank] );
		self->nodeLocalCount = IndexSet_UpdateMembersCount( self->localNodeSets[self->rank] );
		
		/* Setup shadowing information */
		self->shadowDepth = Dictionary_Entry_Value_AsUnsignedInt( Dictionary_GetDefault( self->dictionary, "shadowDepth", 
			Dictionary_Entry_Value_FromUnsignedInt( 0 ) ) );
		
		if( self->shadowDepth && self->procsInUse > 1 ) {
			Processor_Index    lastProcIndex = self->nproc;

			for( proc_I = 0; proc_I < procCnt; proc_I++ ) {
				/* Special case for 2 processors with periodic : lower nbr proc = upper nbr proc */
				if ( procs[proc_I] == lastProcIndex ) continue;
				_HexaMD_BuildShadowSets( self, procs[proc_I] );
				lastProcIndex = procs[proc_I];
			}
		}
		
		if( procs )
			Memory_Free( procs );
			
		/* Calculate totals */
		if( self->shadowElementSets && self->shadowElementSets[self->rank] ) {
			self->elementShadowCount = IndexSet_UpdateMembersCount( self->shadowElementSets[self->rank] );
		}
		else
			self->elementShadowCount = 0;

		self->elementDomainCount = self->elementLocalCount + self->elementShadowCount;
		
		if( self->shadowNodeSets && self->shadowNodeSets[self->rank] ) {
			self->nodeShadowCount = IndexSet_UpdateMembersCount( self->shadowNodeSets[self->rank] );
		}
		else
			self->nodeShadowCount = 0;

		self->nodeDomainCount = self->nodeLocalCount + self->nodeShadowCount;
	}
	else {
		/* processors not being used fill necessary arrays with null values */
		for( axis = 0; axis < 3; axis++ ) {
			if( localSizes[axis] )
				Memory_Free( localSizes[axis] );
			
			if( localOffsets[axis] )
				Memory_Free( localOffsets[axis] );
		}

		/* Clear all values for ranks that are not in use */
		self->partitionedAxis[I_AXIS] = False;
		self->partitionedAxis[J_AXIS] = False;
		self->partitionedAxis[K_AXIS] = False;
		self->procTopology = NULL;
		
		for( axis = 0; axis < 3; axis++ ) {
			self->partition3DCounts[axis] = 0;
			self->elementGlobal3DCounts[axis] = 0;
		}
		
		self->shadowDepth = 0;
	}
	
	Memory_Free( candidateResidual );
	Memory_Free( candidateIJK );
	Memory_Free( candidate );
	Stg_Class_Delete( bestFilter );
	Stg_Class_Delete( filter );

	_HexaMD_CalculateDomain3DCounts( self );
}

/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _HexaMD_Delete( void* hexaMD ) {
	HexaMD* self = (HexaMD*)hexaMD;
	
	if( self->elementLocal3DCounts )
		Memory_Free( self->elementLocal3DCounts );
	
	if( self->_elementOffsets )
		Memory_Free( self->_elementOffsets );
	
	if( self->nodeLocal3DCounts )
		Memory_Free( self->nodeLocal3DCounts );
	
	if( self->_nodeOffsets )
		Memory_Free( self->_nodeOffsets );
	
	/* Stg_Class_Delete parent */
	_MeshDecomp_Delete( self );
}


void _HexaMD_Print( void* hexaMD, Stream* stream ) {
	HexaMD* self = (HexaMD*)hexaMD;
	
	/* Set the Journal for printing informations */
	Stream* hexaMDStream;
	hexaMDStream = Journal_Register( InfoStream_Type, "HexaMDStream" );

	/* Print parent */
	_MeshDecomp_Print( self, stream );
	
	/* General info */
	Journal_Printf( stream, "HexaMD (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* HexaMD info */
}


void* _HexaMD_Copy( void* hexaMD, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	HexaMD*		self = (HexaMD*)hexaMD;
	HexaMD*		newHexaMD;
	PtrMap*		map = ptrMap;
	Bool		ownMap = False;
	Index		idx_I;
	
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newHexaMD = (HexaMD*)_MeshDecomp_Copy( self, dest, deep, nameExt, map );
	
	newHexaMD->numPartitionedDims = self->numPartitionedDims;
	for( idx_I = 0; idx_I < 3; idx_I++ ) {
		newHexaMD->partitionedAxis[idx_I] = self->partitionedAxis[idx_I];
		newHexaMD->partition3DCounts[idx_I] = self->partition3DCounts[idx_I];
		newHexaMD->nodeGlobal3DCounts[idx_I] = self->nodeGlobal3DCounts[idx_I];
		newHexaMD->elementGlobal3DCounts[idx_I] = self->elementGlobal3DCounts[idx_I];
		newHexaMD->elementDomain3DCounts[idx_I] = self->elementDomain3DCounts[idx_I];
	}
	
	if( deep ) {
		if( (newHexaMD->nodeLocal3DCounts = PtrMap_Find( map, self->nodeLocal3DCounts )) == NULL && self->nodeLocal3DCounts ) {
			Partition_Index		proc_I;
			
			newHexaMD->nodeLocal3DCounts = Memory_Alloc_Array( Node_LocalIJK, newHexaMD->procsInUse, "HexaMD->nodeLocal3DCounts" );
			for( proc_I = 0; proc_I < newHexaMD->procsInUse; proc_I++ ) {
				newHexaMD->nodeLocal3DCounts[proc_I][0] = self->nodeLocal3DCounts[proc_I][0];
				newHexaMD->nodeLocal3DCounts[proc_I][1] = self->nodeLocal3DCounts[proc_I][1];
				newHexaMD->nodeLocal3DCounts[proc_I][2] = self->nodeLocal3DCounts[proc_I][2];
			}
		}
		
		if( (newHexaMD->_nodeOffsets = PtrMap_Find( map, self->_nodeOffsets )) == NULL && self->_nodeOffsets ) {
			Partition_Index		proc_I;
			
			newHexaMD->_nodeOffsets = Memory_Alloc_Array( Node_LocalIJK, newHexaMD->procsInUse, "HexaMD->_nodeOffsets" );
			for( proc_I = 0; proc_I < newHexaMD->procsInUse; proc_I++ ) {
				newHexaMD->_nodeOffsets[proc_I][0] = self->_nodeOffsets[proc_I][0];
				newHexaMD->_nodeOffsets[proc_I][1] = self->_nodeOffsets[proc_I][1];
				newHexaMD->_nodeOffsets[proc_I][2] = self->_nodeOffsets[proc_I][2];
			}
		}
		
		if( (newHexaMD->elementLocal3DCounts = PtrMap_Find( map, self->elementLocal3DCounts )) == NULL && self->elementLocal3DCounts ) {
			Partition_Index		proc_I;
			
			newHexaMD->elementLocal3DCounts = Memory_Alloc_Array( Node_LocalIJK, newHexaMD->procsInUse, "HexaMD->elementLocal3DCounts" );
			for( proc_I = 0; proc_I < newHexaMD->procsInUse; proc_I++ ) {
				newHexaMD->elementLocal3DCounts[proc_I][0] = self->elementLocal3DCounts[proc_I][0];
				newHexaMD->elementLocal3DCounts[proc_I][1] = self->elementLocal3DCounts[proc_I][1];
				newHexaMD->elementLocal3DCounts[proc_I][2] = self->elementLocal3DCounts[proc_I][2];
			}
		}
		
		if( (newHexaMD->_elementOffsets = PtrMap_Find( map, self->_elementOffsets )) == NULL && self->_elementOffsets ) {
			Partition_Index		proc_I;
			
			newHexaMD->_elementOffsets = Memory_Alloc_Array( Node_LocalIJK, newHexaMD->procsInUse, "HexaMD->_elementOffsets" );
			for( proc_I = 0; proc_I < newHexaMD->procsInUse; proc_I++ ) {
				newHexaMD->_elementOffsets[proc_I][0] = self->_elementOffsets[proc_I][0];
				newHexaMD->_elementOffsets[proc_I][1] = self->_elementOffsets[proc_I][1];
				newHexaMD->_elementOffsets[proc_I][2] = self->_elementOffsets[proc_I][2];
			}
		}
	}
	else {
		newHexaMD->nodeLocal3DCounts = self->nodeLocal3DCounts;;
		newHexaMD->_nodeOffsets = self->_nodeOffsets;
		newHexaMD->elementLocal3DCounts = self->elementLocal3DCounts;
		newHexaMD->_elementOffsets = self->_elementOffsets;
	}
	
	if( ownMap ) {
		Stg_Class_Delete( map );
	}
	
	return (void*)newHexaMD;
}

void _HexaMD_Construct( void* hexaMD, Stg_ComponentFactory *cf, void* data ) {
	HexaMD *        self            = (HexaMD*)hexaMD;
	ElementLayout*  elementLayout   = NULL;
	NodeLayout*     nodeLayout      = NULL;
	Stg_ObjectList* pointerRegister;
	Index           numPartitionedDims = 1;

	self->dictionary = cf->rootDict;
	
	elementLayout =  Stg_ComponentFactory_ConstructByKey(  cf,  self->name,  ElementLayout_Type, ElementLayout, True, data ) ;
	nodeLayout =  Stg_ComponentFactory_ConstructByKey(  cf,  self->name,  NodeLayout_Type, NodeLayout,  True, data ) ;
	numPartitionedDims = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, "numPartitionedDims", 1 ); 
	
	pointerRegister = Stg_ObjectList_Get( cf->registerRegister, "Pointer_Register" );
	_MeshDecomp_Init( (MeshDecomp*)self, MPI_COMM_WORLD, elementLayout, nodeLayout, pointerRegister );
	_HexaMD_Init( self, numPartitionedDims );
}
	
void _HexaMD_Build( void* hexaMD, void* data ){
	HexaMD *        self            = (HexaMD*)hexaMD;

	/* TODO _MeshDecomp_Build( self, data ); */
	Build( self->elementLayout, self /* TODO - WHY IS ELEMENT LAYOUT EXPECTING DECOMP */, False );
	Build( self->nodeLayout, data, False );
}
	
void _HexaMD_Initialise( void* hexaMD, void* data ){
	
}
	
void _HexaMD_Execute( void* hexaMD, void* data ){
	
}

Partition_Index _HexaMD_Shadow_ProcCount( void* hexaMD ) {
	HexaMD* self = (HexaMD*)hexaMD;
	
	return self->procTopology->neighbourCount( self->procTopology, self->rank );
}


void _HexaMD_Shadow_BuildProcs( void* hexaMD, Partition_Index* procs ) {
	HexaMD* self = (HexaMD*)hexaMD;
	
	self->procTopology->buildNeighbours( self->procTopology, self->rank, procs );
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/


/* Find boundary elements */
void _HexaMD_GetBoundarySet(
		HexaMD*            self,
		Processor_Index    proc_I,
		IndexSet*          boundaryElementSet )
{
	IJK               min, max, ijk;
	Dimension_Index   boundaryAxis, otherAxisA, otherAxisB, dim_I;

	for ( dim_I = 0; dim_I < 3; dim_I++ ) {
		min[dim_I] = self->_elementOffsets[proc_I][dim_I];
		max[dim_I] = self->_elementOffsets[proc_I][dim_I] + self->elementLocal3DCounts[proc_I][dim_I] - 1;
	}
	
	/* Iterate through all the boundary elements, and add them to the element set. Have to go through the mix and
	   max boundary for each axis, then add every element on that surface */ 
	for ( boundaryAxis = I_AXIS; boundaryAxis < 3; boundaryAxis++ ) {	
		otherAxisA = (boundaryAxis+1) % 3;
		otherAxisB = (boundaryAxis+2) % 3;
		
		ijk[boundaryAxis] = min[boundaryAxis];
		while ( ijk[boundaryAxis] <= max[boundaryAxis] ) {
			for ( ijk[otherAxisA] = min[otherAxisA]; ijk[otherAxisA] <= max[otherAxisA]; ijk[otherAxisA]++ )
			{
				for ( ijk[otherAxisB] = min[otherAxisB]; ijk[otherAxisB] <= max[otherAxisB]; ijk[otherAxisB]++ )
				{
					IndexSet_Add( boundaryElementSet, ijk[2] * self->elementGlobal3DCounts[0] * 
						self->elementGlobal3DCounts[1] + ijk[1] * 
						self->elementGlobal3DCounts[0] + ijk[0] );
				}
			}	
				
			/* Necessary for 2D runs, where num elements in a dimension may be zero,
			   or for thin decomps where the slice in a dim is 1 wide, therefore min = max */
			if ( min[boundaryAxis] == max[boundaryAxis] ) break;

			ijk[boundaryAxis] += self->elementLocal3DCounts[proc_I][boundaryAxis] - 1;
		}
	}
}


void _HexaMD_BuildShadowSets( HexaMD* self, Processor_Index procIndexToBuildFor ) {
	Dimension_Index   shadowAxis;
	int               lowerStartShadowRegions[3]; 
	int               upperStartShadowRegions[3]; 
	unsigned int      lowerAdjustedShadowDepth;
	unsigned int      upperAdjustedShadowDepth;
	unsigned int      shadowRegionTotalWidth[3];
	Stream*           debug = Journal_Register( DebugStream_Type, HexaMD_Type );
	Index             shadowElementsFoundCount = 0;
	Index             shadowNodesFoundCount = 0;
	IJK               currProcMin, currProcMax;
	Dimension_Index   dim_I;
	Stream*           error = Journal_Register( Error_Type, self->type );
	Processor_Index   proc_I = 0;

	Journal_DPrintfL( debug, 1, "In %s(): calculating shadow elements & nodes for processor %d, shadow depth=%d:\n",
		__func__, procIndexToBuildFor, self->shadowDepth );
	Stream_Indent( debug );	

	Journal_Firewall( self->shadowDepth > 0, error, "Error: %s() called when shadow depth set to 0.\n",
		__func__ );

	Journal_Firewall( self->nproc > 1, error, "Error: %s() called when number of processors is 1.\n",
		__func__ );
	
	/* If the shadow set arrays have not yet been allocated, do it now */
	if( !self->shadowElementSets ) {
		self->shadowElementSets = Memory_Alloc_Array( IndexSet*, self->procsInUse, "MeshDecomp->shadowElementSets" );
		for( proc_I = 0; proc_I < self->procsInUse; proc_I++ )
			self->shadowElementSets[proc_I] = NULL;
	}
	
	if( !self->shadowNodeSets ) {
		self->shadowNodeSets = Memory_Alloc_Array( IndexSet*, self->procsInUse, "MeshDecomp->shadowNodeSets" );
		for( proc_I = 0; proc_I < self->procsInUse; proc_I++ )
			self->shadowNodeSets[proc_I] = NULL;
	}
	
	/* If the IndexSets for this proc have not been constructed, do it now */
	if( !self->shadowElementSets[procIndexToBuildFor] )
		self->shadowElementSets[procIndexToBuildFor] = IndexSet_New( self->elementGlobalCount );
	
	if( !self->shadowNodeSets[procIndexToBuildFor] )
		self->shadowNodeSets[procIndexToBuildFor] = IndexSet_New( self->nodeGlobalCount );

	Journal_DPrintfL( debug, 2, "calculating shadow elements:\n", shadowAxis );
	for ( dim_I = 0; dim_I < 3; dim_I++ ) {
		currProcMin[dim_I] = self->_elementOffsets[procIndexToBuildFor][dim_I];
		currProcMax[dim_I] = self->_elementOffsets[procIndexToBuildFor][dim_I] +
			self->elementLocal3DCounts[procIndexToBuildFor][dim_I] - 1;
	}
	Stream_Indent( debug );
	shadowElementsFoundCount = 0;
	for ( shadowAxis = I_AXIS; shadowAxis < 3; shadowAxis++ ) {	

		lowerStartShadowRegions[shadowAxis] = currProcMin[shadowAxis];
		upperStartShadowRegions[shadowAxis] = currProcMin[shadowAxis];
		shadowRegionTotalWidth[shadowAxis] = 0;

		if ( False == self->partitionedAxis[shadowAxis] ) continue;
	
		/* Calculate shadow elements on the lower end of the partitioned axis */
		Journal_DPrintfL( debug, 2, "calculating lower shadow elements for Axis %d:\n", shadowAxis );
		_HexaMD_CalculateLowerShadowStartAndEnd( self, shadowAxis, ELEMENT_ITEM_TYPE,
			currProcMin[shadowAxis], self->elementGlobal3DCounts[shadowAxis],
			&lowerStartShadowRegions[shadowAxis], &lowerAdjustedShadowDepth );

		shadowElementsFoundCount += _HexaMD_AddItemsInSliceToSet( self, shadowAxis,
			lowerStartShadowRegions[shadowAxis], lowerAdjustedShadowDepth,
			currProcMin, currProcMax, self->elementGlobal3DCounts,
			self->shadowElementSets[procIndexToBuildFor] );

		/* Calculate shadow elements on the upper end of the partitioned axis */
		Journal_DPrintfL( debug, 2, "calculating upper shadow elements for Axis %d:\n", shadowAxis );
		_HexaMD_CalculateUpperShadowStartAndEnd( self, shadowAxis, ELEMENT_ITEM_TYPE,
			currProcMax[shadowAxis], self->elementGlobal3DCounts[shadowAxis],
			&upperStartShadowRegions[shadowAxis], &upperAdjustedShadowDepth );

		shadowElementsFoundCount += _HexaMD_AddItemsInSliceToSet( self, shadowAxis,
			upperStartShadowRegions[shadowAxis], upperAdjustedShadowDepth,
			currProcMin, currProcMax, self->elementGlobal3DCounts,
			self->shadowElementSets[procIndexToBuildFor] );

		shadowRegionTotalWidth[shadowAxis] = lowerAdjustedShadowDepth + upperAdjustedShadowDepth;  
	}	

	if ( self->numPartitionedDims > 1 ) {
		/* now add the corner elements in case this is a 2D decomp */
		Journal_DPrintfL( debug, 2, "calculating corner shadow elements:\n" );
		shadowElementsFoundCount += _HexaMD_AddItemsInBlockCornersOutsideInnerBlockToSet( 
			self, lowerStartShadowRegions, shadowRegionTotalWidth,
			currProcMin, currProcMax, self->elementGlobal3DCounts,
			self->shadowElementSets[procIndexToBuildFor] );
	}

	Stream_UnIndent( debug );

	Journal_DPrintfL( debug, 1, "%s(): calculated there were %d shadow elements for processor %d:\n",
		__func__, shadowElementsFoundCount, procIndexToBuildFor );

	
	Journal_DPrintfL( debug, 2, "calculating shadow nodes:\n", shadowAxis );
	for ( dim_I = 0; dim_I < 3; dim_I++ ) {
		currProcMin[dim_I] = self->_nodeOffsets[procIndexToBuildFor][dim_I];
		currProcMax[dim_I] = self->_nodeOffsets[procIndexToBuildFor][dim_I] +
			self->nodeLocal3DCounts[procIndexToBuildFor][dim_I] - 1;
	}
	Stream_Indent( debug );
	shadowNodesFoundCount = 0;
	for ( shadowAxis = I_AXIS; shadowAxis < 3; shadowAxis++ ) {	

		if ( False == self->partitionedAxis[shadowAxis] ) continue;
	
		/* Calculate shadow nodes on the lower end of the partitioned axis */
		Journal_DPrintfL( debug, 2, "calculating lower shadow nodes in Axis %d:\n", shadowAxis );
		_HexaMD_CalculateLowerShadowStartAndEnd( self, shadowAxis, NODE_ITEM_TYPE,
			currProcMin[shadowAxis], self->nodeGlobal3DCounts[shadowAxis],
			&lowerStartShadowRegions[shadowAxis], &lowerAdjustedShadowDepth );

		shadowNodesFoundCount += _HexaMD_AddItemsInSliceToSet( self, shadowAxis,
			lowerStartShadowRegions[shadowAxis], lowerAdjustedShadowDepth, currProcMin, currProcMax,
			self->nodeGlobal3DCounts, self->shadowNodeSets[procIndexToBuildFor] );
		
		/* Calculate shadow nodes on the upper end of the partitioned axis */
		Journal_DPrintfL( debug, 2, "calculating upper shadow nodes in Axis %d:\n", shadowAxis );
		_HexaMD_CalculateUpperShadowStartAndEnd( self, shadowAxis, NODE_ITEM_TYPE,
			currProcMax[shadowAxis], self->nodeGlobal3DCounts[shadowAxis],
			&upperStartShadowRegions[shadowAxis], &upperAdjustedShadowDepth );

		shadowNodesFoundCount += _HexaMD_AddItemsInSliceToSet( self, shadowAxis,
			upperStartShadowRegions[shadowAxis], upperAdjustedShadowDepth, currProcMin, currProcMax,
			self->nodeGlobal3DCounts, self->shadowNodeSets[procIndexToBuildFor] );

		shadowRegionTotalWidth[shadowAxis] = lowerAdjustedShadowDepth + upperAdjustedShadowDepth;  
	}	

	if ( self->numPartitionedDims > 1 ) {
		/* now add the corner nodes in case this is a 2D decomp */
		Journal_DPrintfL( debug, 2, "calculating corner shadow nodes:\n" );
		shadowNodesFoundCount += _HexaMD_AddItemsInBlockCornersOutsideInnerBlockToSet( 
			self, lowerStartShadowRegions, shadowRegionTotalWidth,
			currProcMin, currProcMax, self->nodeGlobal3DCounts,
			self->shadowNodeSets[procIndexToBuildFor] );
	}

	Stream_UnIndent( debug );

	Journal_DPrintfL( debug, 1, "%s(): calculated there were %d shadow nodes for processor %d:\n",
		__func__, shadowNodesFoundCount, procIndexToBuildFor );

	Stream_UnIndent( debug );	
}


/* Selecting start and end of lower shadow region, taking into account periodicity */
void _HexaMD_CalculateLowerShadowStartAndEnd(
		HexaMD*         self,
		Dimension_Index shadowAxis,
		MeshItemType    itemType,
		GlobalIndex     currProcMinInShadowAxis,
		GlobalIndex     globalCountInShadowAxis,            
		int*            startIndexInShadowAxis,
		Index*          adjustedShadowDepth )
{	
	Stream*           debug = Journal_Register( DebugStream_Type, HexaMD_Type );

	/* Can safely refer to elementLayout's periodicity since both nodes & elements must be the same periodicity */
	if ( False == ((IJKTopology*)self->elementLayout->topology)->isPeriodic[shadowAxis] ) {
		/* If not periodic, we have to truncate shadow depth if it crosses the global zero element */
		(*startIndexInShadowAxis) = currProcMinInShadowAxis - self->shadowDepth;
		if ( (*startIndexInShadowAxis) < 0 ) {
			(*startIndexInShadowAxis) = 0;
		}
		
		(*adjustedShadowDepth) = currProcMinInShadowAxis - (*startIndexInShadowAxis);
		Journal_DPrintfL( debug, 2, "Not periodic -> start at index %d, go for %d\n",
			(*startIndexInShadowAxis), (*adjustedShadowDepth) );
	}
	else {	/* Periodic elements in decomp dim */
		/* When periodic, wrap around the start to the other end */
		(*startIndexInShadowAxis) = currProcMinInShadowAxis - self->shadowDepth;
		(*adjustedShadowDepth) = self->shadowDepth;

		if ( (*startIndexInShadowAxis) < 0 ) {

			(*startIndexInShadowAxis) += globalCountInShadowAxis;

			if ( (itemType == NODE_ITEM_TYPE) && (self->nodeLayout->type == CornerNL_Type ) ) {
				/* If we are going to hit a period boundary, and are on nodes, need to add one to depth */
				(*startIndexInShadowAxis) -= 1;
				(*adjustedShadowDepth) += 1;
			}	

			// TODO
			/* Need to check if shadow depth is so high that we've wrapped around into
			   our own element/node space */
			/*   
			if ( (*startIndexInShadowAxis) <= currProcMaxInShadowAxis ) {
				(*startIndexIn 
				// TODO: print warning
			}
			*/
		}

		Journal_DPrintfL( debug, 2, "periodic -> start at index %d, go for %d\n", 
			(*startIndexInShadowAxis), (*adjustedShadowDepth) );
	}
}	


/* Selecting start and end of upper shadow region, taking into account periodicity */
void _HexaMD_CalculateUpperShadowStartAndEnd(
		HexaMD*         self,
		Dimension_Index shadowAxis,
		MeshItemType    itemType,
		GlobalIndex     currProcMaxInShadowAxis,
		GlobalIndex     globalCountInShadowAxis,            
		int*            startIndexInShadowAxis,
		Index*          adjustedShadowDepth )
{	
	Stream*           debug = Journal_Register( DebugStream_Type, HexaMD_Type );


	/* Selecting start and end of upper shadow region, taking into account periodicity */
	if ( False == ((IJKTopology*)self->elementLayout->topology)->isPeriodic[shadowAxis] ) {
		/* If not periodic, we have to truncate shadow depth if it crosses the global max element */
		(*startIndexInShadowAxis) = currProcMaxInShadowAxis + 1;

		(*adjustedShadowDepth) = self->shadowDepth;
		if ( (currProcMaxInShadowAxis + self->shadowDepth) >= globalCountInShadowAxis ) {
			/* truncate */
			(*adjustedShadowDepth) = globalCountInShadowAxis - (*startIndexInShadowAxis);
		}
		
		Journal_DPrintfL( debug, 2, "Not periodic -> start at index %d, go for %d\n",
			(*startIndexInShadowAxis), (*adjustedShadowDepth) );
	}
	else {	/* Periodic elements in decomp dim */
		(*startIndexInShadowAxis) = currProcMaxInShadowAxis + 1;
		/* In case this is the last processor: need to wrap the start */
		(*startIndexInShadowAxis) = (*startIndexInShadowAxis) % (int)globalCountInShadowAxis;

		(*adjustedShadowDepth) = self->shadowDepth;
		if ( (currProcMaxInShadowAxis + self->shadowDepth) >= globalCountInShadowAxis ) {
			/* For periodic corner nodes, need to add an extra one to get the right element shadow depth */
			if ( (itemType == NODE_ITEM_TYPE) && (self->nodeLayout->type == CornerNL_Type ) ) {
				(*adjustedShadowDepth) += 1;
			}
		}

		Journal_DPrintfL( debug, 2, "periodic -> start at index %d, go for %d\n", 
			(*startIndexInShadowAxis), (*adjustedShadowDepth) );
	}
}


GlobalIndex _HexaMD_AddItemsInBlockCornersOutsideInnerBlockToSet( 
		HexaMD*         self,
		IJK             outerBlockMin,
		IJK             outerBlockCounts,
		IJK             innerBlockMin,
		IJK             innerBlockMax,
		IJK             itemGlobal3DCounts,   
		IndexSet*       itemSet )
{	
	IJK                  ijk;
	Index                block_I[3];
	Index                itemsFoundInCornersCount = 0;
	Stream*              debug = Journal_Register( DebugStream_Type, HexaMD_Type );
	Dimension_Index      dim_I, axisA, axisB, internalAxis;
	GlobalIndex          gItem_I;

	Journal_DPrintfL( debug, 3, "In %s(): Filling in corner element/nodes around block (%d,%d,%d) - (%d,%d,%d)\n",
		__func__, innerBlockMin[0], innerBlockMin[1], innerBlockMin[2],
		innerBlockMax[0], innerBlockMax[1], innerBlockMax[2] );

	Stream_Indent( debug );

	/* We have to deal with the potentially 8 blocks of 2D corners where one axis is still internal to the 
	   inner block first */
	for ( dim_I = 0; dim_I < 3; dim_I++ ) {
		axisA = dim_I;
		axisB = (dim_I+1) % 3;
		internalAxis = (dim_I+2) % 3;

		Journal_DPrintfL( debug, 3, "Filling 2D side corners items in the %c,%c planes around block\n",
			IJKTopology_DimNumToDimLetter[axisA], IJKTopology_DimNumToDimLetter[axisB] );

		memcpy( ijk, outerBlockMin, sizeof(IJK) );

		Stream_Indent( debug );
		for ( ijk[internalAxis] = innerBlockMin[internalAxis]; ijk[internalAxis] <= innerBlockMax[internalAxis]; ijk[internalAxis]++ ) {
			ijk[axisB] = outerBlockMin[axisB];

			for ( block_I[axisB] = 0; block_I[axisB] < outerBlockCounts[axisB]; block_I[axisB]++ ) {
				if ( ijk[axisB] == innerBlockMin[axisB] ) {
					/* Fast-forward to end of inner block */
					ijk[axisB] = innerBlockMax[axisB] + 1;
				}
				ijk[axisB] = ijk[axisB] % itemGlobal3DCounts[axisB];

				ijk[axisA] = outerBlockMin[axisA];
				
				for ( block_I[axisA] = 0; block_I[axisA] < outerBlockCounts[axisA]; block_I[axisA]++ ) {
					if ( ijk[axisA] == innerBlockMin[axisA] ) {
						/* Fast-forward to end of inner block */
						ijk[axisA] = innerBlockMax[axisA] + 1;
					}
					ijk[axisA] = ijk[axisA] % itemGlobal3DCounts[axisA];

					/* Convert ijk to global item, and add */
					gItem_I = ijk[2] * itemGlobal3DCounts[0] * itemGlobal3DCounts[1]
						+ ijk[1] * itemGlobal3DCounts[0]
						+ ijk[0];
					Journal_DPrintfL( debug, 3, "Adding 2D corner element/node at global index (%d,%d,%d) -> %d\n",
						ijk[0], ijk[1], ijk[2], gItem_I );
					IndexSet_Add( itemSet, gItem_I );
					itemsFoundInCornersCount++;	

					ijk[axisA]++;
				}	
				ijk[axisB]++;
			}
		}
		Stream_UnIndent( debug );
	}	

	Journal_DPrintfL( debug, 3, "Filling 3D corners items\n" );

	Stream_Indent( debug );
	memcpy( ijk, outerBlockMin, sizeof(IJK) );
	for ( block_I[K_AXIS] = 0; block_I[K_AXIS] < outerBlockCounts[K_AXIS]; block_I[K_AXIS]++ ) {
		if ( ijk[K_AXIS] == innerBlockMin[K_AXIS] ) {
			/* Fast-forward to end of inner block */
			ijk[K_AXIS] = innerBlockMax[K_AXIS] + 1;
		}
		ijk[K_AXIS] = ijk[K_AXIS] % itemGlobal3DCounts[K_AXIS];

		ijk[J_AXIS] = outerBlockMin[J_AXIS];

		for ( block_I[J_AXIS] = 0; block_I[J_AXIS] < outerBlockCounts[J_AXIS]; block_I[J_AXIS]++ ) {
			if ( ijk[J_AXIS] == innerBlockMin[J_AXIS] ) {
				/* Fast-forward to end of inner block */
				ijk[J_AXIS] = innerBlockMax[J_AXIS] + 1;
			}
			ijk[J_AXIS] = ijk[J_AXIS] % itemGlobal3DCounts[J_AXIS];

			ijk[I_AXIS] = outerBlockMin[I_AXIS];
		
			for ( block_I[I_AXIS] = 0; block_I[I_AXIS] < outerBlockCounts[I_AXIS]; block_I[I_AXIS]++ ) {
				if ( ijk[I_AXIS] == innerBlockMin[I_AXIS] ) {
					/* Fast-forward to end of inner block */
					ijk[I_AXIS] = innerBlockMax[I_AXIS] + 1;
				}
				ijk[I_AXIS] = ijk[I_AXIS] % itemGlobal3DCounts[I_AXIS];

				/* Convert ijk to global item, and add */
				gItem_I = ijk[2] * itemGlobal3DCounts[0] * itemGlobal3DCounts[1]
					+ ijk[1] * itemGlobal3DCounts[0]
					+ ijk[0];
				Journal_DPrintfL( debug, 3, "Adding 3D corner element/node at global index (%d,%d,%d) -> %d\n",
					ijk[0], ijk[1], ijk[2], gItem_I );
				IndexSet_Add( itemSet, gItem_I );
				itemsFoundInCornersCount++;	
				ijk[I_AXIS]++;
			}
			ijk[J_AXIS]++;
		}

		ijk[K_AXIS]++;
	}
	Stream_UnIndent( debug );
	Stream_UnIndent( debug );

	return itemsFoundInCornersCount;
}


GlobalIndex  _HexaMD_AddItemsInSliceToSet(
		HexaMD*         self,
		Dimension_Index sliceAxis,
		Index           startOfRange,
		Index           widthOfSlice,
		IJK             otherAxisMinimums,
		IJK             otherAxisMaximums,
		IJK             itemGlobal3DCounts,   
		IndexSet*       itemSet )
{	
	Stream*              debug = Journal_Register( DebugStream_Type, HexaMD_Type );
	Index                sliceWidth_I;
	IJK                  ijk;
	Dimension_Index      otherAxisA, otherAxisB;
	GlobalIndex          gItem_I;
	Index                itemsFoundInSliceCount = 0;

	otherAxisA = (sliceAxis+1) % 3;
	otherAxisB = (sliceAxis+2) % 3;
		
	ijk[sliceAxis] = startOfRange;

	Stream_Indent( debug );
	for ( sliceWidth_I=0; sliceWidth_I < widthOfSlice; sliceWidth_I++ ) {
		/* to handle when periodic, check if we have flipped from upper->lower end */
		ijk[sliceAxis] = ijk[sliceAxis] % itemGlobal3DCounts[sliceAxis];
		
		for ( ijk[otherAxisB] = otherAxisMinimums[otherAxisB]; ijk[otherAxisB] <= otherAxisMaximums[otherAxisB]; ijk[otherAxisB]++ )
		{
			for ( ijk[otherAxisA] = otherAxisMinimums[otherAxisA]; ijk[otherAxisA] <= otherAxisMaximums[otherAxisA]; ijk[otherAxisA]++ )
			{
				gItem_I = ijk[2] * itemGlobal3DCounts[0] * itemGlobal3DCounts[1]
					+ ijk[1] * itemGlobal3DCounts[0]
					+ ijk[0];
				Journal_DPrintfL( debug, 3, "Adding element/node at global index (%d,%d,%d) -> %d\n",
					ijk[0], ijk[1], ijk[2], gItem_I );
				IndexSet_Add( itemSet, gItem_I );
				itemsFoundInSliceCount++;	
			}
		}	
		(ijk[sliceAxis])++;
	}
	Stream_UnIndent( debug );

	return itemsFoundInSliceCount;
}


void _HexaMD_DecomposeDimension(
		HexaMD*				self,
		Element_GlobalIndex		numElementsThisDim,
		Partition_Index			procCount,
		Element_LocalIndex*		localElementCounts,
		Element_LocalIndex*		localElementOffsets )
{
	Partition_Index		procCountToUse = procCount;
	Partition_Index		proc_I;
	Partition_Index		div;
	Partition_Index		rem;
	Index				offsetIncrement;
	
	/* We need at least one processor */
	assert( procCount );
	
	/* Clear values to zero */
	for( proc_I = 0; proc_I < procCount; proc_I++ ) {
		localElementCounts[proc_I] = 0;
		localElementOffsets[proc_I] = 0;
	}
	
	if( self->allowUnbalancing ) {
		/* prefer partition on node */
		if( self->allowPartitionOnNode ) {
			
			div = numElementsThisDim / procCount;
			rem = numElementsThisDim % procCount;
			if( !div ) {
				assert( self->allowUnusedCPUs );
				procCountToUse = rem;
			}
		}
		else if( self->allowPartitionOnElement ) {
			Element_GlobalIndex     ae;
			
			procCountToUse = (numElementsThisDim + 1) / 2;
			if( procCountToUse > procCount ) {
				procCountToUse = procCount;
			}	
			else if( procCountToUse < procCount )
				assert( self->allowUnusedCPUs );
			
			ae = numElementsThisDim - procCountToUse + 1;
			
			div = ae / procCountToUse;
			rem = ae % procCountToUse;
			
			for( proc_I = 1; proc_I < procCount; proc_I++ )
				localElementOffsets[proc_I] = proc_I;
		}
	}
	else {
		/* Ok, so unbalanced numbers of elements aren't allowed.
		Go through trying to find an even division partitioned on nodes or
		elements, progressively reducing the procs to use if allowed to. */
		do {
			if( self->allowPartitionOnNode ) {
				rem = numElementsThisDim % procCountToUse;
				if( 0 == rem ) {
					div = numElementsThisDim / procCountToUse;
					break;
				}
			}
			
			if( self->allowPartitionOnElement &&
				(procCountToUse <= ((numElementsThisDim + 1) / 2)) ) {
				Element_GlobalIndex     ae = numElementsThisDim - procCountToUse + 1;
				
				rem = ae % procCountToUse;
				if( 0 == rem ) {
					div = ae / procCountToUse;
					
					for( proc_I = 1; proc_I < procCount; proc_I++ )
						localElementOffsets[proc_I] = proc_I;
					
					break;
				}
			}
			else {
				assert( self->allowUnusedCPUs );
				--procCountToUse;
				assert( procCountToUse );
			}
		}
		while( True );
	}
	
	/* Now we work out the initial local sizes and offsets */
	for( proc_I = 0; proc_I < procCountToUse; proc_I++ ) {
		localElementCounts[proc_I] = div;
		localElementOffsets[proc_I] += proc_I * div;
	}
	
	/* If there was a remainder, we now need to update the sizes and offsets */
	if ( rem != 0 ) {
		/* only the first 'rem' processors have their size increased to take up
		the remainder */
		for( proc_I = 0; proc_I < rem; proc_I++ )
			localElementCounts[proc_I]++;

		/* Now we update the offsets - this is a little tricky ... each time
		a size was increased, we need to increase the _offset to add_ to the
		existing offset of every subsequent processor by 1 */
		offsetIncrement = 1;
		for( proc_I = 1; proc_I < procCountToUse; proc_I++ ) {
			localElementOffsets[proc_I] += offsetIncrement;
			
			if (offsetIncrement < rem ) {
				offsetIncrement++;
			}	
		
		}	
	}
}


void _HexaMD_CalculateDomain3DCounts( HexaMD* self )
{
	Dimension_Index		dim_I = 0;
	Processor_Index		myRank = self->rank;

	if ( myRank < self->procsInUse ) {	
		for ( dim_I = 0; dim_I < 3; dim_I++ ) {
			self->elementDomain3DCounts[dim_I] = self->elementLocal3DCounts[myRank][dim_I];
		}

		if ( self->elementShadowCount > 0 ) {
			for ( dim_I = 0; dim_I < 3; dim_I++ ) {
				if ( self->_elementOffsets[myRank][dim_I] != 0
					|| (((IJKTopology*)self->procTopology)->isPeriodic[dim_I] ) )
				{	
					self->elementDomain3DCounts[dim_I] += self->shadowDepth;
				}
				if ( self->_elementOffsets[myRank][dim_I] + self->elementLocal3DCounts[myRank][dim_I] != self->elementGlobal3DCounts[dim_I]
					|| (((IJKTopology*)self->procTopology)->isPeriodic[dim_I] ) )
				{	
					self->elementDomain3DCounts[dim_I] += self->shadowDepth;
				}	
			}
		}
	}	
	else {
		for ( dim_I = 0; dim_I < 3; dim_I++ ) {
			self->elementDomain3DCounts[dim_I] = 0;
		}	
	}		
}


void _HexaMD_Destroy( HexaMD* self, void *data ) {
	if( self->elementLocal3DCounts )
		Memory_Free( self->elementLocal3DCounts );
	
	if( self->_elementOffsets )
		Memory_Free( self->_elementOffsets );
	
	if( self->nodeLocal3DCounts )
		Memory_Free( self->nodeLocal3DCounts );
	
	if( self->_nodeOffsets )
		Memory_Free( self->_nodeOffsets );
}
