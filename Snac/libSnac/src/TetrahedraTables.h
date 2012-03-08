/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
**	Mappings between Tetrahedras and Nodes and Elements
**
** Assumptions:
**	That there are exactly 8 nodes per element, and 10 tetras per element.
**
** Comments:
**	None as yet.
**
** $Id: TetrahedraTables.h 1084 2004-03-26 11:17:10Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
 
#ifndef __Snac_TetrahedraTables_h__
#define __Snac_TetrahedraTables_h__
	
	/* Node, Element & Tetrahedra constants */
	#define 				Tetrahedra_Count 10
	#define Node_Element_Count		8
	#define Node_Element_Tetrahedra_Count	5
	
	/* Maps from a given tetra, the 4 element nodes of the tetra. */
	extern const Element_NodeIndex 		TetraToNode[Tetrahedra_Count][Tetrahedra_Point_Count];
	
	/* Maps from a given tetra surface, the 3 element nodes of the tetra surface. */
	extern const unsigned int 		FaceToNode[Tetrahedra_Surface_Count][Tetrahedra_Surface_Point_Count];
	
	/* Maps from a given node of an element, the 5 tetras of the element that use the node. */
	extern const Tetrahedra_Index		NodeToTetra[Node_Element_Count][Node_Element_Tetrahedra_Count];
	
	/* Maps from a given node of an element, the surface of the 5 tetras of the element that use the node. */
	extern const Tetrahedra_Index		NodeToSurface[Node_Element_Count][Node_Element_Tetrahedra_Count];
	
	/* Maps triangle's apexes to local numbering of 4-node element -- For 2D remeshing. */
	extern const Index		TriToNode[2][3]; /* 2 triangles and 3 apexes for each. */

#endif /* __Snac_TetrahedraTables_h__ */

