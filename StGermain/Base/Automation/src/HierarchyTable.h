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
**	Abstract class for objects that are named.
**
** Assumptions:
**
** Comments:
**
** $Id: HierarchyTable.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Automation_HierarchyTable_h__
#define __Base_Automation_HierarchyTable_h__
	
	/* Textual name of this class */
	extern const Type HierarchyTable_Type;

	extern HierarchyTable *stgHierarchyTable;
	
	/* HierarchyTable information */
	#define __HierarchyTable \
		/* General info */ \
		__HashTable \
		\
		/* Virtual info */ \
		\
		/* HierarchyTable info */ \
	
	struct HierarchyTable { __HierarchyTable };
	
	/** Constructor Implementation */
	HierarchyTable* HierarchyTable_New(  );
	
	/* Private member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	/** This function returns the pointer to the singleton "stgHierarchyTable" */
	#define HierarchyTable_GetHierarchyTable() \
		((HierarchyTable*) stgHierarchyTable )
	
	/** Function to be called once per class and registers it's parent in the Hierarchy hash table 
	 * the pointer to the child's type string can then be used as a key to find the pointer the 
	 * parent's type string */
	int HierarchyTable_RegisterParent( void* hierarchyTable, Type childType, Type parentType ) ;

	/** Shortcut to above function - it grabs the pointer to the HierarchyTable singleton for you */
	#define RegisterParent( childType, parentType ) \
		HierarchyTable_RegisterParent( HierarchyTable_GetHierarchyTable(), (childType), (parentType) )

	/** Macro to get immediate parent for a type */
	#define HierarchyTable_GetParent( hierarchyTable, type ) \
		(Type) HashTable_FindEntryFunction( (HashTable*) hierarchyTable, type, sizeof(const Type) )

	/** Shortcut to above macro - Gets the HierarchyTable pointer for you */
	#define GetParent( type ) \
		HierarchyTable_GetParent( HierarchyTable_GetHierarchyTable(), type )
	
	/** Function to keep going up the hierarchy to test whether a certain type is a child of a another type */
	Bool HierarchyTable_IsChild( void* hierarchyTable, Type childType, Type possibleParentType ) ;

	/** Shortcut to above function - it grabs the pointer to the HierarchyTable singleton for you */
	#define IsChild( childType, possibleParentType ) \
		HierarchyTable_IsChild( HierarchyTable_GetHierarchyTable(), (childType), (possibleParentType) )

	/** Macro which checks to see if a specific pointer to an object is an instance of a class -
	 * i.e. it's type is a child of this 'possibleParentType' */
	#define Stg_Class_IsInstance( classPtr, possibleParentType )\
			IsChild( ((Stg_Class*) classPtr)->type, (possibleParentType) )

	#define Stg_CheckType( classPtr, possibleParentTypedef ) \
		(possibleParentTypedef*) Stg_Class_CheckType( classPtr, possibleParentTypedef ## _Type )

	#ifdef DEBUG
		#define Stg_DCheckType Stg_CheckType
	#else
		#define Stg_DCheckType( classPtr, possibleParentTypedef ) \
			(possibleParentTypedef*) (classPtr)
	#endif

	Stg_Class* Stg_Class_CheckType( void* classPtr, Type possibleParentType );

	void HierarchyTable_PrintParents( void* hierarchyTable, Type childType, Stream* stream ) ;
	void HierarchyTable_PrintChildren( void* hierarchyTable, Type parentType, Stream* stream ) ;

	#define PrintParents( childType, stream ) \
		HierarchyTable_PrintParents( HierarchyTable_GetHierarchyTable(), (childType), (stream) )
	#define PrintChildren( parentType, stream ) \
		HierarchyTable_PrintChildren( HierarchyTable_GetHierarchyTable(), (parentType), (stream) )

	Type HierarchyTable_GetTypePtrFromName( void* hierarchyTable, Name typeName ) ;

#endif /* __Base_Automation_HierarchyTable_h__ */
