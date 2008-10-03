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
** $Id: QuadEdge.h 3462 2006-02-19 06:53:24Z Raquibul Hassan$
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Geometry_QuadEdge_h__
#define __Discretisation_Geometry_QuadEdge_h__

	/* Virtual function types */
	
	/** Textual name of this class */
	extern const Type QuadEdge_Type;

	#define IN_USE 1
	
	typedef struct VoronoiVertex_t{
		float point[2];
	}VoronoiVertex;

	/** QuadEdge class contents (see QuadEdge) */
	#define __QuadEdge \
		QuadEdgeRef 	next[4];\
		void			*data[4];\
		unsigned		count;\
		unsigned		attributes;

	struct QuadEdge { __QuadEdge };

/* Defining WORD according to the architecture */
#if defined(SYSTEM_SIZEOF_LONG) && SYSTEM_SIZEOF_LONG == 8
	#define WORD 0xfffffffffffffffcu
#else
	#define WORD 0xfffffffcu
#endif

#define COUNT(e)  ((QuadEdge *)((e)&WORD))->count

/* Defining macros for operating on bit flags */
#define IS_IN_USE(e)	(((QuadEdge *)((e)&WORD))->attributes & IN_USE)
#define IS_FREE(e)		(!(((QuadEdge *)((e)&WORD))->attributes & IN_USE))
#define SET_IN_USE(e)	(((QuadEdge *)((e)&WORD))->attributes |= IN_USE)
#define SET_FREE(e)		(((QuadEdge *)((e)&WORD))->attributes &= ~IN_USE)

/* Edge orientation operators: */

#define ROT(e) (((e)&WORD)+(((e)+1)&3u))
#define SYM(e) (((e)&WORD)+(((e)+2)&3u))
#define TOR(e) (((e)&WORD)+(((e)+3)&3u))

/* Vertex/face walking operators: */

#define ONEXT(e) ((QuadEdge *)((e)&WORD))->next[(e)&3]
#define ROTRNEXT(e) ((QuadEdge *)((e)&WORD))->next[((e)+1)&3]
#define SYMDNEXT(e) ((QuadEdge *)((e)&WORD))->next[((e)+2)&3]
#define TORLNEXT(e) ((QuadEdge *)((e)&WORD))->next[((e)+3)&3]

#define RNEXT(e) (TOR(ROTRNEXT(e)))
#define DNEXT(e) (SYM(SYMDNEXT(e)))
#define LNEXT(e) (ROT(TORLNEXT(e)))

#define OPREV(e) (ROT(ROTRNEXT(e)))
#define DPREV(e) (TOR(TORLNEXT(e)))
#define RPREV(e) (SYMDNEXT(e))
#define LPREV(e) (SYM(ONEXT(e)))

/* Data pointers: */

#define ODATA(e) ((QuadEdge *)((e)&WORD))->data[(e)&3]
#define RDATA(e) ((QuadEdge *)((e)&WORD))->data[((e)+1)&3]
#define DDATA(e) ((QuadEdge *)((e)&WORD))->data[((e)+2)&3]
#define LDATA(e) ((QuadEdge *)((e)&WORD))->data[((e)+3)&3]

#define ORG(e) ODATA(e)
#define DEST(e) DDATA(e)

#define VORG(e) RDATA(e)
#define VDEST(e) LDATA(e)

/* Fast absolute floating point value */
#define FABS(a)  ((a) >= 0.0 ? (a) : -(a))

/* Macro for calculating the area of a triangle */
#define CCW(a, b, c, result)\
	do{\
		double ax, ay, bx, by, cx, cy; \
		ax = (*(a))[0]; ay = (*(a))[1];\
		bx = (*(b))[0]; by = (*(b))[1];\
		cx = (*(c))[0]; cy = (*(c))[1];\
		*(result) = ((bx*cy-by*cx) - (ax*cy-ay*cx) + (ax*by-ay*bx));\
	}while(0)

/* Macro for calculating the cross product of 2 3D vectors */
#define CROSS_PROD( result, a, b )\
	do{\
		result[0]=a[1]*b[2] - b[1]*a[2];\
		result[1]=-1.0f*(a[0]*b[2] - b[0]*a[2]);\
		result[2]=a[0]*b[1] - b[0]*a[1];\
	}while(0)

/* Macro for calculating the circumcircle of 3 points */
#define CIRCUM_CIRCLE( a, b, c, result ) \
do{\
	double ab[3], ac[3], abMid[3], acMid[3], cp1[3], cp2[3], rhs[3], lhs[3], mod, temp, lambda; \
	ab[0] = (*(b))[0] - (*(a))[0];\
	ab[1] = (*(b))[1] - (*(a))[1];\
	ab[2] = 0.0f;\
	mod = sqrt(ab[0]*ab[0] + ab[1]*ab[1]);\
	ab[0]/=mod;\
	ab[1]/=mod;\
	abMid[0] = ((*(b))[0] + (*(a))[0])*0.5;\
	abMid[1] = ((*(b))[1] + (*(a))[1])*0.5;\
\
	ac[0] = (*(c))[0] - (*(a))[0];\
	ac[1] = (*(c))[1] - (*(a))[1];\
	ac[2] = 0.0f;\
	mod = sqrt(ac[0]*ac[0] + ac[1]*ac[1]);\
	ac[0]/=mod;\
	ac[1]/=mod;\
	acMid[0] = ((*(c))[0] + (*(a))[0])*0.5;\
	acMid[1] = ((*(c))[1] + (*(a))[1])*0.5;\
\
	temp = ab[0];\
	ab[0] = -ab[1];\
	ab[1] = temp;\
\
	temp = ac[0];\
	ac[0] = -ac[1];\
	ac[1] = temp;\
\
	CROSS_PROD( cp1, abMid, ac );\
	CROSS_PROD( cp2, acMid, ac );\
\
	rhs[0] = cp2[0]-cp1[0];\
	rhs[1] = cp2[1]-cp1[1];\
	rhs[2] = cp2[2]-cp1[2];\
\
	CROSS_PROD( lhs, ab, ac );\
\
	lambda = rhs[2]/lhs[2];\
\
	(*(result))->point[0] = abMid[0] + ab[0]*lambda;\
	(*(result))->point[1] = abMid[1] + ab[1]*lambda;\
}while(0)
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	
	QuadEdgeRef MakeQuadEdge( MemoryPool *qp );
	void DeleteQuadEdge( MemoryPool *qp, QuadEdgeRef e);
	void SpliceQuadEdges(QuadEdgeRef a, QuadEdgeRef b);
	QuadEdgeRef ConnectQuadEdges(MemoryPool *qp, QuadEdgeRef a, QuadEdgeRef b);
	
#endif /* __Discretisation_Geometry_QuadEdge_h__ */
