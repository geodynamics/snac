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
** $Id: ConstructExtensions.c 3265 2006-11-15 21:43:39Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Node.h"
#include "Context.h"
#include "Register.h"
#include "VariableConditions.h"
#include "ConstructExtensions.h"
#include <assert.h>
#include <limits.h>
#ifndef PATH_MAX
	#define PATH_MAX 1024
#endif

void _SnacTemperature_ConstructExtensions( void* _context, void* data ) {
	Snac_Context*				context = (Snac_Context*)_context;
	SnacTemperature_Context*		contextExt = ExtensionManager_Get(
							context->extensionMgr,
							context,
							SnacTemperature_ContextHandle );
	Snac_Node				tmpNode;
	SnacTemperature_Node*			tmpNodeExt = ExtensionManager_Get(
							context->mesh->nodeExtensionMgr,
							&tmpNode,
							SnacTemperature_NodeHandle );
	Dictionary*				temperatureBCsDict;
	char					tmpBuf[PATH_MAX];
	
	/* Because temperature is not an array by itself, we must the "complex" constructor for Variable... the info needs to be
	 * wrapped this generic way... */
	Index					temperatureOffsetCount = 1;
	SizeT					temperatureOffsets[] = { /*GetOffsetOfMember( *tmpNodeExt, temperature ) };*/
		(SizeT)((char*)&tmpNodeExt->temperature - (char*)&tmpNode) };
	Variable_DataType			temperatureDataTypes[] = { Variable_DataType_Double };
	Index					temperatureDataTypeCounts[] = { 1 };
	
		
	#if DEBUG
		printf( "In %s()\n", __func__ );
	#endif

	/* Create the StGermain variable temperature, which is stored on a node extension */
	Variable_New( 
		"temperature", 
		temperatureOffsetCount, 
		temperatureOffsets, 
		temperatureDataTypes, 
		temperatureDataTypeCounts, 
		0, 
		&ExtensionManager_GetFinalSize( context->mesh->nodeExtensionMgr ),
		&context->mesh->layout->decomp->nodeDomainCount,
		(void**)&context->mesh->node,
		context->variable_Register );

	/* Register these functions for use in VCs */
	ConditionFunction_Register_Add(
		context->condFunc_Register,
		ConditionFunction_New( _SnacTemperature_Top2BottomSweep, "SnacTemperature_Top2BottomSweep" ) );
	ConditionFunction_Register_Add(
		context->condFunc_Register,
		ConditionFunction_New( _SnacTemperature_Top2BottomSweep_Spherical, "SnacTemperature_Top2BottomSweep_Spherical" ) );
	ConditionFunction_Register_Add(
		context->condFunc_Register,
		ConditionFunction_New( _SnacTemperature_Citcom_Compatible, "SnacTemperature_Citcom_Compatible" ) );

	/* Temperature variables */
	contextExt->topTemp = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( context->dictionary, "topTemp", Dictionary_Entry_Value_FromDouble( 0.0f ) ) );

	contextExt->bottomTemp = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( context->dictionary, "bottomTemp", Dictionary_Entry_Value_FromDouble( 1300.0f ) ) );

	/* Build the temperature IC and BC managers */
	temperatureBCsDict = Dictionary_Entry_Value_AsDictionary( Dictionary_Get( context->dictionary, "temperatureBCs" ) );
	contextExt->temperatureBCs = CompositeVC_New("tempBC",
		context->variable_Register,
		context->condFunc_Register,
		temperatureBCsDict,
		context->mesh );

	/* Prepare the dump file */
	sprintf( tmpBuf, "%s/temperature.%u", context->outputPath, context->rank );
	if( (contextExt->temperatureOut = fopen( tmpBuf, "w+" )) == NULL ) {
		assert( contextExt->temperatureOut /* failed to open file for writing */ );
	}
}
