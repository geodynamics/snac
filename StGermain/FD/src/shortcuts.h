/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003, Victorian Partnership for Advanced Computing (VPAC) Ltd, 110 Victoria Street, Melbourne, 3053, Australia.
**
** Authors:
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**	Patrick D. Sunter, Software Engineer, VPAC. (pds@vpac.org)
**	Luke J. Hodkinson, Computational Engineer, VPAC. (lhodkins@vpac.org)
**	Siew-Ching Tan, Software Engineer, VPAC. (siew@vpac.org)
**	Alan H. Lo, Computational Engineer, VPAC. (alan@vpac.org)
**	Raquibul Hassan, Computational Engineer, VPAC. (raq@vpac.org)
**
**  This library is free software; you can redistribute it and/or
**  modify it under the terms of the GNU Lesser General Public
**  License as published by the Free Software Foundation; either
**  version 2.1 of the License, or (at your option) any later version.
**
**  This library is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**  Lesser General Public License for more details.
**
**  You should have received a copy of the GNU Lesser General Public
**  License along with this library; if not, write to the Free Software
**  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
**
*/
/** \file
**  Role:
**	Shortcuts to complex types in the eyes of the Context user.
**
** Assumptions:
**	None as yet.
**
** Comments:
**	None as yet.
**
** $Id: shortcuts.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __FD_shortcuts_h__
#define __FD_shortcuts_h__
	
	/* MeshContext shortcuts */
	#define Node_At( self, index )				Mesh_NodeAt( (self)->mesh, (index) )
	#define Element_At( self, index )			Mesh_ElementAt( (self)->mesh, (index) )
	
	#define Node_Neighbour_I( self, node, index )		(self)->mesh->nodeNeighbourTbl[(node)][(index)]
	#define Node_Element_I( self, node, index )		(self)->mesh->nodeElementTbl[(node)][(index)]
	#define Element_Neighbour_I( self, element, index )	(self)->mesh->elementNeighbour[(element)][(index)]
	#define Element_Node_I( self, element, index )		(self)->mesh->elementNodeTbl[(element)][(index)]
	
	#define Node_Neighbour_P( self, node, index )		Node_At( (self), Node_Neighbour_I( (self), (node), (index) ) )
	#define Node_Element_P( self, node, index )		Element_At( (self), Node_Element_I( (self), (node), (index) ) )
	#define Element_Neighbour_P( self, element, index )	Element_At( (self), Element_Neighbour_I( (self), (element), (index) ) )
	#define Element_Node_P( self, element, index )		Node_At( (self), Element_Node_I( (self), (element), (index) ) )
	
#endif /* __FD_shortcuts_h__ */
