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
**
** Assumptions:
**
** Comments:
**
** $Id: QuadEdge.c 3462 2006-02-19 06:53:24Z Raquibul Hassan$
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"


#include "units.h"
#include "types.h"
#include "QuadEdge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

QuadEdgeRef MakeQuadEdge( MemoryPool *qp )
{
	QuadEdgeRef e = 0;

	e = (QuadEdgeRef) MemoryPool_NewObject( QuadEdge, qp );
	SET_IN_USE( (QuadEdgeRef)e );
	
	if( e == 0 ){
		fprintf( stderr, "Out of memory..!\n Aborting..!\n" );
		assert( 0 );
	}
	
	ONEXT(e) = e;
	SYMDNEXT(e) = SYM(e);
	ROTRNEXT(e) = TOR(e);
	TORLNEXT(e) = ROT(e);
	COUNT(e) = 0;
	
	return e;
}

void DeleteQuadEdge( MemoryPool *qp, QuadEdgeRef e)
{
	QuadEdgeRef f = SYM(e);
	
	if (ONEXT(e) != e) SpliceQuadEdges(e, OPREV(e));
	if (ONEXT(f) != f) SpliceQuadEdges(f, OPREV(f));  
    
	MemoryPool_DeleteObject( qp, (QuadEdge*) ((e) & WORD) );
}

void SpliceQuadEdges(QuadEdgeRef a, QuadEdgeRef b)
{
	QuadEdgeRef ta, tb;
	QuadEdgeRef alpha = ROT(ONEXT(a));
	QuadEdgeRef beta = ROT(ONEXT(b));

	ta = ONEXT(a);
	tb = ONEXT(b);
	ONEXT(a) = tb;
	ONEXT(b) = ta;
	ta = ONEXT(alpha);
	tb = ONEXT(beta);
	ONEXT(alpha) = tb;
	ONEXT(beta) = ta;    
}

QuadEdgeRef ConnectQuadEdges(MemoryPool *qp, QuadEdgeRef a, QuadEdgeRef b)
{
	QuadEdgeRef e;

	e = MakeQuadEdge( qp );
	ORG(e) = DEST(a);
	DEST(e) = ORG(b);
	
	SpliceQuadEdges(e, LNEXT(a));
	SpliceQuadEdges(SYM(e), b);
	
	return e;
}

