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
**  Mods by:
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
		Dictionary_GetDefault( context->dictionary, "slopeAngle", Dictionary_Entry_Value_FromDouble( 30.0f ) ) );

/* 	contextExt->rngSeed = Dictionary_Entry_Value_AsUnsignedInt( */
/* 		Dictionary_GetDefault( context->dictionary, "rngSeed", Dictionary_Entry_Value_FromUnsignedInt( 1 ) ) ); */

/* 	contextExt->fractionPlasticSeeds = Dictionary_Entry_Value_AsDouble( */
/* 		Dictionary_GetDefault( context->dictionary, "fractionPlasticSeeds", Dictionary_Entry_Value_FromDouble( 0.02f ) ) ); */




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
