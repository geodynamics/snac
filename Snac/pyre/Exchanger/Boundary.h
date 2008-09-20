/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003,
**	Steve Quenette, 110 Victoria Street, Melbourne, Victoria, 3053, Australia.
**	Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
**	University of Texas, 1 University Station, Austin, Texas, 78712, USA.
**
** Authors:
**	Eh Tan, ?, Caltech (tan2@gps.caltech.edu)
**	Pururav Thoutireddy, ?, Caltech (puru@cacr.caltech.edu)
**	Eun-seo Choi, ?, Caltech (ces74@gps.caltech.edu)
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
** $Id: Boundary.h 1279 2004-04-26 21:59:27Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __SnacExchanger_Boundary_h__
#define __SnacExchanger_Boundary_h__

#include "Exchanger/Boundary.h"
#include "Exchanger/DIM.h"
extern "C" {
	#include <mpi.h>
	#include <StGermain/StGermain.h>
	#include <StGermain/FD/FD.h>
	#include <Snac/Snac.h>
}

class Boundary : public Exchanger::Boundary {
	ExtensionManager_Index SnacExchanger_ContextHandle; // aleviates having to link against the plugin itself

public:
	Boundary();
	explicit Boundary( const void* _context );
	virtual ~Boundary();

private:
	/* Obtain the bounding box to the coupled mesh */
	void initBBox( const void* _context );

	/* Build the array of coupling-boundary nodes and their coordinates */
	void initX( const void* _context );

	/* Is the specified node (at i, j, k) a coupling-boundary node? */
	inline bool isOnBoundary( const void* _context, unsigned int i, unsigned int j, unsigned int k ) const;

	/* Is the specified node (at index) a coupling-boundary node? */
	inline bool isOnBoundary( const void* _context, unsigned int node_dI ) const;
};

#endif /* __SnacExchanger_Boundary_h__ */
