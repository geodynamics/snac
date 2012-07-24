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
*/
/** \file
** Role:
**	Snac exchanger properties extensions to the mesh construct.
**
** Assumptions:
**
** Comments:
**
** $Id: Mesh.h 3173 2005-11-21 23:47:09Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __SnacRemesher_Mesh_h__
#define __SnacRemesher_Mesh_h__

	typedef enum {
		SnacRemesher_Spherical, 
		SnacRemesher_Cartesian, 
		SnacRemesher_MeshTypeMax
	} SnacRemesher_MeshType;

	struct _SnacRemesher_ElementBarcord {
		double				L[4];
		Element_DomainIndex	elnum;
		Tetrahedra_Index	tetnum;
	};	
	typedef struct _SnacRemesher_ElementBarcord SnacRemesher_ElementBarcord;

	struct _SnacRemesher_ElementBarcoef {
		/* Each of 5 tet has 4 shape functions (i.e., 4 nodes), 
		   each of which needs 4 coefficients. */
		double				coef[5][4][4]; 
	};	
	typedef struct _SnacRemesher_ElementBarcoef SnacRemesher_ElementBarcoef;
	
	/* Mesh Information */
	struct _SnacRemesher_Mesh {
		SnacRemesher_MeshType	meshType;
		
		/* A placeholder for new coords, and stores static node coords. */
		Coord*		newNodeCoords;
		Snac_Node*	newNodes;
		Coord*		oldBarycenters;
		Coord*		newBarycenters;
		SnacRemesher_ElementBarcord*	barcord;	
		SnacRemesher_ElementBarcoef*	barcoef;	
		unsigned 	local_range_min[3];	
		unsigned 	local_range_max[3];	

		SnacRemesher_Element*   newElements;
		//Snac_Element*	newElements;
		Element_DomainIndex*	orderedToDomain;
		
		/* If during the remesh nodes are found outside the old mesh they will be stored here. */
		unsigned			nExternalNodes;
		Node_LocalIndex*	externalNodes;
		unsigned			nExternalElements;
		Element_LocalIndex*	externalElements;
		
		/* Used for interpolating top/bottom internal nodes. */
		unsigned			nTopInternNodes;
		Node_LocalIndex*	topInternToLocal;
		unsigned			nBotInternNodes;
		Node_LocalIndex*	botInternToLocal;
		
		unsigned			nTopTriNodes;
		Node_DomainIndex*	topTriToDomain;
		unsigned			nBotTriNodes;
		Node_DomainIndex*	botTriToDomain;
		
		/* Used for interpolating bulk nodes. */
		unsigned			nYLines;
		unsigned*			yLineLTerm;
		unsigned*			yLineUTerm;
		unsigned*			yLineNYInds;
		unsigned**		yLineYInds;
		Node_LocalIndex**	yLineLocalInds;
		
		/* The sync class provides remote terminals for interpolating bulk nodes. */
		SnacSync*	sync;
	};

	/* Print the contents of a mesh */
	void SnacRemesher_Mesh_Print( void* mesh, Stream* stream );
	

#endif /* __SnacRemesher_Mesh_h__ */

