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
**	Utilities to get the sides of a regular mesh etc.
**
** Assumptions:
**
** Comments:
**
** $Id: RegularMeshUtils.h 3574 2006-05-15 11:30:33Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Utils_RegularMeshUtils_h__
#define __Discretisation_Utils_RegularMeshUtils_h__

	extern Index RegularMeshUtils_ascendingIJK_ToHughesNodeNumberMap[8];

	/*--------------------------------------------------------------------------------------------------------------------------
	** Set functions
	*/
	
	#define RegularMeshUtils_Node_1DTo3D( hexaMD, index, iPtr, jPtr, kPtr ) \
		*(iPtr) = (index) % (hexaMD)->nodeGlobal3DCounts[0]; \
		*(jPtr) = ((index) / (hexaMD)->nodeGlobal3DCounts[0]) % (hexaMD)->nodeGlobal3DCounts[1]; \
		*(kPtr) = (index) / ((hexaMD)->nodeGlobal3DCounts[0] * (hexaMD)->nodeGlobal3DCounts[1])
	
	#define RegularMeshUtils_Node_Local1DTo3D( hexaMD, index, iPtr, jPtr, kPtr ) \
		*(iPtr) = (index) % (hexaMD)->nodeLocal3DCounts[(hexaMD)->rank][0]; \
		*(jPtr) = ((index) / (hexaMD)->nodeLocal3DCounts[(hexaMD)->rank][0]) % (hexaMD)->nodeLocal3DCounts[(hexaMD)->rank][1]; \
		*(kPtr) = (index) / ((hexaMD)->nodeLocal3DCounts[(hexaMD)->rank][0] * (hexaMD)->nodeLocal3DCounts[(hexaMD)->rank][1])
	
	#define RegularMeshUtils_Node_Global3DToGlobal1D( hexaMD, i, j, k ) \
		((k) * (hexaMD)->nodeGlobal3DCounts[0] * (hexaMD)->nodeGlobal3DCounts[1] + \
		(j) * (hexaMD)->nodeGlobal3DCounts[0] + (i))

	/** This one needs to be a macro so we can do some decent error checking */
	Node_LocalIndex RegularMeshUtils_Node_Global3DToLocal1D( HexaMD* hexaMD, Index i, Index j, Index k );

	#define RegularMeshUtils_Node_Local3DTo1D( hexaMD, i, j, k ) \
		((k) * (hexaMD)->nodeLocal3DCounts[(hexaMD)->rank][0] * (hexaMD)->nodeLocal3DCounts[(hexaMD)->rank][1] + \
		(j) * (hexaMD)->nodeLocal3DCounts[(hexaMD)->rank][0] + (i))

	#define RegularMeshUtils_Element_1DTo3D( hexaMD, index, iPtr, jPtr, kPtr ) \
		*(iPtr) = (hexaMD)->elementGlobal3DCounts[0] ? (index) % (hexaMD)->elementGlobal3DCounts[0] : 0; \
		*(jPtr) = (hexaMD)->elementGlobal3DCounts[1] ? ((index) / ((hexaMD)->elementGlobal3DCounts[0] ? (hexaMD)->elementGlobal3DCounts[0] : 1)) % (hexaMD)->elementGlobal3DCounts[1] : 0; \
		*(kPtr) = (hexaMD)->elementGlobal3DCounts[2] ? (index) / (((hexaMD)->elementGlobal3DCounts[0] ? (hexaMD)->elementGlobal3DCounts[0] : 1) * ((hexaMD)->elementGlobal3DCounts[1] ? (hexaMD)->elementGlobal3DCounts[1] : 1)) : 0
	
	#define RegularMeshUtils_Element_3DTo1D( hexaMD, i, j, k ) \
		((k) * ((hexaMD)->elementGlobal3DCounts[0] ? (hexaMD)->elementGlobal3DCounts[0] : 1) * ((hexaMD)->elementGlobal3DCounts[1] ? (hexaMD)->elementGlobal3DCounts[1] : 1) + \
		(j) * ((hexaMD)->elementGlobal3DCounts[0] ? (hexaMD)->elementGlobal3DCounts[0] : 1) + (i))
	
	#define RegularMeshUtils_Element_Local3DTo1D( hexaMD, i, j, k ) \
		((k) * \
		((hexaMD)->elementLocal3DCounts[(hexaMD)->rank][0] ? (hexaMD)->elementLocal3DCounts[(hexaMD)->rank][0] : 0) * \
		((hexaMD)->elementLocal3DCounts[(hexaMD)->rank][1] ? (hexaMD)->elementLocal3DCounts[(hexaMD)->rank][1] : 0) + \
		(j) * \
		((hexaMD)->elementLocal3DCounts[(hexaMD)->rank][0] ? (hexaMD)->elementLocal3DCounts[(hexaMD)->rank][0] : 0) + \
		(i))
	
	/** Create a new set, based on node indices, of nodes on the top of the global regular mesh */
	IndexSet* RegularMeshUtils_CreateGlobalTopSet( void* _mesh );
	
	/** Create a new set, based on node indices, of nodes on the bottom of the global regular mesh */
	IndexSet* RegularMeshUtils_CreateGlobalBottomSet( void* _mesh );
	
	/** Create a new set, based on node indices, of nodes on the left of the global regular mesh */
	IndexSet* RegularMeshUtils_CreateGlobalLeftSet( void* _mesh );
	
	/** Create a new set, based on node indices, of nodes on the right of the global regular mesh */
	IndexSet* RegularMeshUtils_CreateGlobalRightSet( void* _mesh );
	
	/** Create a new set, based on node indices, of nodes on the front of the global regular mesh */
	IndexSet* RegularMeshUtils_CreateGlobalFrontSet( void* _mesh );
	
	/** Create a new set, based on node indices, of nodes on the back of the global regular mesh */
	IndexSet* RegularMeshUtils_CreateGlobalBackSet( void* _mesh );


	/** Create a new set, based on node indices, of nodes on the top without the corners of the global regular mesh */
	IndexSet* RegularMeshUtils_CreateGlobalInnerTopSet( void* _mesh );
	
	/** Create a new set, based on node indices, of nodes on the bottom without the corners of the global regular mesh */
	IndexSet* RegularMeshUtils_CreateGlobalInnerBottomSet( void* _mesh );
	
	/** Create a new set, based on node indices, of nodes on the left without the corners of the global regular mesh */
	IndexSet* RegularMeshUtils_CreateGlobalInnerLeftSet( void* _mesh );
	
	/** Create a new set, based on node indices, of nodes on the right without the corners of the global regular mesh */
	IndexSet* RegularMeshUtils_CreateGlobalInnerRightSet( void* _mesh );
	
	/** Create a new set, based on node indices, of nodes on the front without the corners of the global regular mesh */
	IndexSet* RegularMeshUtils_CreateGlobalInnerFrontSet( void* _mesh );
	
	/** Create a new set, based on node indices, of nodes on the back without the corners of the global regular mesh */
	IndexSet* RegularMeshUtils_CreateGlobalInnerBackSet( void* _mesh );
	

	/** Create a new set, based on node indices, of the node on the bottom front lefthand corner */
	IndexSet* RegularMeshUtils_CreateGlobalBottomLeftFrontSet( void* _mesh );

	/** Create a new set, based on node indices, of the node on the bottom front righthand corner */
	IndexSet* RegularMeshUtils_CreateGlobalBottomRightFrontSet( void* _mesh );

	/** Create a new set, based on node indices, of the node on the top front lefthand corner */
	IndexSet* RegularMeshUtils_CreateGlobalTopLeftFrontSet( void* _mesh );

	/** Create a new set, based on node indices, of the node on the top front righthand corner */
	IndexSet* RegularMeshUtils_CreateGlobalTopRightFrontSet( void* _mesh );

	/** Create a new set, based on node indices, of the node on the bottom back lefthand corner */
	IndexSet* RegularMeshUtils_CreateGlobalBottomLeftBackSet( void* _mesh );

	/** Create a new set, based on node indices, of the node on the bottom back righthand corner */
	IndexSet* RegularMeshUtils_CreateGlobalBottomRightBackSet( void* _mesh );

	/** Create a new set, based on node indices, of the node on the top back lefthand corner */
	IndexSet* RegularMeshUtils_CreateGlobalTopLeftBackSet( void* _mesh );

	/** Create a new set, based on node indices, of the node on the top back righthand corner */
	IndexSet* RegularMeshUtils_CreateGlobalTopRightBackSet( void* _mesh );

	
	/** Create a new set, based on node indices, of nodes on the top of the local regular mesh */
	IndexSet* RegularMeshUtils_CreateLocalTopSet( void* _mesh );
	
	/** Create a new set, based on node indices, of nodes on the bottom of the local regular mesh */
	IndexSet* RegularMeshUtils_CreateLocalBottomSet( void* _mesh );
	
	/** Create a new set, based on node indices, of nodes on the left of the local regular mesh */
	IndexSet* RegularMeshUtils_CreateLocalLeftSet( void* _mesh );
	
	/** Create a new set, based on node indices, of nodes on the right of the local regular mesh */
	IndexSet* RegularMeshUtils_CreateLocalRightSet( void* _mesh );
	
	/** Create a new set, based on node indices, of nodes on the front of the local regular mesh */
	IndexSet* RegularMeshUtils_CreateLocalFrontSet( void* _mesh );
	
	/** Create a new set, based on node indices, of nodes on the back of the local regular mesh */
	IndexSet* RegularMeshUtils_CreateLocalBackSet( void* _mesh );
	
	
	/** Create a new set, based on node indices, of nodes on the top of the local regular mesh */
	IndexSet* RegularMeshUtils_CreateLocalInGlobalTopSet( void* _mesh );
	
	/** Create a new set, based on node indices, of nodes on the bottom of the local regular mesh */
	IndexSet* RegularMeshUtils_CreateLocalInGlobalBottomSet( void* _mesh );
	
	/** Create a new set, based on node indices, of nodes on the left of the local regular mesh */
	IndexSet* RegularMeshUtils_CreateLocalInGlobalLeftSet( void* _mesh );
	
	/** Create a new set, based on node indices, of nodes on the right of the local regular mesh */
	IndexSet* RegularMeshUtils_CreateLocalInGlobalRightSet( void* _mesh );
	
	/** Create a new set, based on node indices, of nodes on the front of the local regular mesh */
	IndexSet* RegularMeshUtils_CreateLocalInGlobalFrontSet( void* _mesh );
	
	/** Create a new set, based on node indices, of nodes on the back of the local regular mesh */
	IndexSet* RegularMeshUtils_CreateLocalInGlobalBackSet( void* _mesh );
	
	/** Calculates the node domain ID of the node diagonally opposite a reference node in a given element */ 
	Node_DomainIndex RegularMeshUtils_GetDiagOppositeAcrossElementNodeIndex( 
		void* _mesh, Element_DomainIndex element_dI, Node_DomainIndex refNode_dI );

#endif /* __Discretisation_Utils_RegularMeshUtils_h__ */
