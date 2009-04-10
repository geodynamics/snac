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
** $Id: Register.c 1095 2004-03-28 00:51:42Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "units.h"
#include "types.h"
#include "Register.h"
#include "Context.h"
#include "Node.h"
#include "Element.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#ifndef PATH_MAX
	#define PATH_MAX 1024
#endif

extern void effectiveDensity( void* _context );

void _SnacTemperature_Top2BottomSweep( Node_LocalIndex node_lI, Variable_Index var_I, void* _context, void* result ) {
	Snac_Context*			context = (Snac_Context*)_context;
	SnacTemperature_Context*	contextExt = ExtensionManager_Get(
						context->extensionMgr,
						context,
						SnacTemperature_ContextHandle );
	double*				temperature = (double*)result;
	MeshLayout*			meshLayout = (MeshLayout*)context->meshLayout;
	HexaMD*				decomp = (HexaMD*)meshLayout->decomp;
	Node_GlobalIndex		node_gI;
	IJK				ijk;
	const Index			jCount = decomp->nodeGlobal3DCounts[1];

	node_gI = context->mesh->nodeL2G[node_lI];
	RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );

  	*temperature = contextExt->bottomTemp + (contextExt->topTemp - contextExt->bottomTemp) / (jCount - 1) * ijk[1];

	#if 0
		printf( "Top: %8g, Bottom: %8g, node_I: %3u, node_gI: %3u, iijk: { %3u, %3u, %3u, }, *temperature: %g\n", contextExt->topTemp, contextExt->bottomTemp, node_lI, node_gI, ijk[0], ijk[1], ijk[2], *temperature );
	#endif
}


void _SnacTemperature_Top2BottomSweep_Spherical(
		Node_LocalIndex			node_lI,
		Variable_Index			var_I,
		void*				_context,
		void*				result )
{
	Snac_Context*			context = (Snac_Context*)_context;
	SnacTemperature_Context*	contextExt = ExtensionManager_Get(
						context->extensionMgr,
						context,
						SnacTemperature_ContextHandle );
	Coord*				coord = Snac_NodeCoord_P( context, node_lI );

	double*				temperature = (double*)result;
	Dictionary*			meshStruct= Dictionary_Entry_Value_AsDictionary(
						Dictionary_Get( context->dictionary, "mesh" ) );
	double				rMin = Dictionary_Entry_Value_AsDouble( Dictionary_Get( meshStruct, "rMin" ) );
	double				rMax = Dictionary_Entry_Value_AsDouble( Dictionary_Get( meshStruct, "rMax" ) );
	double				r = sqrt((*coord)[0]*(*coord)[0] + (*coord)[1]*(*coord)[1] + (*coord)[2]*(*coord)[2]);

	assert( (rMin != 0.0 && rMax != 0.0) );
	r /= 6371000.0;
	rMin /= 6371000.0;
	rMax /= 6371000.0;

	(*temperature) = contextExt->bottomTemp + (contextExt->topTemp - contextExt->bottomTemp) / (rMax - rMin) * (r - rMin);

	#if 0
		printf( "Top: %8g, Bottom: %8g, node_lI: %3u, node_gI: %3u, iijk: { %3u, %3u, %3u, }, *temperature: %g\n",
			contextExt->topTemp,
			contextExt->bottomTemp,
			node_lI,
			node_gI,
			ijk[0],
			ijk[1],
			ijk[2],
			*temperature );
	#endif
}

void _SnacTemperature_Citcom_Compatible(
		Node_LocalIndex                 node_lI,
		Variable_Index                  var_I,
		void*                           _context,
		void*                           result )
{
	Snac_Context*                   context = (Snac_Context*)_context;
	SnacTemperature_Context*	contextExt = ExtensionManager_Get(
						context->extensionMgr,
						context,
						SnacTemperature_ContextHandle );
	Mesh*				mesh = context->mesh;
	MeshLayout*			layout = (MeshLayout*)mesh->layout;
	HexaMD*				decomp = (HexaMD*)layout->decomp;
	Node_GlobalIndex		node_gI = _MeshDecomp_Node_LocalToGlobal1D( decomp, node_lI );
	Coord*                          coord = Snac_NodeCoord_P( context, node_lI );

	const Snac_Material*            material = &context->materialProperty[0];
	const double                    rTemp = contextExt->bottomTemp;
	const double                    R = 6371000.0;
	const double                    kappa = 1.0e-06;
	double                          scalet = R*R/kappa/(1.0e+06*365.25*24.0*3600.0);
	double                          age = 5.01f, temp=0.0f;

	double*                         temperature = (double*)result;
	Dictionary*                     meshStruct= Dictionary_Entry_Value_AsDictionary(
	Dictionary_Get( context->dictionary, "mesh" ) );
	double                          rMin = Dictionary_Entry_Value_AsDouble( Dictionary_Get( meshStruct, "rMin" ) );
	double                          rMax = Dictionary_Entry_Value_AsDouble( Dictionary_Get( meshStruct, "rMax" ) );
	double                          r = sqrt((*coord)[0]*(*coord)[0] + (*coord)[1]*(*coord)[1] + (*coord)[2]*(*coord)[2]);
	IJK                             ijk;
	const Node_GlobalIndex		midI = (decomp->nodeGlobal3DCounts[0] + 1) / 2 - 1;
	Node_GlobalIndex		lmidI;

	RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );

	/* for Cartesian case */
	rMin = R - 3.0e+03f;
	rMax = R;
	r = rMax + (*coord)[1];
	/*ccccc*/

	assert( (rMin != 0.0 && rMax != 0.0) );
	r /= R;
	rMin /= R;
	rMax /= R;

#if 0
	if( ijk[0] >= midI-5 && ijk[0] <= midI+5 ) {
		age = 0.01f + 1.0f*abs(ijk[0]-midI)/5.0*5.01;
	}
#endif
	age = 0.5 - 0.2*ijk[2]/(decomp->nodeGlobal3DCounts[2]-1);

	temp = (rMax-r) * 0.5f / sqrt(age/scalet);
	*temperature = rTemp * erf(temp);
	if( (*temperature) < 0.0) (*temperature) = 0.0f;
}

void _SnacTemperature_InitialConditions( void* _context ) {
	Snac_Context*			context = (Snac_Context*)_context;
	SnacTemperature_Context*	contextExt = ExtensionManager_Get(
						context->extensionMgr,
						context,
						SnacTemperature_ContextHandle );
	Element_LocalIndex		element_lI;

	Dictionary_Entry_Value* pluginsList;
	Dictionary_Entry_Value* plugin;

	Journal_Printf( context->snacInfo, "In: %s\n", __func__ );

	/* Temperature ICs are applied onto the "nodeICs" of Snac, and hence do not need to be repeated, but we must apply BCs */

	/* In case of restarting, Temperature IC is still applied here */
	if( context->restartTimestep>0 ) {
		FILE*				fp;
		Node_LocalIndex			node_lI;
		char				path[PATH_MAX];

		sprintf(path, "%s/snac.temp.%d.%06d.restart",context->outputPath,context->rank,context->restartTimestep);
		Journal_Firewall( ( (fp = fopen(path,"r")) != NULL ), context->snacError, "Cannot find %s", path );

		/* read in restart file to construct the initial temperature */
		for( node_lI = 0; node_lI < context->mesh->nodeLocalCount; node_lI++ ) {
			Snac_Node*			node = Snac_Node_At( context, node_lI );
			SnacTemperature_Node*		nodeExt = ExtensionManager_Get( context->mesh->nodeExtensionMgr, node, SnacTemperature_NodeHandle );
			double				T;
			fscanf( fp, "%le", &T);

			nodeExt->temperature = T;
			nodeExt->temperature0 = nodeExt->temperature;
		}
		if( fp )
			fclose( fp );
	}
	else {
		Node_LocalIndex			node_lI;

		/* if not restarting, just copy initialized T to T0. */
		for( node_lI = 0; node_lI < context->mesh->nodeLocalCount; node_lI++ ) {
			Snac_Node*			node = Snac_Node_At( context, node_lI );
			SnacTemperature_Node*		nodeExt = ExtensionManager_Get( context->mesh->nodeExtensionMgr, node, SnacTemperature_NodeHandle );
 			nodeExt->temperature0 = nodeExt->temperature;
		}
	}
	VariableCondition_Apply( contextExt->temperatureBCs, context );

#if 0
	{
		Node_LocalIndex		node_lI;

		for( node_lI=0; node_lI < context->mesh->nodeLocalCount; node_lI++ ) {
			Snac_Node*			node = Snac_Node_At( context, node_lI );
			SnacTemperature_Node*		nodeExt = ExtensionManager_Get(
								context->mesh->nodeExtensionMgr,
								node,
								SnacTemperature_NodeHandle );

			printf( "node_lI: %3u, nodeExt->temperature: %g\n", node_lI, nodeExt->temperature );
		}
	}
#endif

	/* Apply element heat flux ICs */
	for( element_lI = 0; element_lI < context->mesh->elementLocalCount; element_lI++ ) {
		Snac_Element*			element = Snac_Element_At( context, element_lI );
		SnacTemperature_Element*	elementExt = ExtensionManager_Get(
							context->mesh->elementExtensionMgr,
							element,
							SnacTemperature_ElementHandle );

		memset( elementExt->heatFlux, 0, sizeof(elementExt->heatFlux) );
	}

	effectiveDensity( context );

}

void SnacTemperature_BoundaryConditions( void* _context ) {
	Snac_Context*			context = (Snac_Context*)_context;
	SnacTemperature_Context*	contextExt = ExtensionManager_Get(
						context->extensionMgr,
						context,
						SnacTemperature_ContextHandle );

	#if 0
		Node_GlobalIndex		node_gI, node_gI2;
		Node_LocalIndex			node_lI;
		IJK				ijk;
		MeshLayout*			meshLayout = (MeshLayout*)context->meshLayout;
		HexaMeshTopology*		topology = (HexaMeshTopology*)meshLayout->topology;
		RegularMeshDecomp*		decomp = (RegularMeshDecomp*)meshLayout->decomp;
	#endif



	Journal_Printf( context->debug, "In %s(): re-applying temp BCs\n", __func__ );

	VariableCondition_Apply( contextExt->temperatureBCs, context );
}

void SnacTemperature_HeatFluxBC( void* _context ) {
	Snac_Context*                   context = (Snac_Context*)_context;
	Element_LocalIndex              element_lI;

	Journal_Printf( context->debug, "In %s(): re-applying temp BCs\n", __func__ );

	/* no heat flux bc for the vertical walls */
	for( element_lI = 0; element_lI < context->mesh->elementLocalCount; element_lI++ ) {
		Snac_Context*                 context = (Snac_Context*)_context;
		IJK                           ijk;
		Tetrahedra_Index              tetra_I;
		MeshLayout*                   meshLayout = (MeshLayout*)context->meshLayout;
		HexaMD*                       decomp = (HexaMD*)meshLayout->decomp;
		Element_GlobalIndex           element_gI = decomp->elementMapLocalToGlobal( decomp, element_lI );
		Snac_Element*                 element = Snac_Element_At( context, element_lI );
		SnacTemperature_Element*      elementExt = ExtensionManager_Get(
																		context->mesh->elementExtensionMgr,
																		element,
																		SnacTemperature_ElementHandle );
		double                  g[4],Fn,normal[3];
		int                             i;

		RegularMeshUtils_Element_1DTo3D( decomp, element_gI, &ijk[0], &ijk[1], &ijk[2] );

		if( ijk[0] == 0  || ijk[0] == decomp->elementGlobal3DCounts[0]-1 || ijk[2] == 0  || ijk[2] == decomp->elementGlobal3DCounts[2]-1) {
			if( ijk[0] == 0 ) {
				normal[0] = 0.25f * (element->tetra[0].surface[2].normal[0] +
										element->tetra[1].surface[2].normal[0] +
										element->tetra[5].surface[0].normal[0] +
										element->tetra[6].surface[0].normal[0]);
				normal[1] = 0.25f * (element->tetra[0].surface[2].normal[1] +
										element->tetra[1].surface[2].normal[1] +
										element->tetra[5].surface[0].normal[1] +
										element->tetra[6].surface[0].normal[1]);
				normal[2] = 0.25f * (element->tetra[0].surface[2].normal[2] +
										element->tetra[1].surface[2].normal[2] +
										element->tetra[5].surface[0].normal[2] +
										element->tetra[6].surface[0].normal[2]);
			}
			if(ijk[0] == decomp->elementGlobal3DCounts[0]-1) {
				normal[0] = 0.25f * (element->tetra[2].surface[0].normal[0] +
										element->tetra[3].surface[0].normal[0] +
										element->tetra[7].surface[0].normal[0] +
										element->tetra[8].surface[0].normal[0]);
				normal[1] = 0.25f * (element->tetra[2].surface[0].normal[1] +
										element->tetra[3].surface[0].normal[1] +
										element->tetra[7].surface[0].normal[1] +
										element->tetra[8].surface[0].normal[1]);
				normal[2] = 0.25f * (element->tetra[2].surface[0].normal[2] +
										element->tetra[3].surface[0].normal[2] +
										element->tetra[7].surface[0].normal[2] +
										element->tetra[8].surface[0].normal[2]);
			}
			if(ijk[2] == 0 ) {
				normal[0] = 0.25f * (element->tetra[0].surface[1].normal[0] +
										element->tetra[2].surface[2].normal[0] +
										element->tetra[6].surface[1].normal[0] +
										element->tetra[7].surface[1].normal[0]);
				normal[1] = 0.25f * (element->tetra[0].surface[1].normal[1] +
										element->tetra[2].surface[2].normal[1] +
										element->tetra[6].surface[1].normal[1] +
										element->tetra[7].surface[1].normal[1]);
				normal[2] = 0.25f * (element->tetra[0].surface[1].normal[2] +
										element->tetra[2].surface[2].normal[2] +
										element->tetra[6].surface[1].normal[2] +
										element->tetra[7].surface[1].normal[2]);
			}
			if(ijk[2] == decomp->elementGlobal3DCounts[2]-1) {
				normal[0] = 0.25f * (element->tetra[1].surface[1].normal[0] +
										element->tetra[3].surface[2].normal[0] +
										element->tetra[5].surface[1].normal[0] +
										element->tetra[8].surface[1].normal[0]);
				normal[1] = 0.25f * (element->tetra[1].surface[1].normal[1] +
										element->tetra[3].surface[2].normal[1] +
										element->tetra[5].surface[1].normal[1] +
										element->tetra[8].surface[1].normal[1]);
				normal[2] = 0.25f * (element->tetra[1].surface[1].normal[2] +
										element->tetra[3].surface[2].normal[2] +
										element->tetra[5].surface[1].normal[2] +
										element->tetra[8].surface[1].normal[2]);
			}
			for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
				Snac_Element_Tetrahedra*                tetra = &element->tetra[tetra_I];
				Snac_Node*                          contributingNode[4] = {
						Snac_Element_Node_P( context, element_lI, TetraToNode[tetra_I][0] ),
						Snac_Element_Node_P( context, element_lI, TetraToNode[tetra_I][1] ),
						Snac_Element_Node_P( context, element_lI, TetraToNode[tetra_I][2] ),
						Snac_Element_Node_P( context, element_lI, TetraToNode[tetra_I][3] ) };
				SnacTemperature_Node*           nodeExt[4] = {
						ExtensionManager_Get(
												context->mesh->nodeExtensionMgr,
												contributingNode[0],
												SnacTemperature_NodeHandle ),
						ExtensionManager_Get(
												context->mesh->nodeExtensionMgr,
												contributingNode[1],
												SnacTemperature_NodeHandle ),
						ExtensionManager_Get(
												context->mesh->nodeExtensionMgr,
												contributingNode[2],
												SnacTemperature_NodeHandle ),
						ExtensionManager_Get(
												context->mesh->nodeExtensionMgr,
												contributingNode[3],
												SnacTemperature_NodeHandle ),
				};

				g[0] = -1.0f * ( nodeExt[0]->temperature * tetra->surface[1].normal[0] * tetra->surface[1].area +
									nodeExt[1]->temperature * tetra->surface[2].normal[0] * tetra->surface[2].area +
									nodeExt[2]->temperature * tetra->surface[3].normal[0] * tetra->surface[3].area +
									nodeExt[3]->temperature * tetra->surface[0].normal[0] * tetra->surface[0].area )
						/ 3.0f / element->tetra[tetra_I].volume;
				g[1] = -1.0f * ( nodeExt[0]->temperature * tetra->surface[1].normal[1] * tetra->surface[1].area +
									nodeExt[1]->temperature * tetra->surface[2].normal[1] * tetra->surface[2].area +
									nodeExt[2]->temperature * tetra->surface[3].normal[1] * tetra->surface[3].area +
									nodeExt[3]->temperature * tetra->surface[0].normal[1] * tetra->surface[0].area )
						/ 3.0f / element->tetra[tetra_I].volume;
				g[2] = -1.0f * ( nodeExt[0]->temperature * tetra->surface[1].normal[2] * tetra->surface[1].area +
									nodeExt[1]->temperature * tetra->surface[2].normal[2] * tetra->surface[2].area +
									nodeExt[2]->temperature * tetra->surface[3].normal[2] * tetra->surface[3].area +
									nodeExt[3]->temperature * tetra->surface[0].normal[2] * tetra->surface[0].area )
						/ 3.0f / element->tetra[tetra_I].volume;

				Fn = 0.0f;
				for( i=0; i<3;i++)
					Fn += g[i] * normal[i];

				elementExt->heatFlux[tetra_I][0] -= Fn*normal[0];
				elementExt->heatFlux[tetra_I][1] -= Fn*normal[1];
				elementExt->heatFlux[tetra_I][2] -= Fn*normal[2];
			}
		}
	}
}
