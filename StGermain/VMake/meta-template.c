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
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* !!! This is an automatically generated file
 * !!! Do not make modifications here or they will be overwritten
 */
#define Stg_Component_Stringify( str ) #str

/* Note: Two macros are used to resolve the the extra macro level */
#define Stg_Component_Metadata_Create( name ) Stg_Component_Metadata_Create_Macro( name )
#define Stg_Component_Metadata_Create_Macro( name ) \
	const char* name ##_Meta = XML_METADATA; \
	const char* name ##_Name = #name; \
	const char* name ##_Version = VERSION; \
	const char* name ##_Type_GetMetadata() { /* hack...won't be needed when hierarchy rollout is done */\
		return name ##_Meta; \
	} \
	const char* name ##_GetMetadata() { \
		return name ##_Meta; \
	} \
	const char* name ##_GetName() { \
		return name ##_Name; \
	} \
	const char* name ##_GetVersion() { \
		return name ##_Version; \
	}


#if defined(COMPONENT_NAME) && defined(VERSION) && defined(XML_METADATA)

	Stg_Component_Metadata_Create( COMPONENT_NAME )

#endif

