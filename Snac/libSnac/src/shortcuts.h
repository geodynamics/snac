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
**	Shortcuts to complex types in the eyes of the Snac user.
**
** Assumptions:
**	None as yet.
**
** Comments:
**	None as yet.
**
** $Id: shortcuts.h 1084 2004-03-26 11:17:10Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Snac_shortcuts_h__
#define __Snac_shortcuts_h__
	
	/* Snac shortcuts */
	#define Snac_Node_At( self, index )				((Snac_Node*)Node_At( self, index ))
	#define Snac_Element_At( self, index )				((Snac_Element*)Element_At( self, index ))
	
	#define Snac_Node_Neighbour_I( self, node, index )		Node_Neighbour_I( self, node, index )
	#define Snac_Node_Element_I( self, node, index )		Node_Element_I( self, node, index )
	#define Snac_Element_Neighbour_I( self, element, index )	Element_Neighbour_I( self, element, index )
	#define Snac_Element_Node_I( self, element, index )		Element_Node_I( self, element, index )
	
	#define Snac_Node_Neighbour_P( self, node, index )		((Snac_Node*)Node_Neighbour_P( self, node, index ))
	#define Snac_Node_Element_P( self, node, index )		((Snac_Element*)Node_Element_P( self, node, index ))
	#define Snac_Element_Neighbour_P( self, element, index )	((Snac_Element*)Element_Neighbour_P( self, element, index ))
	#define Snac_Element_Node_P( self, element, index )		((Snac_Node*)Element_Node_P( self, element, index ))
	
	#define Snac_NodeCoord_P( self, node_I )			(&(self)->mesh->nodeCoord[node_I])
	#define Snac_Element_NodeCoord(self, element, index)		(self)->mesh->nodeCoord[(self)->mesh->elementNodeTbl[element][index]]
	
#endif /* __Snac_shortcuts_h__ */
