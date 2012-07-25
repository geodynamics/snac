/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003,
**	Steve Quenette, 110 Victoria Street, Melbourne, Victoria, 3053, Australia.
**	Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
**	University of Texas, 1 University Station, Austin, Texas, 78712, USA.
**
** Authors:
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**	Stevan M. Quenette, Visitor in Geophysics, Caltech.
**	Luc Lavier, Research Scientist, The University of Texas. (luc@utig.ug.utexas.edu)
**	Luc Lavier, Research Scientist, Caltech.
**
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2, or (at your option) any
** later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**
** $Id: ConstructExtensions.c 3066 2005-07-06 22:17:29Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Mesh.h"
#include "Context.h"
#include "Register.h"
#include "TestCondFunc.h"
#include "ConstructExtensions.h"
#include <assert.h>
#include <limits.h>
#include <string.h>

static char CONDITION_STR[] = "remeshCondition";
static char TIMESTEPCRITERION_STR[] = "remeshTimeStepCriterion";
static char LENGTHCRITERION_STR[] = "remeshLengthCriterion";
static char OFF_STR[] = "off";
static char ON_STR[] = "on";
static char ONTIMESTEP_STR[] = "onTimeStep";
static char ONMINLENGTHSCALE_STR[] = "onMinLengthScale";
static char ONBOTHTIMESTEPLENGTH_STR[] = "onBothTimeStepLength";
static char MESH_STR[] = "mesh";
static char MESHTYPE_STR[] = "meshType";
static char SPHERICAL_STR[] = "spherical";
static char CARTESIAN_STR[] = "cartesian";
#ifndef PATH_MAX
	#define PATH_MAX 1024
#endif

void _SnacRemesher_ConstructExtensions( void* _context, void* data ) {
	Snac_Context*				context = (Snac_Context*)_context;
	SnacRemesher_Context*			contextExt = ExtensionManager_Get(
							context->extensionMgr,
							context,
							SnacRemesher_ContextHandle );
	Mesh*					mesh = context->mesh;
	SnacRemesher_Mesh*			meshExt = ExtensionManager_Get(
							context->meshExtensionMgr,
							mesh,
							SnacRemesher_MeshHandle );
	char*					conditionStr;
	Dictionary_Entry_Value*			conditionCriterion;
	Dictionary* 				meshDict;
	Stream*					error = Journal_Register( Error_Type, "Remesher" );
	char					tmpBuf[PATH_MAX];

	Journal_Printf( context->debug, "In: %s\n", __func__ );
	
	contextExt->debugIC = Journal_Register( Debug_Type, "Remesher-ICs" );
	contextExt->debugCoords = Journal_Register( Debug_Type, "Remesher-Coords" );
	contextExt->debugNodes = Journal_Register( Debug_Type, "Remesher-Nodes" );
	contextExt->debugElements = Journal_Register( Debug_Type, "Remesher-Elements" );
	contextExt->debugSync = Journal_Register( Debug_Type, "Remesher-Sync" );
	
	/* Additional tables required over the nodeElementTbl already required by core Snac */
	Mesh_ActivateNodeNeighbourTbl( mesh );
	Mesh_ActivateElementNeighbourTbl( mesh );
	
	/* Work out condition to remesh on */
	if( !Dictionary_Get( context->dictionary, CONDITION_STR ) ) {
		Journal_Printf( 
			error,
			"Warning: No \"%s\" entry in dictionary... will default to \"%s\"\n", 
			CONDITION_STR,
			OFF_STR );
	}
	
	conditionStr = Dictionary_Entry_Value_AsString( 
		Dictionary_GetDefault( context->dictionary, CONDITION_STR, Dictionary_Entry_Value_FromString( OFF_STR ) ) );
	
	contextExt->OnTimeStep = 0;
	contextExt->onMinLengthScale = 0;
	if( !strcmp( conditionStr, OFF_STR ) ) {
		contextExt->condition = SnacRemesher_Off;
		Journal_Printf( context->snacInfo, "Remesher is off\n" );
	}
	else if( !strcmp( conditionStr, ONTIMESTEP_STR ) ) {
		contextExt->condition = SnacRemesher_OnTimeStep;
		conditionCriterion = Dictionary_Get( context->dictionary, TIMESTEPCRITERION_STR );
		Journal_Printf( context->snacInfo, "Remesher is on... activated based on timeStep\n" );
		
		if( conditionCriterion ) {
			contextExt->OnTimeStep = Dictionary_Entry_Value_AsUnsignedInt( conditionCriterion );
		}
		else {
		}
		Journal_Printf( context->snacInfo, "Remeshing every %u timeSteps\n", contextExt->OnTimeStep );
	}
	else if( !strcmp( conditionStr, ONMINLENGTHSCALE_STR ) ) {
		contextExt->condition = SnacRemesher_OnMinLengthScale;
		conditionCriterion = Dictionary_Get( context->dictionary, LENGTHCRITERION_STR );
		Journal_Printf( context->snacInfo, "Remesher is on... activated by minLengthScale\n" );
		
		if( conditionCriterion ) {
			contextExt->onMinLengthScale = Dictionary_Entry_Value_AsDouble( conditionCriterion );
		}
		else {
		}
		Journal_Printf( context->snacInfo, "Remesh when minLengthScale < %g\n", contextExt->onMinLengthScale );
	}
	else if( !strcmp( conditionStr, ONBOTHTIMESTEPLENGTH_STR ) ) {
		contextExt->condition = SnacRemesher_OnBothTimeStepLength;
		conditionCriterion = Dictionary_Get( context->dictionary, TIMESTEPCRITERION_STR );
		Journal_Printf( context->snacInfo, "Remesher is on... activated by both timeStep and minLengthScale\n" );
		
		if( conditionCriterion ) {
			contextExt->OnTimeStep = Dictionary_Entry_Value_AsUnsignedInt( conditionCriterion );
			conditionCriterion = Dictionary_Get( context->dictionary, LENGTHCRITERION_STR );
			contextExt->onMinLengthScale = Dictionary_Entry_Value_AsDouble( conditionCriterion );
		}
		else {
		}
		Journal_Printf( context->snacInfo, "Remesh every %u timeSteps or wheen minLengthScale < %g\n", contextExt->OnTimeStep, contextExt->onMinLengthScale );
	}
	else {
		contextExt->condition = SnacRemesher_Off;
		Journal_Printf( context->snacInfo, "Remesher is defaulting to off\n" );
		Journal_Printf( error, "Provided remesh condition \"%s\" unrecognised\n", conditionStr );
	}
	
	
	/* Work out the mesh type */
	meshDict = Dictionary_Entry_Value_AsDictionary( Dictionary_Get( context->dictionary, MESH_STR ) );
	if( meshDict ) {
		char* 					meshTypeStr;
		
		if( !Dictionary_Get( meshDict, MESHTYPE_STR ) ) {
			Journal_Printf( 
				error,
				"Warning: No \"%s\" entry in \"%s\"... will default to \"%s\"\n", 
				MESHTYPE_STR, 
				MESH_STR, 
				CARTESIAN_STR );
		}
		
		meshTypeStr = Dictionary_Entry_Value_AsString( 
			Dictionary_GetDefault( meshDict, MESHTYPE_STR, Dictionary_Entry_Value_FromString( CARTESIAN_STR ) ) );
		
		if( !strcmp( meshTypeStr, SPHERICAL_STR ) ) {
			meshExt->meshType = SnacRemesher_Spherical;
			Journal_Printf( context->snacInfo, "Remesher knows mesh as a spherical mesh\n" );
		}
		else if( !strcmp( meshTypeStr, CARTESIAN_STR ) ) {
			meshExt->meshType = SnacRemesher_Cartesian;
			Journal_Printf( context->snacInfo, "Remesher knows mesh as a cartesian mesh\n" );
		}
		else {
			meshExt->meshType = SnacRemesher_Cartesian;
			Journal_Printf( context->snacInfo, "Remesher assuming mesh as a cartesian mesh\n" );
			Journal_Printf( error, "Provided mesh type \"%s\" unrecognised!\n", meshTypeStr );
		}
	}
	else {
		meshExt->meshType = SnacRemesher_Cartesian;
		Journal_Printf( context->snacInfo, "Remesher assuming mesh as a cartesian mesh\n" );
		Journal_Printf( error, "No \"%s\" entry in dictionary!\n", MESH_STR );
	}
	/* Decide whether to restore the bottom surface */
	contextExt->bottomRestore = 0;
	if( !strcmp( Dictionary_Entry_Value_AsString( Dictionary_GetDefault( context->dictionary, "bottomRestore", Dictionary_Entry_Value_FromString( OFF_STR ) ) ), ON_STR) )
		contextExt->bottomRestore = 1;
	
	/* Register these functions for use in VCs */
	ConditionFunction_Register_Add(
		context->condFunc_Register,
		ConditionFunction_New( _SnacRemesher_TestCondFunc, "SnacRemesher_TestCondFunc" ) );
	
	/* Register these functions for use in VCs */
	ConditionFunction_Register_Add(
		context->condFunc_Register,
		ConditionFunction_New( _SnacRemesher_XFunc, "SnacRemesher_XFunc" ) );
	
	/* Register these functions for use in VCs */
	ConditionFunction_Register_Add(
		context->condFunc_Register,
		ConditionFunction_New( _SnacRemesher_YFunc, "SnacRemesher_YFunc" ) );
	
	/* Obtain the keys for the our new entry points... having the keys saves doing a string compare at run time */
	contextExt->interpolateNodeK = EntryPoint_Register_GetHandle( 
		context->entryPoint_Register, 
		SnacRemesher_EP_InterpolateNode );
	contextExt->interpolateElementK = EntryPoint_Register_GetHandle( 
		context->entryPoint_Register, 
		SnacRemesher_EP_InterpolateElement );
	contextExt->copyElementK = EntryPoint_Register_GetHandle( 
		context->entryPoint_Register, 
		SnacRemesher_EP_CopyElement );

	/* Prepare the dump file */
	if( context->rank == 0) {
		sprintf( tmpBuf, "%s/remeshInfo.%u", context->outputPath, context->rank );
		if( (contextExt->remesherOut = fopen( tmpBuf, "w+" )) == NULL ) {
			assert( contextExt->remesherOut /* failed to open file for writing */ );
		}
	}
	/* initialize remeshing counter */
	contextExt->remeshingCount = 0;
}
