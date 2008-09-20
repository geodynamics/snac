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
** $Id: ConstructExtensions.c 3131 2005-08-13 01:48:13Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Mesh.h"
#include "Register.h"
#include "ConstructExtensions.h"
#include "TestCondFunc.h"
#include <assert.h>
#include <limits.h>
#include <string.h>

static char MESH_STR[] = "mesh";
static char THETAMIN_STR[] = "thetaMin";
static char THETAMAX_STR[] = "thetaMax";
static char PHIMIN_STR[] = "phiMin";
static char PHIMAX_STR[] = "phiMax";
static char RMIN_STR[] = "rMin";
static char RMAX_STR[] = "rMax";
static double MIN[] = { -45.0f, -45.0f, 0.5f };
static double MAX[] = { +45.0f, +45.0f, 1.0f };

void _SnacSpherical_ConstructExtensions( void* _context, void* data ) {
	Snac_Context*			context = (Snac_Context*)_context;
	Mesh*				mesh = context->mesh;
	SnacSpherical_Mesh*		meshExt = ExtensionManager_Get(
						context->meshExtensionMgr,
						mesh,
						SnacSpherical_MeshHandle );
	Dictionary*			meshDict;

	#ifdef DEBUG
		printf( "In: %s\n", __func__ );
	#endif

		/* context->spherical becomes True here */
		context->spherical = True;

	meshDict = Dictionary_Entry_Value_AsDictionary( Dictionary_Get( context->dictionary, MESH_STR ) );
	if( meshDict ) {
		if( !Dictionary_Get( meshDict, THETAMIN_STR ) ) {
			printf( "Warning: No \"%s\" entry in \"%s\", defaulting to %g\n", THETAMIN_STR, MESH_STR, MIN[0] );
		}
		if( !Dictionary_Get( meshDict, THETAMAX_STR ) ) {
			printf( "Warning: No \"%s\" entry in \"%s\", defaulting to %g\n", THETAMAX_STR, MESH_STR, MAX[0] );
		}
		if( !Dictionary_Get( meshDict, PHIMIN_STR ) ) {
			printf( "Warning: No \"%s\" entry in \"%s\", defaulting to %g\n", PHIMIN_STR, MESH_STR, MIN[1] );
		}
		if( !Dictionary_Get( meshDict, PHIMAX_STR ) ) {
			printf( "Warning: No \"%s\" entry in \"%s\", defaulting to %g\n", PHIMAX_STR, MESH_STR, MIN[1] );
		}
		if( !Dictionary_Get( meshDict, RMIN_STR ) ) {
			printf( "Warning: No \"%s\" entry in \"%s\", defaulting to %g\n", RMIN_STR, MESH_STR, MIN[2] );
		}
		if( !Dictionary_Get( meshDict, RMAX_STR ) ) {
			printf( "Warning: No \"%s\" entry in \"%s\", defaulting to %g\n", RMAX_STR, MESH_STR, MIN[2] );
		}


		meshExt->min[0] = Dictionary_Entry_Value_AsDouble(
			Dictionary_GetDefault( meshDict, THETAMIN_STR, Dictionary_Entry_Value_FromDouble( MIN[0] ) ) );
		meshExt->max[0] = Dictionary_Entry_Value_AsDouble(
			Dictionary_GetDefault( meshDict, THETAMAX_STR, Dictionary_Entry_Value_FromDouble( MAX[0] ) ) );
		meshExt->min[1] = Dictionary_Entry_Value_AsDouble(
			Dictionary_GetDefault( meshDict, PHIMIN_STR, Dictionary_Entry_Value_FromDouble( MIN[1] ) ) );
		meshExt->max[1] = Dictionary_Entry_Value_AsDouble(
			Dictionary_GetDefault( meshDict, PHIMAX_STR, Dictionary_Entry_Value_FromDouble( MAX[1] ) ) );
		meshExt->min[2] = Dictionary_Entry_Value_AsDouble(
			Dictionary_GetDefault( meshDict, RMIN_STR, Dictionary_Entry_Value_FromDouble( MIN[2] ) ) );
		meshExt->max[2] = Dictionary_Entry_Value_AsDouble(
			Dictionary_GetDefault( meshDict, RMAX_STR, Dictionary_Entry_Value_FromDouble( MAX[2] ) ) );

		printf(
			"Spherical geometry: { min: { %g, %g, %g }, max: { %g, %g, %g } }\n",
			meshExt->min[0],
			meshExt->min[1],
			meshExt->min[2],
			meshExt->max[0],
			meshExt->max[1],
			meshExt->max[2] );
	}
	else {
		printf(
			"Warning: No \"%s\" entry, defaulting to spherical geometry: { min: { %g, %g, %g }, max: { %g, %g, %g }}\n",
			MESH_STR,
			meshExt->min[0],
			meshExt->min[1],
			meshExt->min[2],
			meshExt->max[0],
			meshExt->max[1],
			meshExt->max[2] );
	}
	/* Registere condition functions to apply plate velocties */
	ConditionFunction_Register_Add(
		context->condFunc_Register,
		ConditionFunction_New( _SnacSpherical_applyPlateVx, "SnacSpherical_applyPlateVx" ) );
	ConditionFunction_Register_Add(
		context->condFunc_Register,
		ConditionFunction_New( _SnacSpherical_applyPlateVy, "SnacSpherical_applyPlateVy" ) );
	ConditionFunction_Register_Add(
		context->condFunc_Register,
		ConditionFunction_New( _SnacSpherical_applyPlateVz, "SnacSpherical_applyPlateVz" ) );
	ConditionFunction_Register_Add(
		context->condFunc_Register,
		ConditionFunction_New( _SnacSpherical_applyInitialVx, "SnacSpherical_applyInitialVx" ) );
	ConditionFunction_Register_Add(
		context->condFunc_Register,
		ConditionFunction_New( _SnacSpherical_applyInitialVy, "SnacSpherical_applyInitialVy" ) );
	ConditionFunction_Register_Add(
		context->condFunc_Register,
		ConditionFunction_New( _SnacSpherical_applyInitialVz, "SnacSpherical_applyInitialVz" ) );

}
