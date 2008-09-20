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
** $Id: UpdateNode.c 3185 2006-01-16 22:07:43Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>

#include "units.h"
#include "types.h"
#include "shortcuts.h"
#include "Material.h"
#include "Tetrahedra.h"
#include "TetrahedraTables.h"
#include "Node.h"
#include "Element.h"
#include "Context.h"
#include "Stress.h"
#include "UpdateNode.h"

#include <math.h>
#include <string.h>
#include <assert.h>

/* There is no standard C "sign" function... so I have made a macro for one here */
#define getsign( val )		( (val) < 0 ? -1.0f : (val) > 0 ? +1.0f : 0.0f )
#define sign( a, b )		getsign( (b) )
#define max( a, b )		    ( (a > b)? a : b )
#define R                   6371000.0

void Cart2Spherical_Coord( Coord *X, double XC[] );

void Snac_UpdateNodeMomentum( void* context, Node_LocalIndex node_lI, Mass inertialMass, Force force ) {
	Snac_Context*		self = (Snac_Context*)context;
	Snac_Node*			node = Snac_Node_At( self, node_lI );
	Coord*				coord = Snac_NodeCoord_P( self, node_lI );

	/* Update coordinates of the nodes */
	(*coord)[0] += self->dt * node->velocity[0];
	(*coord)[1] += self->dt * node->velocity[1];
	(*coord)[2] += self->dt * node->velocity[2];

	if(self->topo_kappa > 0)
		getMaxTopoGrad( context, node_lI );
}

void Snac_UpdateNodeMomentum_PreProcess( void* context, Node_LocalIndex node_lI, Mass inertialMass, Force force ) {
	Snac_Context*					self = (Snac_Context*)context;
	Mesh*							mesh = self->mesh;
	HexaMD*							decomp = (HexaMD*)mesh->layout->decomp;
	Snac_Node*						node = Snac_Node_At( self, node_lI );
	Coord*							coord = Snac_NodeCoord_P( self, node_lI );

	Dictionary_Entry_Value* extensionsList;
	Dictionary_Entry_Value* extension;
	int Spherical = 0;
	assert( self->dictionary );
	extensionsList = Dictionary_Get( self->dictionary, "extensions" );
	if(!extensionsList)
		extensionsList = Dictionary_Get( self->dictionary, "plugins" );
	extension = Dictionary_Entry_Value_GetFirstElement(extensionsList);
	while ( extension ) {
		if ( 0 == strcmp( Dictionary_Entry_Value_AsString( extension ),
							"SnacSpherical" ) ) {
			Spherical = 1;
			break;
		}
		extension = extension->next;
	}

	/* Balance of forces for checks */
	/* luc to do */

	/* Damp forces here */
	force[0] -= self->demf * getsign( node->velocity[0] ) * fabs( force[0] );
	force[1] -= self->demf * getsign( node->velocity[1] ) * fabs( force[1] );
	force[2] -= self->demf * getsign( node->velocity[2] ) * fabs( force[2] );
	Journal_DFirewall( !isnan(force[0]) && !isinf(force[0]), self->snacError, "me: %d node_lI: %u, force[0] is either nan or inf  velocity[0]=%e force[0]=%e\n",self->rank,node_lI,node->velocity[0],force[0]);
	Journal_DFirewall( !isnan(force[1]) && !isinf(force[1]), self->snacError, "me: %d node_lI: %u, force[1] is either nan or inf  velocity[1]=%e force[1]=%e\n",self->rank,node_lI,node->velocity[1],force[1]);
	Journal_DFirewall( !isnan(force[2]) && !isinf(force[2]), self->snacError, "me: %d node_lI: %u, force[2] is either nan or inf  velocity[2]=%e force[2]=%e\n",self->rank,node_lI,node->velocity[2],force[2]);

	/* Update Velocities at the nodes */
	node->velocity[0] += self->dt * force[0] / inertialMass;
	node->velocity[1] += self->dt * force[1] / inertialMass;
	node->velocity[2] += self->dt * force[2] / inertialMass;
	/* Apply boundary conditions */
	/* VariableCondition_ApplyToIndex( self->velocityBCs, node_lI, self ); */
	if(self->spherical) {
		IJK                             ijk;
		Node_GlobalIndex node_gI;
		double radius,theta,phi;
		double sphV[3];
		/*double sphVinner[3],tmpForce[3];*/
		/* Snac_Node *inner_node; */

		radius = sqrt( (*coord)[0]*(*coord)[0] + (*coord)[1]*(*coord)[1] + (*coord)[2]*(*coord)[2] );
		theta = acos((*coord)[2]/radius);
		phi = atan2((*coord)[1],(*coord)[0]);

		node_gI = _MeshDecomp_Node_LocalToGlobal1D( decomp, node_lI );
		RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );

		if(ijk[0] == 0 || ijk[0] == decomp->nodeGlobal3DCounts[0]-1) {
			sphV[0] = node->velocity[0]*sin(theta)*cos(phi) + node->velocity[1]*sin(theta)*sin(phi) + node->velocity[2]*cos(theta);
			VariableCondition_ApplyToIndex( self->velocityBCs, node_lI, self );
			sphV[1] = node->velocity[0]*cos(theta)*cos(phi) + node->velocity[1]*cos(theta)*sin(phi) - node->velocity[2]*sin(theta);
			sphV[2] = -1.0f * node->velocity[0]*sin(phi) + node->velocity[1]*cos(phi);

			node->velocity[0] = sphV[0]*sin(theta)*cos(phi) + sphV[1]*cos(theta)*cos(phi) - sphV[2]*sin(phi);
			node->velocity[1] = sphV[0]*sin(theta)*sin(phi) + sphV[1]*cos(theta)*sin(phi) + sphV[2]*cos(phi);
			node->velocity[2] = sphV[0]*cos(theta) - sphV[1]*sin(theta);
        }
		else if( (ijk[2] == 0 || ijk[2] == decomp->nodeGlobal3DCounts[2]-1) ) {
			VariableCondition_ApplyToIndex( self->velocityBCs, node_lI, self );
			sphV[0] = node->velocity[0]*sin(theta)*cos(phi) + node->velocity[1]*sin(theta)*sin(phi) + node->velocity[2]*cos(theta);
			sphV[1] = node->velocity[0]*cos(theta)*cos(phi) + node->velocity[1]*cos(theta)*sin(phi) - node->velocity[2]*sin(theta);
			sphV[2] = -1.0f * node->velocity[0]*sin(phi) + node->velocity[1]*cos(phi);

			sphV[1] = 0.0f; /* free-slip BC. */

			node->velocity[0] = sphV[0]*sin(theta)*cos(phi) + sphV[1]*cos(theta)*cos(phi) - sphV[2]*sin(phi);
			node->velocity[1] = sphV[0]*sin(theta)*sin(phi) + sphV[1]*cos(theta)*sin(phi) + sphV[2]*cos(phi);
			node->velocity[2] = sphV[0]*cos(theta) - sphV[1]*sin(theta);
		}
		else
			VariableCondition_ApplyToIndex( self->velocityBCs, node_lI, self );
		}
	else {
		VariableCondition_ApplyToIndex( self->velocityBCs, node_lI, self );
	}
}


void getMaxTopoGrad( void* _context, Node_LocalIndex node_lI )
{
	/*!\fn void getMaxTopoGrad( void* _context, Node_LocalIndex node_lI )
	  \brief Computes the maximum gradient of topography.

	  This function computes the magnitude of topo gradient at each node, and then retrieve
	  its global maximum.

	  Functions related to diffusiong the surface topography depend on the array of neighbor
	  nodes. "buildNodeNeighbourTbl" should be "True" in an input file.

	  Naming convention for the FD cell to compute 2-D gradient of topography (y-coordinate)
	       X0  -- xR (right)
	       |
	       xF (front)
	*/

	Snac_Context* self = (Snac_Context*)_context;

	Mesh*					mesh = self->mesh;
	HexaMD*					decomp = (HexaMD*)mesh->layout->decomp;
	Coord					*X0, *xR, *xF;
	double					X0C[3], xRC[3], xFC[3];
	Node_GlobalIndex		node_gI = _MeshDecomp_Node_LocalToGlobal1D( decomp, node_lI );
	IJK						ijk;
	double					localTopoGrad = 0.0f;
	double					dydx,dydz;
	double					tmp;
	Index					neighborI;

	void getMaxTopoGrad_alongX( void* _context, Node_LocalIndex node_lI );
	void getMaxTopoGrad_alongZ( void* _context, Node_LocalIndex node_lI );

	RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );
	/* if not a top-surface node, don't do anything */
	if( self->timeStep <= 1)
		return;

	if( ijk[1] < decomp->nodeGlobal3DCounts[1]-1 )
		return;

	/* if on top, but not on the global edges, compute topo gradient at this node */

	/* Essentially 2-D case */
	if( decomp->nodeGlobal3DCounts[0] > 2 && decomp->nodeGlobal3DCounts[2] > 2 ) {
		if( (ijk[0] > 0 && ijk[0] < decomp->nodeGlobal3DCounts[0]-1) && (ijk[2] > 0 && ijk[2] < decomp->nodeGlobal3DCounts[2]-1) ) {
			X0 = Snac_NodeCoord_P( self, node_lI );

			if(self->spherical) {
				/* get the coordinates for xR */
				neighborI = mesh->nodeNeighbourTbl[node_lI][3];
				Journal_Firewall( (neighborI < mesh->nodeDomainCount), self->snacError, "Edge nodes should not be considered in computing topo gradient!!\n");
				xR = Snac_NodeCoord_P( self, neighborI );

				/* get the coordinates for xF */
				neighborI = mesh->nodeNeighbourTbl[node_lI][5];
				Journal_Firewall( (neighborI < mesh->nodeDomainCount), self->snacError, "Edge nodes should not be considered in computing topo gradient!!\n");
				xF = Snac_NodeCoord_P( self, neighborI );

				/* (0, 1, 2)=(x, y, z)=(theta, r, phi)=(colatitude, radius, azimuth)  */
				Cart2Spherical_Coord( X0, X0C );
				Cart2Spherical_Coord( xR, xRC );
				Cart2Spherical_Coord( xF, xFC );
				/* compute |grad(y)| and find its local maximum */
				dydx = (xFC[1]-X0C[1])/(xFC[0]-X0C[0]);
				dydz = (xRC[1]-X0C[1])/(xRC[2]-X0C[2])/sin(X0C[0]);
				localTopoGrad= sqrt( dydx*dydx + dydz*dydz )/R;
			}
			else {
				/* get the coordinates for xR */
				neighborI = mesh->nodeNeighbourTbl[node_lI][0];
				Journal_Firewall( (neighborI < mesh->nodeDomainCount), self->snacError, "Edge nodes should not be considered in computing topo gradient!!\n");
				xR = Snac_NodeCoord_P( self, neighborI );

				/* get the coordinates for xF */
				neighborI = mesh->nodeNeighbourTbl[node_lI][2];
				Journal_Firewall( (neighborI < mesh->nodeDomainCount), self->snacError, "Edge nodes should not be considered in computing topo gradient!!\n");
				xF = Snac_NodeCoord_P( self, neighborI );

				/* compute |grad(y)| and find its local maximum */
				dydx = ((*xR)[1]-(*X0)[1])/((*xR)[0]-(*X0)[0]);
				dydz = ((*xF)[1]-(*X0)[1])/((*xF)[2]-(*X0)[2]);
				localTopoGrad= sqrt( dydx*dydx + dydz*dydz );
			}
			self->topoGradMax = max( self->topoGradMax, localTopoGrad );
		}
	}
	else if( decomp->nodeGlobal3DCounts[0] == 2 && decomp->nodeGlobal3DCounts[2] > 2 )
		getMaxTopoGrad_alongZ( self, node_lI );
	else if( decomp->nodeGlobal3DCounts[2] == 2 && decomp->nodeGlobal3DCounts[0] > 2 )
		getMaxTopoGrad_alongX( self, node_lI );


	/* if computing local |grad(y)| done, find its global maximum */
	if( node_lI == mesh->nodeLocalCount ) {
		MPI_Barrier( self->communicator );
		MPI_Allreduce( &self->topoGradMax, &tmp, 1, MPI_DOUBLE, MPI_MAX, self->communicator );
		self->topoGradMax = tmp;
	}
}

void getMaxTopoGrad_alongZ( void* _context, Node_LocalIndex node_lI )
{
	Snac_Context* self = (Snac_Context*)_context;

	Mesh*					mesh = self->mesh;
	HexaMD*					decomp = (HexaMD*)mesh->layout->decomp;
	Coord					*X0, *xF;
	double					X0C[3], xFC[3];
	Node_GlobalIndex		node_gI = _MeshDecomp_Node_LocalToGlobal1D( decomp, node_lI );
	IJK						ijk;
	double					localTopoGrad = 0.0f;
	double					dydx,dydz;
	Index					neighborI;

	RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );

	if( ijk[2] > 0 && ijk[2] < decomp->nodeGlobal3DCounts[2]-1 ) { /* Essentially 1-D case */
			X0 = Snac_NodeCoord_P( self, node_lI );

			if(self->spherical) {
				/* get the coordinates for xF */
				neighborI = mesh->nodeNeighbourTbl[node_lI][5];
				Journal_Firewall( (neighborI < mesh->nodeDomainCount), self->snacError, "Edge nodes should not be considered in computing topo gradient!!\n");
				xF = Snac_NodeCoord_P( self, neighborI );

				/* (0, 1, 2)=(x, y, z)=(theta, r, phi)=(colatitude, radius, azimuth)  */
				Cart2Spherical_Coord( X0, X0C );
				Cart2Spherical_Coord( xF, xFC );
				/* compute |grad(y)| and find its local maximum */
				dydx = (xFC[1]-X0C[1])/(xFC[0]-X0C[0]);

				localTopoGrad= sqrt( dydx*dydx )/R;
			}
			else {
				/* get the coordinates for xF */
				neighborI = mesh->nodeNeighbourTbl[node_lI][2];
				Journal_Firewall( (neighborI < mesh->nodeDomainCount), self->snacError, "Edge nodes should not be considered in computing topo gradient!!\n");
				xF = Snac_NodeCoord_P( self, neighborI );

				/* compute |grad(y)| and find its local maximum */
				dydz = ((*xF)[1]-(*X0)[1])/((*xF)[2]-(*X0)[2]);

				localTopoGrad= sqrt( dydz*dydz );
			}
			self->topoGradMax = max( self->topoGradMax, localTopoGrad );
	}
}

void getMaxTopoGrad_alongX( void* _context, Node_LocalIndex node_lI )
{
	Snac_Context* self = (Snac_Context*)_context;

	Mesh*					mesh = self->mesh;
	HexaMD*					decomp = (HexaMD*)mesh->layout->decomp;
	Coord					*X0, *xR;
	double					X0C[3], xRC[3];
	Node_GlobalIndex		node_gI = _MeshDecomp_Node_LocalToGlobal1D( decomp, node_lI );
	IJK						ijk;
	double					localTopoGrad = 0.0f;
	double					dydx,dydz;
	Index					neighborI;

	RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );

	if( ijk[0] > 0 && ijk[0] < decomp->nodeGlobal3DCounts[0]-1 ) { /* Essentially 1-D case */
			X0 = Snac_NodeCoord_P( self, node_lI );

			if(self->spherical) {
				/* get the coordinates for xR */
				neighborI = mesh->nodeNeighbourTbl[node_lI][3];
				Journal_Firewall( (neighborI < mesh->nodeDomainCount), self->snacError, "Edge nodes should not be considered in computing topo gradient!!\n");
				xR = Snac_NodeCoord_P( self, neighborI );

				/* (0, 1, 2)=(x, y, z)=(theta, r, phi)=(colatitude, radius, azimuth)  */
				Cart2Spherical_Coord( X0, X0C );
				Cart2Spherical_Coord( xR, xRC );
				/* compute |grad(y)| and find its local maximum */
				dydz = (xRC[1]-X0C[1])/(xRC[2]-X0C[2])/sin(X0C[0]);

				localTopoGrad= sqrt( dydz*dydz )/R;
			}
			else {
				/* get the coordinates for xR */
				neighborI = mesh->nodeNeighbourTbl[node_lI][0];
				Journal_Firewall( (neighborI < mesh->nodeDomainCount), self->snacError, "Edge nodes should not be considered in computing topo gradient!!\n");
				xR = Snac_NodeCoord_P( self, neighborI );

				/* compute |grad(y)| and find its local maximum */
				dydx = ((*xR)[1]-(*X0)[1])/((*xR)[0]-(*X0)[0]);

				localTopoGrad= sqrt( dydx*dydx );
			}
			self->topoGradMax = max( self->topoGradMax, localTopoGrad );
	}
}


void Snac_UpdateNodeMomentum_DiffTopo( void* _context, Node_LocalIndex node_lI, Mass inertialMass, Force force )
{

	/*!\fn void topo_diff( void* _context, Node_LocalIndex node_lI, Mass inertialMass, Force force )
	  \brief Diffuses topography.

	  This function diffuses topography if the global maximum of topo gradient exceeds a criterion.
	  Naming convention for the FD cell to compute 2-D laplacian of topography (y-coordinate)

	  Functions related to diffusiong the surface topography depend on the array of neighbor
	  nodes. "buildNodeNeighbourTbl" should be "True" in an input file.

	                 xB (back, 5)
	                 |
	  (left, 3) xL --   X0  -- xR (right, 0)
	                 |
	                 xF (front, 2)
	*/

	Snac_Context* self = (Snac_Context*)_context;

	Mesh*					mesh = self->mesh;
	HexaMD*					decomp = (HexaMD*)mesh->layout->decomp;
	Coord					*X0, *xR, *xL, *xF, *xB;
	double					X0C[3], xRC[3], xLC[3], xFC[3], xBC[3];
	Node_GlobalIndex		node_gI = _MeshDecomp_Node_LocalToGlobal1D( decomp, node_lI );
	IJK						ijk;
	double					d2ydx2,d2ydz2;
	double					dydx1,dydx2,dydz1,dydz2;
	double					lapl_topo;
	Index					neighborI;
	Snac_Node *node = Snac_Node_At( self, node_lI );

	double diffTopo_alongX( void* _context, Node_LocalIndex node_lI );
	double diffTopo_alongZ( void* _context, Node_LocalIndex node_lI );

	RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );
	/* if not a top-surface node, don't do anything */
	if( self->timeStep <= 1)
		return;

	if( ijk[1] < decomp->nodeGlobal3DCounts[1]-1 || self->topoGradMax < self->topoGradCriterion )
		return;

	/* if on top, but not on the global edges, diffuse topo for this node */
	/* Essentially 2-D case */
	if( decomp->nodeGlobal3DCounts[0] > 2 && decomp->nodeGlobal3DCounts[2] > 2 ) {
		if( (ijk[0] > 0 && ijk[0] < decomp->nodeGlobal3DCounts[0]-1) && (ijk[2] > 0 && ijk[2] < decomp->nodeGlobal3DCounts[2]-1) ) {

			X0 = Snac_NodeCoord_P( self, node_lI );

			if(self->spherical) {
				/* get the coordinates for xR */
				neighborI = mesh->nodeNeighbourTbl[node_lI][3];
				Journal_Firewall( (neighborI < mesh->nodeDomainCount), self->snacError, "Edge nodes should not be considered in computing topo diffusion!!\n");
				xR = Snac_NodeCoord_P( self, neighborI );

				/* get the coordinates for xF */
				neighborI = mesh->nodeNeighbourTbl[node_lI][5];
				Journal_Firewall( (neighborI < mesh->nodeDomainCount), self->snacError, "Edge nodes should not be considered in computing topo diffusion!!\n");
				xF = Snac_NodeCoord_P( self, neighborI );

				/* get the coordinates for xL */
				neighborI = mesh->nodeNeighbourTbl[node_lI][0];
				Journal_Firewall( (neighborI < mesh->nodeDomainCount), self->snacError, "Edge nodes should not be considered in computing topo diffusion!!\n");
				xL = Snac_NodeCoord_P( self, neighborI );

				/* get the coordinates for xB */
				neighborI = mesh->nodeNeighbourTbl[node_lI][2];
				Journal_Firewall( (neighborI < mesh->nodeDomainCount), self->snacError, "Edge nodes should not be considered in computing topo diffusion!!\n");
				xB = Snac_NodeCoord_P( self, neighborI );

				/* (0, 1, 2)=(x, y, z)=(theta, r, phi)=(colatitude, radius, azimuth)  */
				Cart2Spherical_Coord( X0, X0C );
				Cart2Spherical_Coord( xR, xRC );
				Cart2Spherical_Coord( xF, xFC );
				Cart2Spherical_Coord( xL, xLC );
				Cart2Spherical_Coord( xB, xBC );
				/* compute laplacian(y) */
				dydx1 = (xFC[1]-X0C[1])/(xFC[0]-X0C[0]);
				dydx2 = (X0C[1]-xBC[1])/(X0C[0]-xBC[0]);
				d2ydx2 = (dydx1 - dydx2)/(0.5f*(xFC[0]-xBC[0]));
				d2ydx2 += cos(X0C[0])/sin(X0C[0])*(xFC[1]-X0C[1])/(xFC[0]-X0C[0]);

				dydz1 = (xRC[1]-X0C[1])/(xRC[2]-X0C[2]);
				dydz2 = (X0C[1]-xLC[1])/(X0C[2]-xLC[2]);
				d2ydz2 = (dydz1 - dydz2)/(0.5f*(xRC[2]-xLC[2]))/(sin(X0C[0])*sin(X0C[0]));

				lapl_topo = (d2ydx2 + d2ydz2)/(R*R);
				Journal_Firewall( (!isinf(lapl_topo)), self->snacError, "The computation of the laplacian of topo went wrong!!\n");
				/* 			if(self->rank==0) { */
				/* 				fprintf(stderr,"me=%d timeStep=%d node_lI=%d %d %d %d %d\n(%e %e %e) (%e %e %e) (%e %e %e) (%e %e %e) (%e %e %e)\n",self->rank,self->timeStep, */
				/* 						node_lI,mesh->nodeNeighbourTbl[node_lI][5],mesh->nodeNeighbourTbl[node_lI][2],mesh->nodeNeighbourTbl[node_lI][3],mesh->nodeNeighbourTbl[node_lI][0], */
				/* 						(*X0)[0],(*X0)[1],(*X0)[2],(*xB)[0],(*xB)[1],(*xB)[2],(*xF)[0],(*xF)[1],(*xF)[2],(*xL)[0],(*xL)[1],(*xL)[2],(*xR)[0],(*xR)[1],(*xR)[2]); */
				/* 				fprintf(stderr,"timeStep=%d (%e %e %e) (%e %e %e) (%e %e %e) (%e %e %e) (%e %e %e)\n",self->timeStep, */
				/* 						X0C[0],X0C[1],X0C[2], */
				/* 						xBC[0],xBC[1],xBC[2],xFC[0],xFC[1],xFC[2],xLC[0],xLC[1],xLC[2],xRC[0],xRC[1],xRC[2]); */
				/* 				fprintf(stderr,"timeStep=%d dh=%e (%e %e %e) (%e %e %e) dx=%e\n\n",self->timeStep,node->dh, */
				/* 						dydx1,dydx2,d2ydx2,dydz1,dydz2,d2ydz2,(X0C[0]-xRC[0])); */
				/* 			} */
			}
			else {
				/* get the coordinates for xR */
				neighborI = mesh->nodeNeighbourTbl[node_lI][0];
				Journal_Firewall( (neighborI < mesh->nodeDomainCount), self->snacError, "Edge nodes should not be considered in computing topo diffusion!!\n");
				xR = Snac_NodeCoord_P( self, neighborI );

				/* get the coordinates for xF */
				neighborI = mesh->nodeNeighbourTbl[node_lI][2];
				Journal_Firewall( (neighborI < mesh->nodeDomainCount), self->snacError, "Edge nodes should not be considered in computing topo diffusion!!\n");
				xF = Snac_NodeCoord_P( self, neighborI );

				/* get the coordinates for xL */
				neighborI = mesh->nodeNeighbourTbl[node_lI][3];
				Journal_Firewall( (neighborI < mesh->nodeDomainCount), self->snacError, "Edge nodes should not be considered in computing topo diffusion!!\n");
				xL = Snac_NodeCoord_P( self, neighborI );

				/* get the coordinates for xB */
				neighborI = mesh->nodeNeighbourTbl[node_lI][5];
				Journal_Firewall( (neighborI < mesh->nodeDomainCount), self->snacError, "Edge nodes should not be considered in computing topo diffusion!!\n");
				xB = Snac_NodeCoord_P( self, neighborI );

				/* compute laplacian(y) */
				dydx1 = ((*xR)[1]-(*X0)[1])/((*xR)[0]-(*X0)[0]);
				dydx2 = ((*X0)[1]-(*xL)[1])/((*X0)[0]-(*xL)[0]);
				d2ydx2 = (dydx1 - dydx2)/(0.5f*((*xR)[0]-(*xL)[0]));

				dydz1 = ((*xF)[1]-(*X0)[1])/((*xF)[2]-(*X0)[2]);
				dydz2 = ((*X0)[1]-(*xB)[1])/((*X0)[2]-(*xB)[2]);
				d2ydz2 = (dydz1 - dydz2)/(0.5f*((*xF)[2]-(*xB)[2]));

				lapl_topo = d2ydx2 + d2ydz2;
			}
		}
	}
	else if( decomp->nodeGlobal3DCounts[0] == 2 && decomp->nodeGlobal3DCounts[2] > 2 ) {
		lapl_topo = diffTopo_alongZ( self, node_lI );
	}
	else if( decomp->nodeGlobal3DCounts[2] == 2 && decomp->nodeGlobal3DCounts[0] > 2 ) {
		lapl_topo = diffTopo_alongX( self, node_lI );
	}

	/* compute diffused topo and add it to the current topo */
	node->dh = self->topo_kappa * self->dt * lapl_topo;
/* 	node->dh = self->topo_kappa * lapl_topo; */
	Journal_Firewall( (!isinf( node->dh )), self->snacError, "The computation of topo change went wrong!!\n");
}


double diffTopo_alongZ( void* _context, Node_LocalIndex node_lI )
{
	Snac_Context* self = (Snac_Context*)_context;

	Mesh*					mesh = self->mesh;
	HexaMD*					decomp = (HexaMD*)mesh->layout->decomp;
	Coord					*X0, *xF, *xB;
	double					X0C[3], xFC[3], xBC[3];
	Node_GlobalIndex		node_gI = _MeshDecomp_Node_LocalToGlobal1D( decomp, node_lI );
	IJK						ijk;
	double					d2ydx2,d2ydz2;
	double					dydx1,dydx2,dydz1,dydz2;
	double					lapl_topo;
	Index					neighborI;

	RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );

	if( ijk[2] > 0 && ijk[2] < decomp->nodeGlobal3DCounts[2]-1 ) {

		X0 = Snac_NodeCoord_P( self, node_lI );

		if(self->spherical) {
			/* get the coordinates for xF */
			neighborI = mesh->nodeNeighbourTbl[node_lI][5];
			Journal_Firewall( (neighborI < mesh->nodeDomainCount), self->snacError, "Edge nodes should not be considered in computing topo diffusion!!\n");
			xF = Snac_NodeCoord_P( self, neighborI );

			/* get the coordinates for xB */
			neighborI = mesh->nodeNeighbourTbl[node_lI][2];
			Journal_Firewall( (neighborI < mesh->nodeDomainCount), self->snacError, "Edge nodes should not be considered in computing topo diffusion!!\n");
			xB = Snac_NodeCoord_P( self, neighborI );

			/* (0, 1, 2)=(x, y, z)=(theta, r, phi)=(colatitude, radius, azimuth)  */
			Cart2Spherical_Coord( X0, X0C );
			Cart2Spherical_Coord( xF, xFC );
			Cart2Spherical_Coord( xB, xBC );
			/* compute laplacian(y) */
			dydx1 = (xFC[1]-X0C[1])/(xFC[0]-X0C[0]);
			dydx2 = (X0C[1]-xBC[1])/(X0C[0]-xBC[0]);
			d2ydx2 = (dydx1 - dydx2)/(0.5f*(xFC[0]-xBC[0]));
			d2ydx2 += cos(X0C[0])/sin(X0C[0])*(xFC[1]-X0C[1])/(xFC[0]-X0C[0]);

			lapl_topo = d2ydx2/(R*R);
			Journal_Firewall( (!isinf(lapl_topo)), self->snacError, "The computation of the laplacian of topo went wrong!!\n");
		}
		else {
			/* get the coordinates for xF */
			neighborI = mesh->nodeNeighbourTbl[node_lI][2];
			Journal_Firewall( (neighborI < mesh->nodeDomainCount), self->snacError, "Edge nodes should not be considered in computing topo diffusion!!\n");
			xF = Snac_NodeCoord_P( self, neighborI );

			/* get the coordinates for xB */
			neighborI = mesh->nodeNeighbourTbl[node_lI][5];
			Journal_Firewall( (neighborI < mesh->nodeDomainCount), self->snacError, "Edge nodes should not be considered in computing topo diffusion!!\n");
			xB = Snac_NodeCoord_P( self, neighborI );

			/* compute laplacian(y) */
			dydz1 = ((*xF)[1]-(*X0)[1])/((*xF)[2]-(*X0)[2]);
			dydz2 = ((*X0)[1]-(*xB)[1])/((*X0)[2]-(*xB)[2]);
			d2ydz2 = (dydz1 - dydz2)/(0.5f*((*xF)[2]-(*xB)[2]));

			lapl_topo = d2ydz2;
			Journal_Firewall( (!isinf(lapl_topo)), self->snacError, "The computation of the laplacian of topo went wrong!!\n");
		}
	}

	return lapl_topo;
}

double diffTopo_alongX( void* _context, Node_LocalIndex node_lI )
{
	Snac_Context* self = (Snac_Context*)_context;

	Mesh*					mesh = self->mesh;
	HexaMD*					decomp = (HexaMD*)mesh->layout->decomp;
	Coord					*X0, *xR, *xL;
	double					X0C[3], xRC[3], xLC[3];
	Node_GlobalIndex		node_gI = _MeshDecomp_Node_LocalToGlobal1D( decomp, node_lI );
	IJK						ijk;
	double					d2ydz2, dydz1, dydz2;
	double					lapl_topo = 0.0f;
	Index					neighborI;


	RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );

	if( ijk[0] > 0 && ijk[0] < decomp->nodeGlobal3DCounts[0]-1 ) {

		X0 = Snac_NodeCoord_P( self, node_lI );

		if(self->spherical) {
			/* get the coordinates for xR */
			neighborI = mesh->nodeNeighbourTbl[node_lI][3];
			Journal_Firewall( (neighborI < mesh->nodeDomainCount), self->snacError, "Edge nodes should not be considered in computing topo diffusion!!\n");
			xR = Snac_NodeCoord_P( self, neighborI );

			/* get the coordinates for xL */
			neighborI = mesh->nodeNeighbourTbl[node_lI][0];
			Journal_Firewall( (neighborI < mesh->nodeDomainCount), self->snacError, "Edge nodes should not be considered in computing topo diffusion!!\n");
			xL = Snac_NodeCoord_P( self, neighborI );

			/* (0, 1, 2)=(x, y, z)=(theta, r, phi)=(colatitude, radius, azimuth)  */
			Cart2Spherical_Coord( X0, X0C );
			Cart2Spherical_Coord( xR, xRC );
			Cart2Spherical_Coord( xL, xLC );
			/* compute laplacian(y) */
			dydz1 = (xRC[1]-X0C[1])/(xRC[2]-X0C[2]);
			dydz2 = (X0C[1]-xLC[1])/(X0C[2]-xLC[2]);
			d2ydz2 = (dydz1 - dydz2)/(0.5f*(xRC[2]-xLC[2]))/(sin(X0C[0])*sin(X0C[0]));

			lapl_topo = d2ydz2/(R*R);
			Journal_Firewall( (!isinf(lapl_topo)), self->snacError, "The computation of the laplacian of topo went wrong!!\n");
		}
		else {
			/* get the coordinates for xR */
			neighborI = mesh->nodeNeighbourTbl[node_lI][0];
			Journal_Firewall( (neighborI < mesh->nodeDomainCount), self->snacError, "Edge nodes should not be considered in computing topo diffusion!!\n");
			xR = Snac_NodeCoord_P( self, neighborI );

			/* get the coordinates for xL */
			neighborI = mesh->nodeNeighbourTbl[node_lI][3];
			Journal_Firewall( (neighborI < mesh->nodeDomainCount), self->snacError, "Edge nodes should not be considered in computing topo diffusion!!\n");
			xL = Snac_NodeCoord_P( self, neighborI );

			/* compute laplacian(y) */
			dydz1 = ((*xR)[1]-(*X0)[1])/((*xR)[0]-(*X0)[0]);
			dydz2 = ((*X0)[1]-(*xL)[1])/((*X0)[0]-(*xL)[0]);
			d2ydz2 = (dydz1 - dydz2)/(0.5f*((*xR)[0]-(*xL)[0]));

/* 			fprintf(stderr,"me=%d timeStep=%d node=%d dydz1=%e (%e %e %e %e) dydz2=%e (%e %e %e %e) d2ydz2=%e\n", */
/* 					self->rank,self->timeStep,node_lI,dydz1,(*xR)[1],(*X0)[1],(*xR)[0],(*X0)[0],dydz2,(*X0)[1],(*xL)[1],(*X0)[0],(*xL)[0],d2ydz2); */
			lapl_topo = d2ydz2;
			Journal_Firewall( (!isinf(lapl_topo)), self->snacError, "The computation of the laplacian of topo went wrong!!\n");
		}
	}

	return lapl_topo;
}

void Snac_UpdateNodeMomentum_UpdateInteriorTopo( void* _context, Node_LocalIndex node_lI, Mass inertialMass, Force force )
{
	/*!\fn void Snac_UpdateNodeMomentum_UpdateInteriorTopo( void* _context, Node_LocalIndex node_lI, Mass inertialMass, Force force )
	  \brief Update top-interior topo with dh.

	  This function simply adds dh to y coordinate of each top interior node.
	*/

	Snac_Context* self = (Snac_Context*)_context;

	Mesh*					mesh = self->mesh;
	HexaMD*					decomp = (HexaMD*)mesh->layout->decomp;
	Node_GlobalIndex		node_gI = _MeshDecomp_Node_LocalToGlobal1D( decomp, node_lI );
	IJK						ijk;
	Coord					*X0 = Snac_NodeCoord_P( self, node_lI );
	double                  X0C[3];
	Snac_Node *node = Snac_Node_At( self, node_lI );

	RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );

	if( self->timeStep <= 1)
		return;
	if( ijk[1] < decomp->nodeGlobal3DCounts[1]-1 || self->topoGradMax < self->topoGradCriterion )
		return;

	if( decomp->nodeGlobal3DCounts[0] > 2 && decomp->nodeGlobal3DCounts[2] > 2 ) {
		if( (ijk[0] > 0 && ijk[0] < decomp->nodeGlobal3DCounts[0]-1) && (ijk[2] > 0 && ijk[2] < decomp->nodeGlobal3DCounts[2]-1) ) {
			if(self->spherical) {
				Cart2Spherical_Coord( X0, X0C );
				X0C[1] += node->dh;
				(*X0)[0] = X0C[1]*sin(X0C[0])*cos(X0C[2]);
				(*X0)[1] = X0C[1]*sin(X0C[0])*sin(X0C[2]);
				(*X0)[2] = X0C[1]*cos(X0C[0]);
			}
			else {
				(*X0)[1] += node->dh;
			}
		}
	}
	else if( decomp->nodeGlobal3DCounts[0] == 2 && (ijk[2] > 0 && ijk[2] < decomp->nodeGlobal3DCounts[2]-1) ) {
		if(self->spherical) {
			Cart2Spherical_Coord( X0, X0C );
			X0C[1] += node->dh;
			(*X0)[0] = X0C[1]*sin(X0C[0])*cos(X0C[2]);
			(*X0)[1] = X0C[1]*sin(X0C[0])*sin(X0C[2]);
			(*X0)[2] = X0C[1]*cos(X0C[0]);
		}
		else {
			(*X0)[1] += node->dh;
		}
	}
	else if( decomp->nodeGlobal3DCounts[2] == 2 && (ijk[0] > 0 && ijk[0] < decomp->nodeGlobal3DCounts[0]-1) ) {
		if(self->spherical) {
			Cart2Spherical_Coord( X0, X0C );
			/* 			if( self->timeStep % 1000 == 0 && self->rank==4) */
			/* 				fprintf(stderr,"me=%d node_lI=%d Before X0C[1]=%e\t",self->rank,node_lI,X0C[1]); */
			X0C[1] += node->dh;
			/* 			if( self->timeStep % 1000 == 0 && self->rank==4 ) */
			/* 				fprintf(stderr,"dh = %e  After X0C[1]=%e\n",node->dh,X0C[1]); */
			(*X0)[0] = X0C[1]*sin(X0C[0])*cos(X0C[2]);
			(*X0)[1] = X0C[1]*sin(X0C[0])*sin(X0C[2]);
			(*X0)[2] = X0C[1]*cos(X0C[0]);
		}
		else {
			(*X0)[1] += node->dh;
		}
	}
}

void Snac_UpdateNodeMomentum_AdjustEdgeTopo( void* _context, Node_LocalIndex node_lI, Mass inertialMass, Force force )
{

	/*!\fn void Snac_UpdateNodeMomentum_AdjustEdgeTopo( void* _context, Node_LocalIndex node_lI, Mass inertialMass, Force force )
	  \brief Adjust topography at edge nodes.

	  This function copy topography from next-to-edge nodes to edge nodes. This operation is necessary because the FD cell is not
	  defined on these edge nodes.

	  Functions related to diffusiong the surface topography depend on the array of neighbor
	  nodes. "buildNodeNeighbourTbl" should be "True" in an input file.
	*/

	Snac_Context* self = (Snac_Context*)_context;

	Mesh*					mesh = self->mesh;
	HexaMD*					decomp = (HexaMD*)mesh->layout->decomp;
	Coord					*X0, *xN1 = NULL, *xN2 = NULL;
	double                  X0C[3], xN1C[3], xN2C[3];
	Node_GlobalIndex		node_gI = _MeshDecomp_Node_LocalToGlobal1D( decomp, node_lI );
	IJK						ijk;
	Index					neighborI;

	RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );
	/* if not a top-surface node, don't do anything */
	if( self->timeStep <= 1)
		return;

	if( ijk[1] < decomp->nodeGlobal3DCounts[1]-1 || self->topoGradMax < self->topoGradCriterion  )
		return;

	if( decomp->nodeGlobal3DCounts[0] > 2 && decomp->nodeGlobal3DCounts[2] > 2 ) {
		/* if on top and on the global edges, adjust topo for this node */
		/* get the coordinates of the neighbour. */
		if( ijk[0] == 0 ) {
			neighborI = mesh->nodeNeighbourTbl[node_lI][0];
			Journal_Firewall( (neighborI < mesh->nodeDomainCount), self->snacError, "%d: (1) Updating Edge node topo: Check the total node number along x axis!!\n", self->rank);
			xN1 = Snac_NodeCoord_P( self, neighborI );
		}
		else if( ijk[0] == decomp->nodeGlobal3DCounts[0]-1 ) {
			neighborI = mesh->nodeNeighbourTbl[node_lI][3];
			Journal_Firewall( (neighborI < mesh->nodeDomainCount), self->snacError, "%d: (2) Updating Edge node topo: Check the total node number along x axis!!\n", self->rank);
			xN1 = Snac_NodeCoord_P( self, neighborI );
		}

		if( ijk[2] == 0 ) {
			neighborI = mesh->nodeNeighbourTbl[node_lI][2];
			Journal_Firewall( (neighborI < mesh->nodeDomainCount), self->snacError, "%d: (3) Updating Edge node topo: Check the total node number along x axis!!\n", self->rank);
			xN2 = Snac_NodeCoord_P( self, neighborI );
		}
		else if( ijk[2] == decomp->nodeGlobal3DCounts[2]-1 ) {
			neighborI = mesh->nodeNeighbourTbl[node_lI][5];
			Journal_Firewall( (neighborI < mesh->nodeDomainCount), self->snacError, "%d: (4) Updating Edge node topo: Check the total node number along x axis!!\n", self->rank);
			xN2 = Snac_NodeCoord_P( self, neighborI );
		}

		X0 = Snac_NodeCoord_P( self, node_lI );
		if( xN1 != NULL && xN2 == NULL) {
			if(self->spherical) {
				Cart2Spherical_Coord( xN1, xN1C );
				Cart2Spherical_Coord( X0, X0C );
				X0C[1] = xN1C[1];
				(*X0)[0] = X0C[1]*sin(X0C[0])*cos(X0C[2]);
				(*X0)[1] = X0C[1]*sin(X0C[0])*sin(X0C[2]);
				(*X0)[2] = X0C[1]*cos(X0C[0]);
			}
			else
				(*X0)[1] = (*xN1)[1];
		}
		else if( xN1 == NULL && xN2 != NULL) {
			if(self->spherical) {
				Cart2Spherical_Coord( xN2, xN2C );
				Cart2Spherical_Coord( X0, X0C );
				X0C[1] = xN2C[1];
				(*X0)[0] = X0C[1]*sin(X0C[0])*cos(X0C[2]);
				(*X0)[1] = X0C[1]*sin(X0C[0])*sin(X0C[2]);
				(*X0)[2] = X0C[1]*cos(X0C[0]);
			}
			else
				(*X0)[1] = (*xN2)[1];
		}
		else if( xN1 != NULL && xN2 != NULL ) {
			if(self->spherical) {
				Cart2Spherical_Coord( xN1, xN1C );
				Cart2Spherical_Coord( xN2, xN2C );
				Cart2Spherical_Coord( X0, X0C );
				X0C[1] = 0.5f * ( xN1C[1] + xN2C[1] );
				(*X0)[0] = X0C[1]*sin(X0C[0])*cos(X0C[2]);
				(*X0)[1] = X0C[1]*sin(X0C[0])*sin(X0C[2]);
				(*X0)[2] = X0C[1]*cos(X0C[0]);
			}
			else
				(*X0)[1] = 0.5f * ( (*xN1)[1] + (*xN2)[1] );
		}
	}
	else if( decomp->nodeGlobal3DCounts[0] == 2 && decomp->nodeGlobal3DCounts[2] > 2 ) {
		if( ijk[2] == 0 )
			neighborI = mesh->nodeNeighbourTbl[node_lI][2];
		else if( ijk[2] == decomp->nodeGlobal3DCounts[2]-1 )
			neighborI = mesh->nodeNeighbourTbl[node_lI][5];
		else
			return;

		Journal_Firewall( (neighborI < mesh->nodeDomainCount), self->snacError, "%d: (5) Updating Edge node topo: Check the total node number along x axis!!\n", self->rank);

		xN2 = Snac_NodeCoord_P( self, neighborI );
		X0 = Snac_NodeCoord_P( self, node_lI );
		if(self->spherical) {
			Cart2Spherical_Coord( xN2, xN2C );
			Cart2Spherical_Coord( X0, X0C );
			X0C[1] = xN2C[1];
			(*X0)[0] = X0C[1]*sin(X0C[0])*cos(X0C[2]);
			(*X0)[1] = X0C[1]*sin(X0C[0])*sin(X0C[2]);
			(*X0)[2] = X0C[1]*cos(X0C[0]);
		}
		else
			(*X0)[1] = (*xN2)[1];
	}
	else if( decomp->nodeGlobal3DCounts[0] > 2 && decomp->nodeGlobal3DCounts[2] == 2 ) {
		if( ijk[0] == 0 )
			neighborI = mesh->nodeNeighbourTbl[node_lI][0];
		else if( ijk[0] == decomp->nodeGlobal3DCounts[0]-1 )
			neighborI = mesh->nodeNeighbourTbl[node_lI][3];
		else
			return;

		Journal_Firewall( (neighborI < mesh->nodeDomainCount), self->snacError, "%d: (6) Updating Edge node topo: Check the total node number along x axis!!\n", self->rank);

		xN1 = Snac_NodeCoord_P( self, neighborI );
		X0 = Snac_NodeCoord_P( self, node_lI );
		if(self->spherical) {
			Cart2Spherical_Coord( xN1, xN1C );
			Cart2Spherical_Coord( X0, X0C );
			X0C[1] = xN1C[1];
			(*X0)[0] = X0C[1]*sin(X0C[0])*cos(X0C[2]);
			(*X0)[1] = X0C[1]*sin(X0C[0])*sin(X0C[2]);
			(*X0)[2] = X0C[1]*cos(X0C[0]);
		}
		else
			(*X0)[1] = (*xN1)[1];
	}
}

void Cart2Spherical_Coord( Coord *X, double CX[] )
{

	/* (0, 1, 2)=(x, y, z)=(theta, r, phi)=(colatitude, radius, azimuth)  */
	CX[1] = sqrt((*X)[0]*(*X)[0] + (*X)[1]*(*X)[1] + (*X)[2]*(*X)[2]);
	CX[0] = acos((*X)[2]/CX[1]);
	CX[2] = atan2((*X)[1],(*X)[0]);

}
