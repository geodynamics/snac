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
** $Id: StencilMD.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisaton_Mesh_StencilMD_h__
#define __Discretisaton_Mesh_StencilMD_h__


	/* Virtual function types */
	
	/* Textual name of this class */
	extern const Type StencilMD_Type;

	/* StencilMD information */
	#define __StencilMD \
		/* General info */ \
		__MeshDecomp \
		\
		/* Virtual info */ \
		\
		/* StencilMD info ... */ \
		MeshLayout*			stencilLayout;
	struct _StencilMD { __StencilMD };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/* Create a StencilMD */
	StencilMD* StencilMD_New(
		Dictionary*					dictionary,
		MPI_Comm					communicator,
		ElementLayout*					elementLayout,
		NodeLayout*					nodeLayout,
		MeshLayout*					stencilLayout );
	
	/* Initialise a StencilMD */
	void StencilMD_Init(
		StencilMD*					self,
		Dictionary*					dictionary,
		MPI_Comm					communicator,
		ElementLayout*					elementLayout,
		NodeLayout*					nodeLayout,
		MeshLayout*					stencilLayout );
	
	/* Creation implementation */
	StencilMD* _StencilMD_New(
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print,
		MeshDecomp_Node_MapLocalToGlobalFunction*       nodeMapLocalToGlobal,
		MeshDecomp_Node_MapDomainToGlobalFunction*      nodeMapDomainToGlobal,
		MeshDecomp_Node_MapShadowToGlobalFunction*      nodeMapShadowToGlobal,
		MeshDecomp_Node_MapGlobalToLocalFunction*       nodeMapGlobalToLocal,
		MeshDecomp_Node_MapGlobalToDomainFunction*      nodeMapGlobalToDomain,
		MeshDecomp_Node_MapGlobalToShadowFunction*      nodeMapGlobalToShadow,
		MeshDecomp_Element_MapLocalToGlobalFunction*	elementMapLocalToGlobal,
		MeshDecomp_Element_MapDomainToGlobalFunction*	elementMapDomainToGlobal,
		MeshDecomp_Element_MapShadowToGlobalFunction*	elementMapShadowToGlobal,
		MeshDecomp_Element_MapGlobalToLocalFunction*	elementMapGlobalToLocal,
		MeshDecomp_Element_MapGlobalToDomainFunction*	elementMapGlobalToDomain,
		MeshDecomp_Element_MapGlobalToShadowFunction*	elementMapGlobalToShadow,
		MeshDecomp_Shadow_ProcCountFunction*		shadowProcCount,
		MeshDecomp_Shadow_BuildProcsFunction*		shadowBuildProcs,
		MeshDecomp_Shadow_ProcElementCountFunction*     shadowProcElementCount,
		MeshDecomp_Proc_WithElementFunction*		procWithElement,
		Dictionary*					dictionary,
		MPI_Comm					communicator,
		ElementLayout*					elementLayout,
		NodeLayout*					nodeLayout,
		MeshLayout*					stencilLayout );
	
	/* Initialisation implementation functions */
	void _StencilMD_Init(
		StencilMD*					self,
		MeshLayout*					stencilLayout );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/* Stg_Class_Delete StencilMD implementation */
	void _StencilMD_Delete( void* stencilMD );
	
	/* Print StencilMD implementation */
	void _StencilMD_Print( void* stencilMD, Stream* stream );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public member functions
	*/
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	
	
#endif /* __Discretisaton_Mesh_StencilMD_h__ */
