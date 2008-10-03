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
** $Id: AbstractContext.c 3562 2006-05-11 10:43:48Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdarg.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"
#include "Base/Automation/Automation.h"
#include "Base/Extensibility/Extensibility.h"

#include "units.h"
#include "types.h"
#include "shortcuts.h"
#include "ContextEntryPoint.h"
#include <mpi.h>  /* subsequent files need this */
#include "DictionaryCheck.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>



void CheckDictionaryKeys( Dictionary* dictionary, char* errorMessage)
{
	Dictionary_Index index_I, index_J;
	/* Put in the Journal_Firewall stream */
	Stream* errStream = Journal_Register( Error_Type, "DictionaryCheck");
	int errCount;
	int** keyIndexArray;


	keyIndexArray = Memory_Alloc_2DArray( int, ((dictionary->count)*(dictionary->count - 1)), 
					2, "Key Index Array" );
	/* Iterate through the whole dictionary*/
	errCount = 0;
	for ( index_I = 0; index_I < dictionary->count; ++index_I )
	{
		/*For Each key, search through dictionary to see if there is another 
		key the same*/
		for (index_J = index_I+1; index_J < dictionary->count; ++index_J ) {
			if (index_J != index_I)
			{
				/* If there are two keys with the same name */
				if ( (0 == strcasecmp( dictionary->entryPtr[index_I]->key, 
					dictionary->entryPtr[index_J]->key)) )
				{

					//preserve indexes index_I, index_J
					keyIndexArray[errCount][0] = index_I;
					keyIndexArray[errCount][1] = index_J;					
					//increment counter
					errCount++;
					
				}
			}
			
		}
	}
	/*if keyIndexArray is not empty, then do a print to error stream 
	for each problem then call Journal_Firewall */
	

	if (errCount > 0) {
		Index errIndex;
		Journal_Printf(errStream, errorMessage);
		Journal_Printf(errStream,"Error found in dictionary (ptr) %p:\n",dictionary);
		Journal_Printf(errStream,"The following keys were repeated:\n");
		Stream_Indent(errStream);
		for (errIndex = 0; errIndex < errCount; errIndex++) {

			Journal_Printf(errStream, "\"%s\"\n",					 
				dictionary->entryPtr[keyIndexArray[errIndex][1]]->key );
		}
		//Do I need this one here if Journal_Firewall exits prog?
		Stream_UnIndent(errStream);
		Journal_Firewall(errCount == 0, errStream, 
					"Error in %s with %d entries in dictionary keys\n",
					__func__, errCount);
		
	}
	Memory_Free(keyIndexArray);
}
