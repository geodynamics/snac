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
** $Id: FileParticleLayout.c 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"
#include "Discretisation/Utils/Utils.h"

#include "types.h"
#include "shortcuts.h"
#include "ParticleLayout.h"
#include "GlobalParticleLayout.h"
#include "FileParticleLayout.h"

#include "SwarmClass.h"
#include "StandardParticle.h"
#include "CellLayout.h"
#include "ElementCellLayout.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

const Type FileParticleLayout_Type = "FileParticleLayout";

FileParticleLayout* FileParticleLayout_New( Name name, Name filename )
{
	FileParticleLayout* self = (FileParticleLayout*) _FileParticleLayout_DefaultNew( name );
	_FileParticleLayout_Init( self, filename );
	return self;
}


FileParticleLayout* _FileParticleLayout_New( 
                SizeT                                            _sizeOfSelf,
                Type                                             type,
                Stg_Class_DeleteFunction*                        _delete,
                Stg_Class_PrintFunction*                         _print,
                Stg_Class_CopyFunction*                          _copy,
                Stg_Component_DefaultConstructorFunction*        _defaultConstructor,
                Stg_Component_ConstructFunction*                 _construct,
                Stg_Component_BuildFunction*                     _build,
                Stg_Component_InitialiseFunction*                _initialise,
                Stg_Component_ExecuteFunction*                   _execute,
                Stg_Component_DestroyFunction*                   _destroy,
                ParticleLayout_SetInitialCountsFunction*         _setInitialCounts,
                ParticleLayout_InitialiseParticlesFunction*      _initialiseParticles,
                GlobalParticleLayout_InitialiseParticleFunction* _initialiseParticle,
                Name                                             name,
                Bool                                             initFlag,
                Name                                             filename )
{
	FileParticleLayout* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof( FileParticleLayout ) );
	self = (FileParticleLayout*)_GlobalParticleLayout_New( 
			_sizeOfSelf, 
			type,
			_delete,
			_print,
			_copy, 
			_defaultConstructor,
			_construct,
			_build,
			_initialise,
			_execute,
			_destroy,
			_setInitialCounts,
			_initialiseParticles,
			_initialiseParticle,
			name,
			initFlag,
			GlobalCoordSystem,
			False,
			0,
			0.0 );

	if ( initFlag ) {
		_FileParticleLayout_Init( self, filename );
	}

	return self;
}


void _FileParticleLayout_Init( void* particleLayout, Name filename )
{
	FileParticleLayout* self = (FileParticleLayout*) particleLayout;

	self->filename = StG_Strdup( filename );
	self->file        = NULL;
	self->errorStream = Journal_MyStream( Error_Type, self );
	_GlobalParticleLayout_Init( self, GlobalCoordSystem, False, 0, 0.0 );
}


void _FileParticleLayout_Delete( void* particleLayout ) {
	FileParticleLayout* self = (FileParticleLayout*)particleLayout;

	Memory_Free( self->filename );

	/* Stg_Class_Delete parent class */
	_GlobalParticleLayout_Delete( self );
}

void _FileParticleLayout_Print( void* particleLayout, Stream* stream ) {
	FileParticleLayout* self  = (FileParticleLayout*)particleLayout;
	
	/* General info */
	Journal_Printf( stream, "FileParticleLayout (ptr): %p:\n", self );
	Stream_Indent( stream );
	
	/* Parent class info */
	_GlobalParticleLayout_Print( self, stream );
	
	/* FileParticleLayout */
	Journal_Printf( stream, "filename: %s\n", self->filename );
	Journal_Printf( stream, "file (ptr): %p\n", self->file );
	
	Stream_UnIndent( stream );
}


void* _FileParticleLayout_Copy( void* particleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	FileParticleLayout*		self                    = (FileParticleLayout*)particleLayout;
	FileParticleLayout*		newFileParticleLayout;
	
	newFileParticleLayout = (FileParticleLayout*)_GlobalParticleLayout_Copy( self, dest, deep, nameExt, ptrMap );
	
	newFileParticleLayout->filename = self->filename;

	return (void*)newFileParticleLayout;
}

void* _FileParticleLayout_DefaultNew( Name name ) {
	return (void*)_FileParticleLayout_New( 
			sizeof(FileParticleLayout),
			FileParticleLayout_Type,
			_FileParticleLayout_Delete,
			_FileParticleLayout_Print,
			_FileParticleLayout_Copy,
			_FileParticleLayout_DefaultNew,
			_FileParticleLayout_Construct,
			_FileParticleLayout_Build,
			_FileParticleLayout_Initialise,
			_FileParticleLayout_Execute,
			_FileParticleLayout_Destroy,
			_FileParticleLayout_SetInitialCounts,
			_FileParticleLayout_InitialiseParticles,
			_FileParticleLayout_InitialiseParticle,
			name,
			False, 
			NULL /* filename */ );
}

void _FileParticleLayout_Construct( void* particleLayout, Stg_ComponentFactory *cf, void* data ) {
	FileParticleLayout* self     = (FileParticleLayout*) particleLayout;
	Name                filename;

	filename = Stg_ComponentFactory_GetString( cf, self->name, "filename", "Swarm.dat" );
	
	_FileParticleLayout_Init( self, filename );
}
	
void _FileParticleLayout_Build( void* particleLayout, void* data ) {
}
void _FileParticleLayout_Initialise( void* particleLayout, void* data ) {
}
void _FileParticleLayout_Execute( void* particleLayout, void* data ) {
}
void _FileParticleLayout_Destroy( void* particleLayout, void* data ) {
}

void _FileParticleLayout_SetInitialCounts( void* particleLayout, void* _swarm ) {
	FileParticleLayout*        self         = (FileParticleLayout*)particleLayout;
	Swarm*                     swarm        = (Swarm*)_swarm;
	Name                       filename     = self->filename;
	MPI_File                   mpiFile;
	int                        openResult;
	MPI_Offset                 bytesCount;
	SizeT                      particleSize = swarm->particleExtensionMgr->finalSize;
	div_t                      division;

	Journal_DPrintf( self->debug, "In %s(): for ParticleLayout \"%s\", of type %s\n",
		__func__, self->name, self->type );
	Stream_IndentBranch( Swarm_Debug );	

	Journal_DPrintf( self->debug, "Finding number of bytes in checkpoint file \"%s\":\n",
		self->filename );

	openResult = MPI_File_open( swarm->comm, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &mpiFile );

	Journal_Firewall( 
		openResult == 0, 
		self->errorStream,
		"Error in %s for %s '%s' - Cannot open file %s.\n", 
		__func__, 
		self->type, 
		self->name, 
		filename );
	
	MPI_File_get_size( mpiFile, &bytesCount );
	MPI_File_close( &mpiFile );

	Journal_DPrintf( self->debug, "...calculated bytes total of %u.\n", bytesCount );
	
	/* Divide by particle size to get number of particles */
	division = div( bytesCount, particleSize );
	self->totalInitialParticles = division.quot;

	Journal_DPrintf( self->debug, "given bytes total %u / particle size %u ->\n"
		"\ttotalInitialParticles = %u.\n", bytesCount, (unsigned int)particleSize,
		self->totalInitialParticles );

	Journal_Firewall( 
		division.rem == 0,
		self->errorStream,
		"Error in func %s for %s '%s' - Trying to read particle information from %s which stores %u bytes.\n"
		"This doesn't produce an integer number of particles of size %u - It gives remainder = %u\n", 
		__func__, 
		self->type, 
		self->name, 
		filename, 
		bytesCount, 
		(unsigned int)particleSize, 
		division.rem ); 

	Journal_DPrintf( self->debug, "calling parent func to set cell counts:\n", bytesCount );
	_GlobalParticleLayout_SetInitialCounts( self, swarm );

	Stream_UnIndentBranch( Swarm_Debug );	
	Journal_DPrintf( self->debug, "...finished %s() for ParticleLayout \"%s\".\n",
		__func__, self->name );
}

void _FileParticleLayout_InitialiseParticles( void* particleLayout, void* _swarm ) {
	FileParticleLayout*        self             = (FileParticleLayout*)particleLayout;
	
	self->file = fopen( self->filename, "rb" );
	Journal_Firewall( 
		self->file != NULL, 
		self->errorStream,
		"Error in %s for %s '%s' - Cannot open file %s.\n", 
		__func__, 
		self->type, 
		self->name, 
		self->filename );

	_GlobalParticleLayout_InitialiseParticles( self, _swarm );
	
	fclose( self->file );
	self->file = NULL;
}	
	
void _FileParticleLayout_InitialiseParticle( 
		void*              particleLayout, 
		void*              _swarm, 
		Particle_Index     newParticle_I,
		void*              particle )
{
	FileParticleLayout*        self             = (FileParticleLayout*)particleLayout;
	Swarm*                     swarm            = (Swarm*)_swarm;
	SizeT                      particleSize     = swarm->particleExtensionMgr->finalSize;
	int                        result;

	result = fread( particle, particleSize, 1, self->file );

	Journal_Firewall( 
		result == 1,
		self->errorStream,
		"Error in func %s for %s '%s':\n"
		"\tCouldn't read in particle %u - May have reached end-of-file.\n",
		__func__, 
		self->type, 
		self->name, 
		newParticle_I );
}
		



