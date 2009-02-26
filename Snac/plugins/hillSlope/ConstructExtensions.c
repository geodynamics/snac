/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003,
**	Steve Quenette, 110 Victoria Street, Melbourne, Victoria, 3053, Australia.
**	Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
**	University of Texas, 1 University Station, Austin, Texas, 78712, USA.
**
** Authors:
**           Colin Stark, Doherty Research Scientist, Lamont-Doherty Earth Observatory (cstark@ldeo.columbia.edu)
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
** $Id: ConstructExtensions.c $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Context.h"
#include "Register.h"
//#include "InitialConditions.h"
#include "ConstructExtensions.h"
#include <assert.h>
#include <limits.h>

//#define DEBUG

void _SnacHillSlope_ConstructExtensions( void* _context, void* data ) {
	Snac_Context*				context = (Snac_Context*)_context;
	SnacHillSlope_Context*			contextExt = ExtensionManager_Get(
								context->extensionMgr,
								context,
								SnacHillSlope_ContextHandle );
/* 	Dictionary*				hillSlopeBCsDict; */
/* 	char					tmpBuf[PATH_MAX]; */

	/* Because hillSlope is not an array by itself, we must the "complex" constructor for Variable... the info needs to be
	 * wrapped this generic way... */
/* 	Index					hillSlopeOffsetCount = 1; */
/*  	SizeT					hillSlopeOffsets[] = { (SizeT)((char*)&tmpNodeExt->hillSlope - (char*)&tmpNode) };  */
/* 	Variable_DataType			hillSlopeDataTypes[] = { Variable_DataType_Double }; */
/* 	Index					hillSlopeDataTypeCounts[] = { 1 }; */
	#ifdef DEBUG
		printf( "In %s()\n", __func__ );
	#endif


	/* Create the StGermain variable hillSlope, which is stored on a node extension */
/* 	Variable_New(  */
/* 		"hillSlope",  */
/* 		hillSlopeOffsetCount,  */
/* 		hillSlopeOffsets,  */
/* 		hillSlopeDataTypes,  */
/* 		hillSlopeDataTypeCounts,  */
/* 		0,  */
/* 		&ExtensionManager_GetFinalSize( context->mesh->nodeExtensionMgr ), */
/* 		&context->mesh->layout->decomp->nodeDomainCount, */
/* 		(void**)&context->mesh->node, */
/* 		context->variable_Register ); */



	/* HillSlope variables */
	contextExt->slopeAngle = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( context->dictionary, "slopeAngle", 
				       Dictionary_Entry_Value_FromDouble( 0.0f ) ) );

	contextExt->rngSeed = Dictionary_Entry_Value_AsUnsignedInt(
		Dictionary_GetDefault( context->dictionary, "rngSeed", 
				       Dictionary_Entry_Value_FromUnsignedInt( 1 ) ) );

	contextExt->fractionWeakPoints = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( context->dictionary, "fractionWeakPoints", 
				       Dictionary_Entry_Value_FromDouble( 0.02f ) ) );

	contextExt->resolveDepth = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( context->dictionary, "resolveDepth", 
				       Dictionary_Entry_Value_FromDouble( -1.0f ) ) );

	contextExt->leftFlatFraction = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( context->dictionary, "leftFlatFraction", 
				       Dictionary_Entry_Value_FromDouble( 0.05f ) ) );
	contextExt->rightFlatFraction = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( context->dictionary, "rightFlatFraction", 
				       Dictionary_Entry_Value_FromDouble( -1.0f ) ) );
	contextExt->rampFlatSmoothFactor = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( context->dictionary, "rampFlatSmoothFactor", 
				       Dictionary_Entry_Value_FromDouble( 3.0f ) ) );

	contextExt->xSubDomainFraction = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( context->dictionary, "xSubDomainFraction", 
				       Dictionary_Entry_Value_FromDouble( 0.5f ) ) );
	contextExt->ySubDomainFraction = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( context->dictionary, "ySubDomainFraction", 
				       Dictionary_Entry_Value_FromDouble( 0.4f ) ) );
	contextExt->zSubDomainFraction = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( context->dictionary, "zSubDomainFraction", 
				       Dictionary_Entry_Value_FromDouble( 0.75f ) ) );

	contextExt->xTriggerPointFraction = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( context->dictionary, "xTriggerPointFraction", 
				       Dictionary_Entry_Value_FromDouble( 0.5f ) ) );
	contextExt->yTriggerPointFraction = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( context->dictionary, "yTriggerPointFraction", 
				       Dictionary_Entry_Value_FromDouble( 0.4f ) ) );
	contextExt->zTriggerPointFraction = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( context->dictionary, "zTriggerPointFraction", 
				       Dictionary_Entry_Value_FromDouble( 0.5f ) ) );

	contextExt->weakPointCohesion = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( context->dictionary, "weakPointCohesion", 
				       Dictionary_Entry_Value_FromDouble( 4.0e+05f ) ) );
	contextExt->triggerPointCohesion = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( context->dictionary, "triggerPointCohesion", 
				       Dictionary_Entry_Value_FromDouble( 1.0e+04f ) ) );

	contextExt->trackLevel = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( context->dictionary, "trackLevel", 
				       Dictionary_Entry_Value_FromDouble( 0.0f ) ) );

	contextExt->startThreshold = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( context->dictionary, "startThreshold", 
				       Dictionary_Entry_Value_FromDouble( 1.0f ) ) );
	contextExt->stopThreshold = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( context->dictionary, "stopThreshold", 
				       Dictionary_Entry_Value_FromDouble( 1.0f ) ) );

	contextExt->startedTrackingFlag = Dictionary_Entry_Value_AsBool(
		Dictionary_GetDefault( context->dictionary, "startedTrackingFlag", 
				       Dictionary_Entry_Value_FromBool( 0 ) ) );
	contextExt->elasticStabilizedFlag = Dictionary_Entry_Value_AsBool(
		Dictionary_GetDefault( context->dictionary, "elasticStabilizedFlag", 
				       Dictionary_Entry_Value_FromBool( 0 ) ) );
	contextExt->solveElasticEqmOnlyFlag = Dictionary_Entry_Value_AsBool(
		Dictionary_GetDefault( context->dictionary, "solveElasticEqmOnlyFlag", 
				       Dictionary_Entry_Value_FromBool( 0 ) ) );
	contextExt->seedingCompletedFlag = Dictionary_Entry_Value_AsBool(
		Dictionary_GetDefault( context->dictionary, "seedingCompletedFlag", 
				       Dictionary_Entry_Value_FromBool( 0 ) ) );

	/* 
	 *  Open the output streams for mesh data
	 */
/* 	sprintf( tmpBuf, "%s/shearStress.%u", context->outputPath, context->rank ); */
/* 	if( (contextExt->shearStress = fopen( tmpBuf, "w+" )) == NULL ) { */
/* 	    assert( contextExt->shearStress ); */
/* 	} */
/* 	sprintf( tmpBuf, "%s/failurePotential.%u", context->outputPath, context->rank ); */
/* 	if( (contextExt->failurePotential = fopen( tmpBuf, "w+" )) == NULL ) { */
/* 	    assert( contextExt->failurePotential); */
/* 	} */


	/* Build the hillSlope IC and BC managers */
/* 	hillSlopeBCsDict = Dictionary_Entry_Value_AsDictionary( Dictionary_Get( context->dictionary, "hillSlopeBCs" ) ); */
/* 	contextExt->hillSlopeBCs = CompositeVC_New("tempBC", */
/* 		context->variable_Register, */
/* 		context->condFunc_Register, */
/* 		hillSlopeBCsDict, */
/* 		context->mesh ); */


/* 	#ifdef DEBUG */
/* 		fprintf( stderr, "In %s()\n", __func__ ); */
/* 	#endif */

/* 	Journal_Printf( context->debug, "slopeAngle:  %g\n", contextExt->slopeAngle ); */

}
