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
** $Id: Utils.h 1848 2004-08-11 07:10:08Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __SnacRemesher_Utils_h__
#define __SnacRemesher_Utils_h__
	
	
	#define SnacArcTan( o, a ) \
		(((a) == 0.0) ? 0.5 * M_PI : \
		((a) > 0.0) ? atan( (o) / (a) ) : \
		((o) >= 0.0) ? M_PI + atan( (o) / (a) ) : \
		atan( (o) / (a) ) - M_PI)
	
	
	Node_DomainIndex findClosestNode( void* _context, Coord point, Node_LocalIndex refNodeInd );

	Node_DomainIndex findClosestNodeInElement( void* _context, Coord point,	unsigned nEltNodes, Node_DomainIndex *eltNodes );
	
	Bool pointInElement( void* _context, Coord point, Element_DomainIndex dElementInd );

	
#endif /* __SnacRemesher_Utils_h__ */
