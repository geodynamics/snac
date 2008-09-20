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
** $Id: SemiRegDeform.h 2225 1970-01-02 13:48:23Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Utils_SemiRegDeform_h__
#define __Discretisation_Utils_SemiRegDeform_h__
	

	/* A support structure for this class. */
	typedef struct {
	      Mesh*		mesh;
	      unsigned short	nDims;
	      unsigned		nNodes[3];
	      unsigned		basis[3];
	} GRM;

	/* Textual name of this class */
	extern const Type SemiRegDeform_Type;

	/* Virtual function types */
	
	/* Class contents */
	#define __SemiRegDeform \
		/* General info */ \
		__Stg_Component \
		\
		/* Virtual info */ \
		\
		/* SemiRegDeform info ... */ \
		GRM					grm; \
		unsigned				nStrips; \
		unsigned*				beginInds; \
		unsigned*				endInds; \
		unsigned*				conDims; \
		\
		Sync*					sync; \
		unsigned				nRemotes; \
		Coord*					remotes;

	struct SemiRegDeform { __SemiRegDeform };
	
	
	/*-----------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/* Create a SemiRegDeform */
	SemiRegDeform* SemiRegDeform_DefaultNew( Name name );
	
	SemiRegDeform* SemiRegDeform_New( Name name );
	
	/* Creation implementation */
	SemiRegDeform* _SemiRegDeform_New(SizeT						_sizeOfSelf, 
					  Type						type,
					  Stg_Class_DeleteFunction*			_delete,
					  Stg_Class_PrintFunction*			_print, 
					  Stg_Class_CopyFunction*			_copy, 
					  Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
					  Stg_Component_ConstructFunction*		_construct,
					  Stg_Component_BuildFunction*			_build,
					  Stg_Component_InitialiseFunction*		_initialise,
					  Stg_Component_ExecuteFunction*		_execute,
					  Stg_Component_DestroyFunction*		_destroy, 
					  Name						name );
	
	
	/* Initialise a SemiRegDeform */
	void SemiRegDeform_Init( SemiRegDeform* self, Name name );
	
	/* Initialisation implementation functions */
	void _SemiRegDeform_Init( SemiRegDeform* self );
	
	
	/*-----------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/* Stg_Class_Delete implementation */
	void _SemiRegDeform_Delete( void* srd );
	
	/* Print implementation */
	void _SemiRegDeform_Print( void* srd, Stream* stream );
	
	/* Construct implementation */
	void _SemiRegDeform_Construct( void* srd, Stg_ComponentFactory* cf, void* data );
	
	/* Build implementation */
	void _SemiRegDeform_Build( void* srd, void* data );
	
	/* Component implementation */
	void _SemiRegDeform_Initialise( void* srd, void* data );
	
	/* Execute implementation */
	void _SemiRegDeform_Execute( void* srd, void* data );
	
	/* Destroy implementation */
	void _SemiRegDeform_Destroy( void* srd, void* data );
	
	
	/*-----------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	void SemiRegDeform_SetMesh( void* srd, Mesh* mesh );

	void SemiRegDeform_AddStrip( void* srd, unsigned begin, unsigned end );

	void SemiRegDeform_Deform( void* srd );
	
	
	/*-----------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	void _SemiRegDeform_SyncInit( void* srd );

	void _SemiRegDeform_FreeInternal( void* srd );

	void RegMesh_Generalise( Mesh* mesh, GRM* grm );

	void GRM_Lift( GRM* grm, unsigned ind, unsigned* dimInds );

	void GRM_Project( GRM* grm, unsigned* dimInds, unsigned* ind );
	
	
#endif /* __Discretisation_Utils_SemiRegDeform_h__ */
