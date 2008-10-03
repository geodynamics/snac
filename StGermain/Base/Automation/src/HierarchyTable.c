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
** $Id: HierarchyTable.c 3652 2006-06-27 01:11:50Z AlanLo $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdarg.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"

#include "types.h"
#include "shortcuts.h"
#include "Stg_Component.h"
#include "Stg_ComponentRegister.h"
#include "HierarchyTable.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* Textual name of this class */
const Type HierarchyTable_Type = "HierarchyTable";

/* Global pointer to singleton */
HierarchyTable* stgHierarchyTable = NULL;

HierarchyTable* HierarchyTable_New( void ) {
	HierarchyTable* self = NULL;

	if( stgHierarchyTable == NULL ){
		self = (HierarchyTable*) HashTable_New( NULL, NULL, NULL, HASHTABLE_POINTER_KEY );
	}
	else{
		self = stgHierarchyTable;
	}

	return self;
}
	
/* Private member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int HierarchyTable_RegisterParent( void* hierarchyTable, Type childType, Type parentType ) {
	HierarchyTable *self = (HierarchyTable*) hierarchyTable;

	return HashTable_InsertEntry( (HashTable*) self, childType, sizeof(const Type), parentType, sizeof(const Type) );
}

Bool HierarchyTable_IsChild( void* hierarchyTable, Type childType, Type possibleParentType ) {
	HierarchyTable* self     = (HierarchyTable*) hierarchyTable;
	Type            currType;
	
	/* Starting from 'childType', sweep through Hierarchy Table, going from parent to parent.
	 * Keep going until the Hierarchy Table produces a NULL - this means that we've reached the end of the table */
	for ( currType = childType ; currType != NULL ; currType = HierarchyTable_GetParent( self, currType ) ) {
		/* Test if this parent is the same as the one being tested for */
		if ( currType == possibleParentType )
			return True;
	}

	/* If we've gone through every parent in the table and reached the end - 
	 * then 'childType' cannot be a child of 'possibleParentType' */
	return False;
}

void HierarchyTable_PrintParents( void* hierarchyTable, Type childType, Stream* stream ) {
	HierarchyTable* self     = (HierarchyTable*) hierarchyTable;
	Type            parentType;

	Journal_Printf( stream, "Type '%s' inherits from:\n", childType );

	parentType = HierarchyTable_GetParent( self, childType );
	for ( ; parentType != NULL ; parentType = HierarchyTable_GetParent( self, parentType ) ) {
		Journal_Printf( stream, "\t%s\n", parentType );
	}
}

void HierarchyTable_PrintChildren( void* hierarchyTable, Type parentType, Stream* stream ) {
	HierarchyTable*    self     = (HierarchyTable*) hierarchyTable;
	HashTable_Entry*   hashTableEntry;
	Index              entry_I;
	Stg_ComponentRegister* componentRegister = Stg_ComponentRegister_Get_ComponentRegister();

	Journal_Printf( stream, "%s \t\t\t (%s Class)\n", 
			parentType, ( Stg_ComponentRegister_Get( componentRegister, parentType, "0" ) ? "Concrete" : "Abstract" ) );
	Stream_Flush( stream );
	Stream_Indent( stream );

	for ( entry_I = 0 ; entry_I < self->max ; entry_I++ ) {
		hashTableEntry = self->entries[ entry_I ];

		while( hashTableEntry ){
			if ( (Type) hashTableEntry->data == parentType ) {
				HierarchyTable_PrintChildren( self, (Type)hashTableEntry->key, stream );
			}
			hashTableEntry = hashTableEntry->next;
		}

	}
	Stream_UnIndent( stream );
}


Stg_Class* Stg_Class_CheckType( void* classPtr, Type possibleParentType ) {
	Stg_Class* self = (Stg_Class*) classPtr;
	/* Check if the pointer is null */
	Journal_Firewall(
		self != NULL,
		Journal_Register( Error_Type, HierarchyTable_Type ),
		"Error doing type checking against possibleParentType %s - pointer is NULL.\n",
		possibleParentType );
	
	if ( ! Stg_Class_IsInstance( self, possibleParentType ) ) {
		Stream* stream = Journal_Register( Error_Type, HierarchyTable_Type );
		
		Journal_Printf( stream, "Error doing type checking:\nTrying to cast " );

		/* Give the user the name of the object if you can */
		if ( Stg_Class_IsInstance( classPtr, Stg_Object_Type ) ) 
			Journal_Printf( stream, "'%s' ", ((Stg_Object*)classPtr)->name );
		else 		
			Journal_Printf( stream, "pointer '%p' ", classPtr );

		Journal_Printf( stream, 
				"as '%s' when it is actually of type '%s'.\n",
				possibleParentType, self->type );
		
		PrintParents( possibleParentType, stream );
		
		Journal_Firewall( 
			0,
			stream, 
			"Either the HierarchyTable for this class is incorrect or this object has been cast incorrectly.\n" );
	}
	return self;
}

Type HierarchyTable_GetTypePtrFromName( void* hierarchyTable, Name typeName ) {
	HierarchyTable*    self     = (HierarchyTable*) hierarchyTable;
	HashTable_Entry*   hashTableEntry;
	Index              entry_I;

	for ( entry_I = 0 ; entry_I < self->max ; entry_I++ ) {
		hashTableEntry = self->entries[ entry_I ];

		while( hashTableEntry ){
			if ( strcmp( (Type) hashTableEntry->key, typeName ) == 0 )
				return (Type) hashTableEntry->key;
		
			hashTableEntry = hashTableEntry->next;
		}

	}
	return NULL;
}
