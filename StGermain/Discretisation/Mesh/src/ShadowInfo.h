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
**	Stores the information necessary to share shadow values between
**	processors.
**
** Assumptions:
**
** Comments:
**	May want to turn this into a full-fledged class eventually that
**	actually transfers the information for you...
**
** $Id: ShadowInfo.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisaton_Mesh_ShadowInfo_h__
#define __Discretisaton_Mesh_ShadowInfo_h__

struct ProcNbrInfo {
	/** Count of neighbouring processors */
	PartitionIndex	procNbrCnt;
	/** The rank of each neighbour processor */
	PartitionIndex*	procNbrTbl;
};

/** Stores the information necessary to transfer shadowed values between
processors. */
struct ShadowInfo {
	/** Info on neigbouring processor indices */
	ProcNbrInfo*	procNbrInfo;
	/** For each nbr processor, count of how many of its shadow nodes
	live on the current processor */
	Index*		procShadowedCnt;
	/** For each nbr processor, for each of its shadow nodes
	that live on the current processor, gives the local index of that
	node */
	LocalIndex**	procShadowedTbl;
	/** For each nbr processor, count of how many of my shadow nodes
	live on that processor */
	Index*		procShadowCnt;
	/** For each nbr processor, for each of my shadow nodes that
	live on that processor, gives its domain index on my proc. */
	DomainIndex**	procShadowTbl;
};

#endif /* __Discretisaton_Mesh_ShadowInfo_h__ */
