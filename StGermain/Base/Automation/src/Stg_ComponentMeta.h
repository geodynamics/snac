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
**	A class which encapsulates the component meta data.
**
** Assumptions:
**
** Comments:
**
** $Id: Stg_ComponentMeta.h 3367 2005-12-09 07:39:53Z AlanLo $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Automation_Stg_ComponentMeta_h__
#define __Base_Automation_Stg_ComponentMeta_h__
	
	#define __Stg_ComponentMeta_Value       \
		__Stg_Object                    \
		Type            metaType;       \
		const char*     description;    \
		const char*     defaultValue;   \
		Bool            haveValue;

	struct Stg_ComponentMeta_Value { __Stg_ComponentMeta_Value };

	Stg_ComponentMeta_Value* Stg_ComponentMeta_Value_New( Name name, Type metaType, const char* description, const char* defaultValue );
	void _Stg_ComponentMeta_Value_Delete( void* instance );
	void* _Stg_ComponentMeta_Value_Copy( void* instance, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );
	
	/* Textual name of this class */
	extern const Type Stg_ComponentMeta_Type;

	
	/* Stg_ComponentMeta information */
	#define __Stg_ComponentMeta \
		__Stg_Object \
		\
		XML_IO_Handler* ioHandler; /** For buffer memory */     \
		Dictionary*     dict;      /** For buffer memory */     \
		Type            metaType;  /**< Type of the component */\
		char*           project;                                \
		char*           location;                               \
		char*           web;                                    \
		char*           copyright;                              \
	        char*           license;                                \
		char*           parent;                                 \
		char*           description;                            \
		Bool            isValid;                                \
		Stg_ObjectList* allDependencies;                        \
		Stg_ObjectList* essentialDependencies;                  \
		Stg_ObjectList* optionalDependencies;                   \
		Stg_ObjectList* missingDependencies;                    \
		Stg_ObjectList* unexpectedDependencies;                 \
		\
	        Stg_ObjectList* allParams;                              
	
	struct Stg_ComponentMeta { __Stg_ComponentMeta };
	
	Stg_ComponentMeta* Stg_ComponentMeta_New( Name name, Type metaType );
	
	void _Stg_ComponentMeta_Delete( void* component );
	
	void _Stg_ComponentMeta_Print( void* component, Stream* stream );
	
	void* _Stg_ComponentMeta_Copy( void* component, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );
	
	
#endif /* __Base_Automation_Stg_ComponentMeta_h__ */
