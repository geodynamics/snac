// /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// **
// ** Copyright (C), 2003,
// **	Steve Quenette, 110 Victoria Street, Melbourne, Victoria, 3053, Australia.
// **	Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
// **	University of Texas, 1 University Station, Austin, Texas, 78712, USA.
// **
// ** Authors:
// **	Eh Tan, ?, Caltech (tan2@gps.caltech.edu)
// **	Pururav Thoutireddy, ?, Caltech (puru@cacr.caltech.edu)
// **	Eun-seo Choi, ?, Caltech (ces74@gps.caltech.edu)
// **	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
// **	Stevan M. Quenette, Visitor in Geophysics, Caltech.
// **	Luc Lavier, Research Scientist, The University of Texas. (luc@utig.ug.utexas.edu)
// **	Luc Lavier, Research Scientist, Caltech.
// **
// ** This program is free software; you can redistribute it and/or modify it
// ** under the terms of the GNU General Public License as published by the
// ** Free Software Foundation; either version 2, or (at your option) any
// ** later version.
// **
// ** This program is distributed in the hope that it will be useful,
// ** but WITHOUT ANY WARRANTY; without even the implied warranty of
// ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// ** GNU General Public License for more details.
// **
// ** You should have received a copy of the GNU General Public License
// ** along with this program; if not, write to the Free Software
// ** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// **
// ** $Id: SnacInterpolator.cc 1351 2004-05-03 16:55:44Z EunseoChoi $
// **
// **~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <assert.h>
#include <vector>
#include <algorithm>
extern "C" {
	#include <mpi.h>
	#include <StGermain/StGermain.h>
	#include <StGermain/FD/FD.h>
	#include <Snac/Snac.h>
	/* Required module */
	#include "Snac/Exchanger/Exchanger.h"
	#include "Snac/Temperature/Temperature.h"
	#include "Snac/Temperature/Register.h"
}
#include "journal/debug.h"
#include "journal/firewall.h"
#include "global_bbox.h"
#include "Exchanger/BoundedBox.h"
#include "Exchanger/BoundedMesh.h"
#include "SnacInterpolator.h"

using Exchanger::Array2D;
using Exchanger::BoundedBox;
using Exchanger::BoundedMesh;
using Exchanger::DIM;
using Exchanger::NODES_PER_ELEMENT;
using Exchanger::STRESS_DIM;

SnacInterpolator::SnacInterpolator( const BoundedMesh& boundedMesh, const void* _context, Array2D<int,1>& meshNode ) {

	journal::debug_t debug("SnacInterpolator");

	init( boundedMesh, _context, meshNode );

	debug << "size of elem: " << elem_.size() << "size of shape_: "<< shape_.size() << journal::endl;

	selfTest( boundedMesh, _context, meshNode );

	elem_.print( "snac_elem" );
	tetra_.print( "snac_tetra" );
	shape_.print( "shape" );
}


SnacInterpolator::~SnacInterpolator()
{}


void SnacInterpolator::interpolateV( Array2D<double,Exchanger::DIM>& target, const void* _context ) const {
	Snac_Context* context = (Snac_Context*)_context;

	target.assign( size(), 0 );

	for( int i = 0; i < size(); i++ ) {
		Element_LocalIndex	element_lI = elem_[0][i];
		Element_NodeIndex	elementNodeCount = context->mesh->elementNodeCountTbl[element_lI];

		for( unsigned int elementNode_I = 0; elementNode_I < elementNodeCount; elementNode_I++ ) {
			Node_LocalIndex	node_lI = context->mesh->elementNodeTbl[element_lI][elementNode_I];

			for( int d = 0; d < Exchanger::DIM; d++ ) {
				target[d][i] += shape_[elementNode_I][i] * Snac_Node_At( context, node_lI )->velocity[d];
			}
		}
	}
}


void SnacInterpolator::interpolateT( Array2D<double,1>& target, const void* _context ) const {
	journal::debug_t debug( "SnacInterpolator" );
	debug << journal::at( __HERE__ ) << "SnacInterpolator::interpolateT" << journal::endl;

	Snac_Context* context = (Snac_Context*)_context;

	target.assign( size(), 0 );

	for( int i = 0; i < size(); i++ ) {
		Element_LocalIndex	element_lI = elem_[0][i];
		Element_NodeIndex	elementNodeCount = context->mesh->elementNodeCountTbl[element_lI];

// 		debug<<"Before interpolation: i= "<<i<<" target= "<<target[0][i]<<' '
// 		     <<"element_lI = "<<element_lI<<' '<<"NodeCount = "<<elementNodeCount<<journal::endl;

		for( unsigned int elementNode_I = 0; elementNode_I < elementNodeCount; elementNode_I++ ) {
			Node_LocalIndex	node_lI = context->mesh->elementNodeTbl[element_lI][elementNode_I];
			Snac_Node*              node = (Snac_Node*)Snac_Node_At( context, node_lI );
			SnacTemperature_Node*	nodeExt =
				(SnacTemperature_Node*)ExtensionManager_Get(
								     context->mesh->nodeExtensionMgr,
								     node,
								     SnacTemperature_NodeHandle );
			target[0][i] += shape_[elementNode_I][i] * nodeExt->temperature;
			// 			debug<<"Interpolating: i= "<<i<<" elementNode_I = "<<elementNode_I<<' '
// 			     <<"node_lI = "<<node_lI<<' '
// 			     <<"shape = "<<shape_[elementNode_I][i]<<' '<<"T = "<<nodeExt->temperature
// 			     <<journal::endl;
		}
		target[0][i] += 273.25; // to make the unit Kelvin
// 		debug<<"After interpolation: i= "<<i<<" target= "<<target[0][i]<<journal::endl;
	}
}


void SnacInterpolator::interpolateS( Array2D<double,STRESS_DIM>& target, const void* _context ) const {
	journal::debug_t debug( "SnacInterpolator" );
	debug << journal::at( __HERE__ ) << "SnacInterpolator::interpolateS" << journal::endl;

	double Pressure = 0.0f;

	Snac_Context* context = (Snac_Context*)_context;

	for( int i = 0; i < size(); i++ ) {
        for( int j = 0; j < STRESS_DIM; j++)
            target[j][i] = 0.0;

		Element_LocalIndex	element_lI = elem_[0][i];
        Snac_Element* element = Snac_Element_At( context, element_lI );
        for(int tetra_I=0; tetra_I < Tetrahedra_Count; tetra_I++) {
            Pressure = ( element->tetra[tetra_I].stress[0][0] + element->tetra[tetra_I].stress[1][1] + element->tetra[tetra_I].stress[2][2] ) / 3.0f;
            target[0][i] += element->tetra[tetra_I].stress[0][0] - Pressure;
            target[1][i] += element->tetra[tetra_I].stress[1][1] - Pressure;
            target[2][i] += element->tetra[tetra_I].stress[2][2] - Pressure;
            target[3][i] += element->tetra[tetra_I].stress[0][1];
            target[4][i] += element->tetra[tetra_I].stress[0][2];
            target[5][i] += element->tetra[tetra_I].stress[1][2];
        }
        for( int j = 0; j < STRESS_DIM; j++)
            target[j][i] /= Tetrahedra_Count;
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// private functions


// vertices of five sub-tetrahedra
const int nsub[] = {
	0, 2, 3, 7,
	0, 1, 2, 5,
	4, 7, 5, 0,
	5, 7, 6, 2,
	5, 7, 2, 0
};


void SnacInterpolator::findMaxGridSpacing( const void* _context ) {
	Snac_Context* context = (Snac_Context*)_context;

	xtol = ytol = ztol = 0;
	for( Element_LocalIndex element_lI = 0; element_lI < context->mesh->elementLocalCount; element_lI++ ) {
		Element_NodeIndex	elementNodeCount = context->mesh->elementNodeCountTbl[element_lI];
		if( elementNodeCount != (unsigned)NODES_PER_ELEMENT ) {
			journal::firewall_t firewall( "SnacInterpolator" );
			firewall << journal::at( __HERE__ ) << "elementNodeCount != " << NODES_PER_ELEMENT
				<< ", element_lI = " << element_lI << journal::endl;
		}

		// view the node as an advectable... c++ is picker than c about this. TODO: fix.
		double* node0 = Mesh_CoordAt( context->mesh, context->mesh->elementNodeTbl[element_lI][0] );
		double* node1 = Mesh_CoordAt( context->mesh, context->mesh->elementNodeTbl[element_lI][1] );
		double* node2 = Mesh_CoordAt( context->mesh, context->mesh->elementNodeTbl[element_lI][2] );
		double* node4 = Mesh_CoordAt( context->mesh, context->mesh->elementNodeTbl[element_lI][4] );

		xtol = std::max( xtol, fabs( node1[0] - node0[0] ) );
		ytol = std::max( ytol, fabs( node2[1] - node0[1] ) );
		ztol = std::max( ztol, fabs( node4[2] - node0[2] ) );
	}

	journal::info_t info( "SnacInterpolator" );
	info << journal::at( __HERE__ ) << "Max grid spacing: " << xtol << " " << ytol << " " << ztol << journal::endl;
}


void SnacInterpolator::init( const BoundedMesh& boundedMesh, const void* _context, Array2D<int,1>& meshNode ) {

	journal::debug_t debug("SnacInterpolator");

	Snac_Context* context = (Snac_Context*)_context;

	double xt[Exchanger::DIM], xc[Exchanger::DIM*NODES_PER_ELEMENT], x1[Exchanger::DIM], x2[Exchanger::DIM], x3[Exchanger::DIM], x4[Exchanger::DIM];

	BoundedBox elembbox(Exchanger::DIM);

	int *ind;

	ind = new int[boundedMesh.size()];

	for(int i=0; i<boundedMesh.size(); i++) ind[i]=0;

	findMaxGridSpacing( context );

	elem_.reserve( boundedMesh.size() );
	tetra_.reserve( boundedMesh.size() );
	shape_.reserve( boundedMesh.size() );

	for( Element_LocalIndex element_lI = 0; element_lI < context->mesh->elementLocalCount; element_lI++ ) {
		Element_NodeIndex	elementNodeCount = context->mesh->elementNodeCountTbl[element_lI];
		if( elementNodeCount != (unsigned)NODES_PER_ELEMENT ) {
			journal::firewall_t firewall( "SnacInterpolator" );
			firewall << journal::at( __HERE__ ) << "elementNodeCount != " << NODES_PER_ELEMENT <<
				", element_lI = " << element_lI << journal::endl;
		}

		for( Element_NodeIndex j = 0; j < elementNodeCount; j++ ) {
			for( int k = 0; k < Exchanger::DIM; k++ ) {
				xc[j*Exchanger::DIM+k] = Mesh_CoordAt( context->mesh, context->mesh->elementNodeTbl[element_lI][j] )[k];
			}
		}

		if( !isCandidate( xc, boundedMesh.bbox() ) ) continue;

		for(int d=0; d<Exchanger::DIM; ++d) {
			elembbox[0][d] = std::numeric_limits<double>::max();
			elembbox[1][d] = std::numeric_limits<double>::min();
		}

		for(unsigned int n=0; n<elementNodeCount; ++n)
			for(int d=0; d < Exchanger::DIM; ++d) {
				elembbox[0][d] = std::min(elembbox[0][d], xc[n*Exchanger::DIM+d]);
				elembbox[1][d] = std::max(elembbox[1][d], xc[n*Exchanger::DIM+d]);
			}

		for( int i = 0; i < boundedMesh.size(); i++ ) {

			if(ind[i]) continue;

			for( int j = 0; j < Exchanger::DIM; j++ ) {
				xt[j] = boundedMesh.X( j, i );
			}

			//   if(!isInside(xt, elembbox))continue;
			if(xt[0] < elembbox[0][0] || xt[0] > elembbox[1][0] ||
			   xt[1] < elembbox[0][1] || xt[1] > elembbox[1][1] ||
			   xt[2] < elembbox[0][2] || xt[2] > elembbox[1][2]) continue;


			bool found = false;
			// loop over 5 sub tets in a brick element
			for( int k = 0; k < 5; k++ ) {
				for( int m = 0; m < Exchanger::DIM; m++ ) {
					x1[m] = xc[nsub[k*4]*Exchanger::DIM+m];
					x2[m] = xc[nsub[k*4+1]*Exchanger::DIM+m];
					x3[m] = xc[nsub[k*4+2]*Exchanger::DIM+m];
					x4[m] = xc[nsub[k*4+3]*Exchanger::DIM+m];
				}

				double dett, det[4];
				dett = TetrahedronVolume( x1, x2, x3, x4 );
				det[0] = TetrahedronVolume( x2, x4, x3, xt );
				det[1] = TetrahedronVolume( x3, x4, x1, xt );
				det[2] = TetrahedronVolume( x1, x4, x2, xt );
				det[3] = TetrahedronVolume( x1, x2, x3, xt );
				assert(dett != 0.0);

				if( dett < 0 ) {
					journal::firewall_t firewall( "SnacInterpolator" );
					firewall << journal::at( __HERE__ ) << "Determinant evaluation is wrong" << journal::newline
							 << " node " << i << " " << xt[0] << " " << xt[1] << " " << xt[2]
							 << journal::newline;

					for( int j = 0; j < NODES_PER_ELEMENT; j++ ) {
						firewall << xc[j*Exchanger::DIM] << " " << xc[j*Exchanger::DIM+1] << " " << xc[j*Exchanger::DIM+2]
								 << journal::newline;
					}
					firewall << journal::endl;
				}

				// found if all det are greated than zero
				found = ( det[0] > -1.e-10 && det[1] > -1.e-10 && det[2] > -1.e-10 && det[3] > -1.e-10 );

				if( found ) {
					debug <<journal::newline<<"i = "<<i<<' '<<"xt = "<<xt[0]<<' '<<xt[1]<<' '<<xt[2]<<journal::endl;
					ind[i]=1;

					meshNode.push_back( i );
					appendFoundElement( element_lI, k, det, dett );
					break;
				}
			}
	    }

	}

	elem_.shrink();
	tetra_.shrink();
	shape_.shrink();

	delete []ind;
}


bool SnacInterpolator::isCandidate( const double* xc, const BoundedBox& bbox ) const {
	std::vector<double> x( Exchanger::DIM );
	for( int j = 0; j < NODES_PER_ELEMENT; j++ ) {
		for( int k = 0; k < Exchanger::DIM; k++ ) {
			x[k] = xc[j*Exchanger::DIM+k];
		}
		if( isInside( x, bbox ) ) return true;
	}
	return false;
}


double SnacInterpolator::TetrahedronVolume( double *x1, double *x2,  double *x3, double *x4 ) const {
	double vol;

	// xx[0] = x2;  xx[1] = x3;  xx[2] = x4;
	vol = det3_sub( x2, x3, x4 );

	// xx[0] = x1;  xx[1] = x3;  xx[2] = x4;
	vol -= det3_sub( x1, x3, x4 );

	// xx[0] = x1;  xx[1] = x2;  xx[2] = x4;
	vol += det3_sub( x1, x2, x4 );

	// xx[0] = x1;  xx[1] = x2;  xx[2] = x3;
	vol -= det3_sub( x1, x2, x3 );

	return (vol / 6.0f);
}


double SnacInterpolator::det3_sub( double *x1, double *x2, double *x3 ) const {
	return ( x1[0] * ( x2[1] * x3[2] - x3[1] * x2[2] ) - x1[1] * ( x2[0] * x3[2] - x3[0] * x2[2] )
		+ x1[2] * ( x2[0] * x3[1] - x3[0] * x2[1] ) );
}


void SnacInterpolator::appendFoundElement( int el, int ntetra, const double* det, double dett ) {

	journal::debug_t debug("SnacInterpolator");
	std::vector<double> tmp( NODES_PER_ELEMENT, 0 );

	tmp[nsub[ntetra*4]] = det[0]/dett;
	tmp[nsub[ntetra*4+1]] = det[1]/dett;
	tmp[nsub[ntetra*4+2]] = det[2]/dett;
	tmp[nsub[ntetra*4+3]] = det[3]/dett;

	debug << "adding shape function: "<<tmp[0]<<' '<<tmp[1]<<' '<<tmp[2]<<' '<<tmp[3]<<' '<<tmp[4]<<' '<<tmp[5]<<' '<<tmp[6]<<' '<<tmp[7]<<journal::endl;

	shape_.push_back( tmp );
	elem_.push_back( el );
	tetra_.push_back( ntetra );
}


void SnacInterpolator::selfTest( const BoundedMesh& boundedMesh, const void* _context, const Array2D<int,1>& meshNode) const {

	journal::debug_t debug("SnacInterpolator");
	Snac_Context* context = (Snac_Context*)_context;

	double xc[Exchanger::DIM*NODES_PER_ELEMENT], xi[Exchanger::DIM], xt[Exchanger::DIM];

	for( int i = 0; i < size(); i++ ) {
		for( int j = 0; j < Exchanger::DIM; j++ ) {
			xt[j] = boundedMesh.X( j, meshNode[0][i] );
		}
		double refnorm = sqrt( xt[0]*xt[0] + xt[1]*xt[1] + xt[2]*xt[2] );

		Element_LocalIndex	element_lI = elem_[0][i];

		for( int j = 0; j < NODES_PER_ELEMENT; j++ ) {
			for( int k = 0; k < Exchanger::DIM; k++ ) {
				xc[j*Exchanger::DIM+k] = Mesh_CoordAt( context->mesh, context->mesh->elementNodeTbl[element_lI][j] )[k];
			}
		}

		for( int k = 0; k < Exchanger::DIM; k++ ) {
			xi[k] = 0.0;
		}

		debug <<journal::newline<< "in selfTest    i = " << i <<' '
		      << xt[0] << " " << xt[1] << " " << xt[2] << " "
		      << shape_[0][i] <<' '<< shape_[1][i] <<' '<< shape_[2][i] <<' '<< shape_[3][i] <<' '<< shape_[4][i] <<' '<< shape_[5][i] <<' '<< shape_[6][i] <<' '<< shape_[7][i] << journal::newline<<journal::endl;

		for( int k = 0; k < Exchanger::DIM; k++  ) {
			for( int j = 0; j < NODES_PER_ELEMENT; j++ ) {
				xi[k] += xc[j*Exchanger::DIM+k] * shape_[j][i];
			}
		}

		double norm = 0.0;
		for( int k = 0; k < Exchanger::DIM; k++ ) {
			norm += ( xt[k] - xi[k] ) * ( xt[k] - xi[k] );
		}

		if( norm > 1.e-10*refnorm ) {
			double tshape = 0.0;
			for( int j = 0; j < NODES_PER_ELEMENT; j++ ) {
				tshape += shape_[j][i];
			}

			journal::firewall_t firewall( "SnacInterpolator" );
			firewall << journal::at( __HERE__ ) << "node #" << i << " tshape = " << tshape << journal::newline
					 << xi[0] << " " << xt[0] << " " << xi[1] << " " << xt[1] << " " << xi[2] << " " << xt[2] << journal::newline
					 << (xt[0]-xi[0]) << " " << (xt[1]-xi[1]) << " " << (xt[2]-xi[2]) << " "
					 << " norm = " << norm << " " << " refnorm= " << refnorm << journal::newline 
					 << "elem interpolation functions are wrong" << journal::endl;
		}
	}
}
