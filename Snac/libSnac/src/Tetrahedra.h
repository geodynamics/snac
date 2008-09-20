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
**	Calculates the geometric propereties for a given Tetarhedron. 
**
** Assumptions:
**	None as yet.
**
** Comments:
**	None as yet.
**
** $Id: Tetrahedra.h 1084 2004-03-26 11:17:10Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
 
#ifndef __Snac_Tetrahedra_h__
#define __Snac_Tetrahedra_h__
	
	/* Tetrahedra constants */
	#define					Tetrahedra_Surface_Count 4
	#define					Tetrahedra_Point_Count 4
	#define					Tetrahedra_Surface_Point_Count 3
	
	/* Calculate the volume of a tetrahedra */
	Volume Tetrahedra_Volume( Coord coord1, Coord coord2, Coord coord3, Coord coord4 );
	
	/* Calculate the area of a triangle (a face/surface of a tetrahedra) */
	Area Tetrahedra_SurfaceArea( Coord coord1, Coord coord2, Coord coord3 );
	
	/* Calculate the normal to a triangle (a face/surface of a tetrahedra) */
	void Tetrahedra_SurfaceNormal( Coord coord1, Coord coord2, Coord coord3, Normal* normal );
	
#endif /* __Snac_Tetrahedra_h__ */

