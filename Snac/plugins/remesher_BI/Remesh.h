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
**
** Assumptions:
**
** Comments:
**
** $Id: Remesh.h 3250 2006-10-23 06:15:18Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __SnacRemesher_Remesh_h__
#define __SnacRemesher_Remesh_h__

	void _SnacRemesher_Remesh( void* _context, void* data );

/*	void _SnacRemesher_Sync( void* _context ); */
	void _SnacRemesher_NewCoords( void* _context );

	void _SnacRemesher_InterpolateNodes( void* _context );
	void _SnacRemesher_InterpolateNode( void* context,
					    unsigned nodeInd, unsigned elementInd, unsigned tetInd, 
					    unsigned* tetNodeInds, double* weights, 
					    Snac_Node* dstNodes );

	void _SnacRemesher_InterpolateElements( void* context );
	void _SnacRemesher_UpdateElements( void* context );
	void _SnacRemesher_InterpolateElement(
		void*					_context,
		Element_LocalIndex		dstEltInd, 
		Tetrahedra_Index		dstTetInd, 
		SnacRemesher_Element*	dstEltArray, 
		Element_DomainIndex		srcEltInd, 
		Tetrahedra_Index		srcTetInd );
	void _SnacRemesher_CopyElement(
		void*					_context,
		Element_LocalIndex		eltInd, 
		Tetrahedra_Index		tetInd, 
		SnacRemesher_Element*	eltArray );

	struct dist_id_pair { double dist; unsigned int id; };


#endif /* __SnacRemesher_Remesh_h__ */
