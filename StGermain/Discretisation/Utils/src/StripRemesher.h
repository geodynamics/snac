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
** $Id: StripRemesher.h 2225 1970-01-02 13:48:23Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __StGermain_Discretisation_Utils_StripRemesher_h__
#define __StGermain_Discretisation_Utils_StripRemesher_h__

	/* Textual name of this class. */
	extern const Type StripRemesher_Type;

	/* Virtual function types. */

	/* Class contents. */
	#define __StripRemesher					\
		/* General info */				\
		__Remesher					\
								\
		/* Virtual info */				\
								\
		/* StripRemesher info ... */			\
		unsigned			nDims;		\
		Bool*				deformDims;	\
		SemiRegDeform*			srd;

	struct StripRemesher { __StripRemesher };


	/*-----------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/

	/* Create a StripRemesher */
	StripRemesher* StripRemesher_New( Name name );

	/* Creation implementation */
	StripRemesher* _StripRemesher_New( CLASS_ARGS, 
					   COMPONENT_ARGS, 
					   REMESHER_ARGS );

	/* Initialise a StripRemesher */
	void StripRemesher_Init( StripRemesher* self );

	/* Initialisation implementation functions */
	void _StripRemesher_Init( StripRemesher* self );


	/*-----------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _StripRemesher_Delete( void* stripRemesher );
	void _StripRemesher_Print( void* stripRemesher, Stream* stream );
	StripRemesher* _StripRemesher_DefaultNew( Name name );
	void _StripRemesher_Construct( void* stripRemesher, Stg_ComponentFactory* cf, void* data );
	void _StripRemesher_Build( void* stripRemesher, void* data );
	void _StripRemesher_Initialise( void* stripRemesher, void* data );
	void _StripRemesher_Execute( void* stripRemesher, void* data );
	void _StripRemesher_Destroy( void* stripRemesher, void* data );

	void _StripRemesher_SetMesh( void* stripRemesher, Mesh* mesh );


	/*-----------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	void StripRemesher_SetDims( void* stripRemesher, Bool* dims );


	/*-----------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	void _StripRemesher_Free( StripRemesher* self );
	void _StripRemesher_BuildStrips( SemiRegDeform* srd, unsigned dim, unsigned deformDim, 
					 IJK ijkLow, IJK ijkUpp );

#endif
