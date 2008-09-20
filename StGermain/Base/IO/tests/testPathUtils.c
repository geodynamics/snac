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
** $Id: testLibStGermainDynamic.c 2742 2005-03-05 05:33:43Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdlib.h>
#include <string.h>

#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"


int main( int argc, char* argv[] ) {
	char		searchPaths[1024];
	char		fullPath[256];
	unsigned	len;
	
	PathJoin( searchPaths, 1, "." );
	len = strlen( searchPaths );
	searchPaths[len] = ':';
	
	PathJoin( &searchPaths[len + 1], 2,  ".", "data" );
	len = strlen( searchPaths );
	searchPaths[len] = ':';
	
	PathJoin( &searchPaths[len + 1], 6, "..", "..", "..", "does", "not", "exist" );
	
	printf( "searchPaths: %s\n", searchPaths );
	
	/* try and open some files using the search path */
	FindFile( fullPath, "testPathUtils.c", searchPaths );
	printf( "%s\n", fullPath );
	
	FindFile( fullPath, "normal.xml", searchPaths );
	printf( "%s\n", fullPath );
	
	FindFile( fullPath, "nofile.man", searchPaths );
	printf( "%s\n", fullPath );
	
	FindFile( fullPath, "/Users/luke/Projects/StGermain/env_vars", searchPaths );
	printf( "%s\n", fullPath );
	
	return EXIT_SUCCESS;
}
