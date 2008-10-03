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
** $Id: Edge.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"


#include "units.h"
#include "types.h"
#include "Edge.h"

#include <stdlib.h>
#include <assert.h>


/*--------------------------------------------------------------------------------------------------------------------------
** Macros
*/

#define Edge_TriMax( triCount )		\
	(2 * triCount + 1)


/*--------------------------------------------------------------------------------------------------------------------------
** Functions
*/

Edge_Index Edge_BuildList_FromTriangles(
		Triangle_List		triTbl, 
		Triangle_Index		triCount, 
		Edge_List*		edgeTbl, 
		EdgeFaces_List*		edgeFacesTbl )
{
	Edge_Index	cnt = 0;
	Triangle_Index	tri_I;
	
	*edgeTbl = Memory_Alloc_Array( Edge, Edge_TriMax(triCount), "Edge->edgeTbl" );
	*edgeFacesTbl = Memory_Alloc_Array( EdgeFaces, Edge_TriMax(triCount), "Edge->edgeFacesTbl" );
	
	for( tri_I = 0; tri_I < triCount; tri_I++ )
	{
		Index		i;
		
		for( i = 0; i < 3; i++ )
		{
			Coord_Index	p1 = triTbl[tri_I][i];
			Coord_Index	p2 = triTbl[tri_I][(i + 1) % 3];
			Edge_Index	edge_I;
			
			for( edge_I = 0; edge_I < cnt; edge_I++ )
				if( (p1 == (*edgeTbl)[edge_I][0] && p2 == (*edgeTbl)[edge_I][1]) ||
				    (p1 == (*edgeTbl)[edge_I][1] && p2 == (*edgeTbl)[edge_I][0]) ) break;
			if (edge_I < cnt)
			{
				(*edgeFacesTbl)[edge_I][1] = tri_I;
				continue;
			}
			
			(*edgeTbl)[cnt][0] = p1;
			(*edgeTbl)[cnt][1] = p2;
			(*edgeFacesTbl)[cnt][0] = tri_I;
			(*edgeFacesTbl)[cnt++][1] = triCount;
		}
	}
	
	*edgeTbl = Memory_Realloc_Array( *edgeTbl, Edge, cnt );
	*edgeFacesTbl = Memory_Realloc_Array( *edgeFacesTbl, EdgeFaces, cnt );

	return cnt;
}
