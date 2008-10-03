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
** Invariants:
**
** Comments:
**
** $Id: CellRemesher.h 2225 1970-01-02 13:48:23Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __StGermain_Discretisation_Utils_CellRemesher_h__
#define __StGermain_Discretisation_Utils_CellRemesher_h__

	/* Textual name of this class. */
	extern const Type CellRemesher_Type;

	/* Virtual function types. */

	/* Class contents. */
	#define __CellRemesher					\
		/* General info */				\
		__Remesher					\
								\
		/* Virtual info */				\
								\
		/* CellRemesher info ... */			\
		unsigned			nDims;		\
		Mesh*				cellMesh;

	struct CellRemesher { __CellRemesher };


	/*-----------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/

	/* Create a CellRemesher */
	CellRemesher* CellRemesher_New( Name name );

	/* Creation implementation */
	CellRemesher* _CellRemesher_New( CLASS_ARGS, 
					 COMPONENT_ARGS, 
					 REMESHER_ARGS );

	/* Initialise a CellRemesher */
	void CellRemesher_Init( CellRemesher* self );

	/* Initialisation implementation functions */
	void _CellRemesher_Init( CellRemesher* self );


	/*-----------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _CellRemesher_Delete( void* cellRemesher );
	void _CellRemesher_Print( void* cellRemesher, Stream* stream );
	CellRemesher* _CellRemesher_DefaultNew( Name name );
	void _CellRemesher_Construct( void* cellRemesher, Stg_ComponentFactory* cf, void* data );
	void _CellRemesher_Build( void* cellRemesher, void* data );
	void _CellRemesher_Initialise( void* cellRemesher, void* data );
	void _CellRemesher_Execute( void* cellRemesher, void* data );
	void _CellRemesher_Destroy( void* cellRemesher, void* data );

	void _CellRemesher_SetMesh( void* cellRemesher, Mesh* mesh );


	/*-----------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/


	/*-----------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	void _CellRemesher_Free( CellRemesher* self );

#endif
