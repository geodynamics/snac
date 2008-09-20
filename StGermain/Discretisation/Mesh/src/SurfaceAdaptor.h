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
** $Id: SurfaceAdaptor.h 3584 2006-05-16 11:11:07Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisaton_Mesh_SurfaceAdaptor_h__
#define __Discretisaton_Mesh_SurfaceAdaptor_h__

	/** Textual name of this class */
	extern const Type SurfaceAdaptor_Type;

	/** Virtual function types */

	/** SurfaceAdaptor class contents */
	typedef enum {
		SurfaceAdaptor_SurfaceType_Wedge, 
		SurfaceAdaptor_SurfaceType_Sine, 
		SurfaceAdaptor_SurfaceType_Cosine, 
		SurfaceAdaptor_SurfaceType_Invalid
	} SurfaceAdaptor_SurfaceType;

	typedef struct {
		double	offs;
		double	grad;
	} SurfaceAdaptor_WedgeInfo;

	typedef struct {
		double	origin[2];
		double	amp;
		double	freq;
	} SurfaceAdaptor_TrigInfo;

	typedef union {
		SurfaceAdaptor_WedgeInfo	wedge;
		SurfaceAdaptor_TrigInfo		trig;
	} SurfaceAdaptor_SurfaceInfo;

	#define __SurfaceAdaptor				\
		/* General info */				\
		__MeshAdaptor					\
								\
		/* Virtual info */				\
								\
		/* SurfaceAdaptor info */			\
		SurfaceAdaptor_SurfaceType	surfaceType;	\
		SurfaceAdaptor_SurfaceInfo	info;

	struct SurfaceAdaptor { __SurfaceAdaptor };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/

	#define SURFACEADAPTOR_DEFARGS	\
		MESHADAPTOR_DEFARGS

	#define SURFACEADAPTOR_PASSARGS	\
		MESHADAPTOR_PASSARGS

	SurfaceAdaptor* SurfaceAdaptor_New( Name name );
	SurfaceAdaptor* _SurfaceAdaptor_New( SURFACEADAPTOR_DEFARGS );
	void _SurfaceAdaptor_Init( SurfaceAdaptor* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _SurfaceAdaptor_Delete( void* adaptor );
	void _SurfaceAdaptor_Print( void* adaptor, Stream* stream );
	void _SurfaceAdaptor_Construct( void* adaptor, Stg_ComponentFactory* cf, void* data );
	void _SurfaceAdaptor_Build( void* adaptor, void* data );
	void _SurfaceAdaptor_Initialise( void* adaptor, void* data );
	void _SurfaceAdaptor_Execute( void* adaptor, void* data );
	void _SurfaceAdaptor_Destroy( void* adaptor, void* data );

	void SurfaceAdaptor_Generate( void* adaptor, void* _mesh );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	double SurfaceAdaptor_Wedge( SurfaceAdaptor* self, Mesh* mesh, 
				     unsigned* globalSize, unsigned vertex, unsigned* vertexInds );
	double SurfaceAdaptor_Sine( SurfaceAdaptor* self, Mesh* mesh, 
				    unsigned* globalSize, unsigned vertex, unsigned* vertexInds );
	double SurfaceAdaptor_Cosine( SurfaceAdaptor* self, Mesh* mesh, 
				      unsigned* globalSize, unsigned vertex, unsigned* vertexInds );

#endif /* __Discretisaton_Mesh_SurfaceAdaptor_h__ */
