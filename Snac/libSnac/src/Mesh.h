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
**	Currently used to store forces on the nodes when the user requests so.
**
** Assumptions:
**
** Comments:
**	Storing of forces on the nodes is implemented as an extension, as it is not needed by the algorithm/implementation,
**		but may be desired for output. Hence not to pay the price in memory increase when it is not used, this is added
**		by the dictionary tag "StoreForces" == "yes".
**
** $Id: Mesh.h 2934 2005-05-12 06:14:18Z SteveQuenette $
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Snac_Mesh_h__
#define __Snac_Mesh_h__
	
	/* Mesh Information */
	struct _Snac_Mesh {
		Force*			force;
		Node_LocalIndex		nodeLocalCount;			/* needed, else print function doesn't know */
		Stream*			debug;				/* needed, else functions dont know */
		int			rank;				/* needed, else functions dont know */
	};
	
	/* Stg_Class_Delete the contents of a mesh */
	void Snac_Mesh_Delete( void* mesh );
	
	/* Print the contents of a mesh */
	void Snac_Mesh_Print( void* mesh, Stream* stream );
	
	/* Build the mesh */
	void Snac_Mesh_Build( void* mesh, void* context );
	
	/* Initialise the mesh */
	void Snac_Mesh_IC( void* mesh );
	
#endif /* __Snac_Mesh_h__ */

