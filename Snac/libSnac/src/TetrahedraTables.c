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
** $Id: TetrahedraTables.c 1792 2004-07-30 05:42:39Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
 
#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>

#include "units.h"
#include "types.h"
#include "shortcuts.h"
#include "Tetrahedra.h"
#include "TetrahedraTables.h"

const Element_NodeIndex TetraToNode[Tetrahedra_Count][Tetrahedra_Point_Count] = {
	{ 7, 2, 3, 0 },
	{ 0, 5, 4, 7 },
	{ 2, 5, 1, 0 },
	{ 5, 2, 6, 7 },
	{ 2, 5, 0, 7 },
	{ 3, 4, 7, 6 },
	{ 4, 3, 0, 1 },
	{ 6, 1, 2, 3 },
	{ 1, 6, 5, 4 },
	{ 1, 3, 6, 4 }
};


const unsigned int FaceToNode[Tetrahedra_Surface_Count][Tetrahedra_Surface_Point_Count] = { 
	{ 0, 1, 2 },
	{ 1, 3, 2 },
	{ 0, 2, 3 },
	{ 3, 1, 0 }
}; 


const Tetrahedra_Index NodeToTetra[Node_Element_Count][Node_Element_Tetrahedra_Count] = {
	{ 3, 5, 7, 8, 9 }, /* 0: left,  bottom, far  element of node... node of element = 7 */
	{ 0, 1, 3, 4, 5 }, /* 1: right, bottom, far  element of node... node of element = 6 */
	{ 1, 2, 3, 4, 8 }, /* 2: left,  top,    far  element of node... node of element = 5 */
	{ 1, 5, 6, 8, 9 }, /* 3: right, top,    far  element of node... node of element = 4 */
	{ 0, 2, 3, 4, 7 }, /* 4: left,  bottom, near element of node... node of element = 3 */
	{ 0, 5, 6, 7, 9 }, /* 5: right, bottom, near element of node... node of element = 2 */
	{ 2, 6, 7, 8, 9 }, /* 6: left,  top,    near element of node... node of element = 1 */
	{ 0, 1, 2, 4, 6 }  /* 7: right, top,    near element of node... node of element = 0 */
};

const Tetrahedra_Index NodeToSurface[Node_Element_Count][Node_Element_Tetrahedra_Count] = {
	{ 3, 0, 1, 2, 3 }, /* 0: left,  bottom, far  element of node... node of element = 7 */
	{ 1, 0, 0, 0, 3 }, /* 1: right, bottom, far  element of node... node of element = 6 */
	{ 2, 2, 1, 2, 3 }, /* 2: left,  top,    far  element of node... node of element = 5 */
	{ 3, 2, 1, 0, 0 }, /* 3: right, top,    far  element of node... node of element = 4 */
	{ 2, 1, 2, 1, 3 }, /* 4: left,  bottom, near element of node... node of element = 3 */
	{ 3, 1, 2, 0, 2 }, /* 5: right, bottom, near element of node... node of element = 2 */
	{ 3, 0, 2, 1, 1 }, /* 6: left,  top,    near element of node... node of element = 1 */
	{ 0, 1, 0, 3, 3 }, /* 7: right, top,    near element of node... node of element = 0 */
};

