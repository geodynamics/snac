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
** $Id: Mesh.c 3095 2005-07-13 09:50:46Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>

#include "Snac/Snac.h"
#include "types.h"
#include "Mesh.h"

#include <stdio.h>


/*
** Due to the rewrite this func is severely useless.  Need to fix it.
*/

void SnacRemesher_Mesh_Print( void* mesh, Stream* stream ) {
	SnacRemesher_Mesh*	self = (SnacRemesher_Mesh*)mesh;
/*	Node_LocalIndex		node_lI; */
/*	Node_LocalIndex		index; */

	Journal_Printf( stream, "SnacRemesher_Mesh:\n" );

#if 0
	Journal_Printf( stream, "\tnodeLocalCount: %u\n", self->nodeLocalCount );
	Journal_Printf( stream, "\tnodeDomainCount: %u\n", self->nodeDomainCount );
	Journal_Printf( stream, "\tnodeGlobalCount: %u\n", self->nodeGlobalCount );
	Journal_Printf( stream, "\telementLocalCount: %u\n", self->elementLocalCount );
	Journal_Printf( stream, "\telementDomainCount: %u\n", self->elementDomainCount );
	Journal_Printf( stream, "\telementGlobalCount: %u\n", self->elementGlobalCount );
	Journal_Printf( stream, "\tinitialNodeCoord[0-%u]: { ", self->nodeLocalCount );
	for( node_lI = 0; node_lI < self->nodeLocalCount; node_lI++ ) {
		Journal_Printf( stream,
			"{ %g, %g, %g } ",
			self->initialNodeCoord[node_lI][0],
			self->initialNodeCoord[node_lI][1],
			self->initialNodeCoord[node_lI][2] );
	}
	Journal_Printf( stream, "}\n" );
	Journal_Printf( stream, "\tnewNodeCoord[0-%u]: { ", self->nodeLocalCount );
	for( node_lI = 0; node_lI < self->nodeLocalCount; node_lI++ ) {
		Journal_Printf( stream,
			"{ %g, %g, %g } ",
			self->newNodeCoord[node_lI][0],
			self->newNodeCoord[node_lI][1],
			self->newNodeCoord[node_lI][2] );
	}
	Journal_Printf( stream, "}\n" );
	Journal_Printf( stream, "\tnewNode[0-%u]: { ... TODO: Print nodes nicely... }\n", self->nodeLocalCount );

	Journal_Printf( stream,
		"\tmeshType: %s\n",
		self->meshType == SnacRemesher_Spherical ? "Spherical" :
			self->meshType == SnacRemesher_Cartesian ? "Cartesian" : "Invalid value!" );

	Journal_Printf( stream, "\tneighbourRankCount: %u\n", self->neighbourRankCount );
	Journal_Printf( stream, "\tneighbourRank[0-%u]: {", self->neighbourRankCount );
	for( index = 0; index < self->neighbourRankCount; index++ ) {
		Journal_Printf( stream, "%i ", self->neighbourRank[index] );
	}
	Journal_Printf( stream, "}\n" );

	Journal_Printf( stream, "\ttopInternalSet: " ); Print( self->topInternalSet, stream );
	Journal_Printf( stream, "\tbottomInternalSet: " ); Print( self->bottomInternalSet, stream );
	Journal_Printf( stream, "\twallSet: " ); Print( self->wallSet, stream );
	Journal_Printf( stream, "\tinternalSet: " ); Print( self->internalSet, stream );

	Journal_Printf( stream, "\ttopInternalCount: %u\n", self->topInternalCount );
	Journal_Printf( stream, "\tbottomInternalCount: %u\n", self->bottomInternalCount );
	Journal_Printf( stream, "\twalllCount: %u\n", self->wallCount );
	Journal_Printf( stream, "\tinternalCount: %u\n", self->internalCount );

	Journal_Printf( stream, "\ttopInternalArray[0-%u]: { ", self->topInternalCount );
	for( index = 0; index < self->topInternalCount; index++ ) {
		Journal_Printf( stream, "%u ", self->topInternalArray[index] );
	}
	Journal_Printf( stream, "}\n" );
	Journal_Printf( stream, "\tbottomInternalArray[0-%u]: { ", self->bottomInternalCount );
	for( index = 0; index < self->bottomInternalCount; index++ ) {
		Journal_Printf( stream, "%u ", self->bottomInternalArray[index] );
	}
	Journal_Printf( stream, "}\n" );
	Journal_Printf( stream, "\twallSet[0-%u]: { ", self->wallCount );
	for( index = 0; index < self->wallCount; index++ ) {
		Journal_Printf( stream, "%u ", self->wallArray[index] );
	}
	Journal_Printf( stream, "}\n" );
	Journal_Printf( stream, "\tinternalArray[0-%u]: { ", self->internalCount );
	for( index = 0; index < self->internalCount; index++ ) {
		Journal_Printf( stream, "%u ", self->internalArray[index] );
	}
	Journal_Printf( stream, "}\n" );

	/* Shadow */
	Journal_Printf( stream, "\tshadowNodeLocalCount[0-%u]: { ", self->neighbourRankCount );
	for( index = 0; index < self->neighbourRankCount; index++ ) {
		Journal_Printf( stream, "%u ", self->shadowNodeLocalCount[index] );
	}
	Journal_Printf( stream, "}\n" );
	Journal_Printf( stream, "\tshadowNodeLocalArray[0-%u]: {\n", self->neighbourRankCount );
	for( index = 0; index < self->neighbourRankCount; index++ ) {
		Index					index2;

		Journal_Printf( stream, "\t\tshadowNodeLocalArray[%u][0-%u]: { ", index, self->shadowNodeLocalCount[index] );
		for( index2 = 0; index2 < self->shadowNodeLocalCount[index]; index2++ ) {
			Journal_Printf( stream, "%u ", self->shadowNodeLocalArray[index][index2] );
		}
		Journal_Printf( stream, "}\n" );
	}
	Journal_Printf( stream, "}\n" );
	Journal_Printf( stream, "\tshadowNodeRemoteCount[0-%u]: { ", self->neighbourRankCount );
	for( index = 0; index < self->neighbourRankCount; index++ ) {
		Journal_Printf( stream, "%u ", self->shadowNodeRemoteCount[index] );
	}
	Journal_Printf( stream, "}\n" );
	Journal_Printf( stream, "\tshadowNodeRemoteArray[0-%u]: {\n", self->neighbourRankCount );
	for( index = 0; index < self->neighbourRankCount; index++ ) {
		Index					index2;

		Journal_Printf( stream, "\t\tshadowNodeRemoteArray[%u][0-%u]: { ", index, self->shadowNodeRemoteCount[index] );
		for( index2 = 0; index2 < self->shadowNodeRemoteCount[index]; index2++ ) {
			Journal_Printf( stream, "%u ", self->shadowNodeRemoteArray[index][index2] );
		}
		Journal_Printf( stream, "}\n" );
	}
	Journal_Printf( stream, "}\n" );
	Journal_Printf( stream, "\tshadowNodeRemoteCoord[0-%u]: {\n", self->neighbourRankCount );
	for( index = 0; index < self->neighbourRankCount; index++ ) {
		Index					index2;

		Journal_Printf( stream, "\t\tshadowNodeRemoteCoord[%u][0-%u]: { ", index, self->shadowNodeRemoteCount[index] );
		for( index2 = 0; index2 < self->shadowNodeRemoteCount[index]; index2++ ) {
			Journal_Printf( stream,
				"{ %g, %g, %g }, ",
				self->shadowNodeRemoteCoord[index][index2][0],
				self->shadowNodeRemoteCoord[index][index2][1],
				self->shadowNodeRemoteCoord[index][index2][2] );
		}
		Journal_Printf( stream, "}\n" );
	}
	Journal_Printf( stream, "}\n" );
	Journal_Printf( stream, "\tshadowNodeRemote[0-%u]: {\n", self->neighbourRankCount );
	for( index = 0; index < self->neighbourRankCount; index++ ) {
		Index					index2;

		Journal_Printf( stream, "\t\tshadowNodeRemote[%u][0-%u]: { ", index, self->shadowNodeRemoteCount[index] );
		for( index2 = 0; index2 < self->shadowNodeRemoteCount[index]; index2++ ) {
			Snac_Node_Print( &self->shadowNodeRemote[index][index2] );
		}
		Journal_Printf( stream, "}\n" );
	}
	Journal_Printf( stream, "}\n" );

	/* Shadow Elements*/
	Journal_Printf( stream, "\tshadowElementLocalCount[0-%u]: { ", self->neighbourRankCount );
	for( index = 0; index < self->neighbourRankCount; index++ ) {
		Journal_Printf( stream, "%u ", self->shadowElementLocalCount[index] );
	}
	Journal_Printf( stream, "}\n" );
	Journal_Printf( stream, "\tshadowElementLocalArray[0-%u]: {\n", self->neighbourRankCount );
	for( index = 0; index < self->neighbourRankCount; index++ ) {
		Index					index2;

		Journal_Printf( stream, "\t\tshadowElementLocalArray[%u][0-%u]: { ", index, self->shadowElementLocalCount[index] );
		for( index2 = 0; index2 < self->shadowElementLocalCount[index]; index2++ ) {
			Journal_Printf( stream, "%u ", self->shadowElementLocalArray[index][index2] );
		}
		Journal_Printf( stream, "}\n" );
	}
	Journal_Printf( stream, "}\n" );
	Journal_Printf( stream, "\tshadowElementRemoteCount[0-%u]: { ", self->neighbourRankCount );
	for( index = 0; index < self->neighbourRankCount; index++ ) {
		Journal_Printf( stream, "%u ", self->shadowElementRemoteCount[index] );
	}
	Journal_Printf( stream, "}\n" );
	Journal_Printf( stream, "\tshadowElementRemoteArray[0-%u]: {\n", self->neighbourRankCount );
	for( index = 0; index < self->neighbourRankCount; index++ ) {
		Index					index2;

		Journal_Printf( stream, "\t\tshadowElementRemoteArray[%u][0-%u]: { ", index, self->shadowElementRemoteCount[index] );
		for( index2 = 0; index2 < self->shadowElementRemoteCount[index]; index2++ ) {
			Journal_Printf( stream, "%u ", self->shadowElementRemoteArray[index][index2] );
		}
		Journal_Printf( stream, "}\n" );
	}
	Journal_Printf( stream, "}\n" );
	for( index = 0; index < self->neighbourRankCount; index++ ) {
		Index					index2;

		Journal_Printf( stream, "\t\tshadowElementRemote[%u][0-%u]: { ", index, self->shadowElementRemoteCount[index] );
		for( index2 = 0; index2 < self->shadowElementRemoteCount[index]; index2++ ) {
			Snac_Element_Print( &self->shadowElementRemote[index][index2], stream );
		}
		Journal_Printf( stream, "}\n" );
	}
	Journal_Printf( stream, "}\n" );
#endif
}
