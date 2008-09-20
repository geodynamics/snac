/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003,
**	Steve Quenette, 110 Victoria Street, Melbourne, Victoria, 3053, Australia.
**	Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
**	University of Texas, 1 University Station, Austin, Texas, 78712, USA.
**
** Authors:
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**	Stevan M. Quenette, Visitor in Geophysics, Caltech.
**	Luc Lavier, Research Scientist, The University of Texas. (luc@utig.ug.utexas.edu)
**	Luc Lavier, Research Scientist, Caltech.
**
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2, or (at your option) any
** later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**
*/
/** \file
** Role:
**
** Assumptions:
**	regular hex mesh
**
** Comments:
**
** $Id$
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Snac_Parallel_h__
#define __Snac_Parallel_h__

	/* Textual name of this class */
	extern const Type Snac_Parallel_Type;

	/* Snac_Parallel info */
	#define __Snac_Parallel \
		/* General info */ \
		__Stg_Class \
		\
		/* Virtual info */ \
		\
		/* Snac_Parallel info */ \
		MPI_Comm			communicator; \
		Partition_Index			nproc; \
		Partition_Index			rank; \
		Partition_Index			rnCount; \
		Partition_Index*		rn; \
		Partition_Index			decompDimCount; \
		Partition_Index			decompRankCount[3]; \
		IndexSet**			boundarySet; \
		Index**				boundaryArray; \
		IndexSet_Index*			boundaryCount; \
		Force**				boundaryForce; \
		Force**				boundaryForceRemote; \
		Mass**				boundaryIMass; \
		Mass**				boundaryIMassRemote; \
		\
		Mesh*				mesh; \
		\
		Stream*				info; \
		Stream*				debug;

	struct _Snac_Parallel { __Snac_Parallel };


	/* Create a new Snac_Parallel and initialise */
	Snac_Parallel* Snac_Parallel_New( MPI_Comm communicator, Mesh* mesh );

	/* Initialise a Snac_Parallel */
	void Snac_Parallel_Init( Snac_Parallel* self, MPI_Comm communicator, Mesh* mesh );

	/* Creation implementation / Virtual constructor */
	Snac_Parallel* _Snac_Parallel_New(
		SizeT					_sizeOfSelf,
		Type					type,
		Stg_Class_DeleteFunction*			_delete,
		Stg_Class_PrintFunction*			_print,
		Stg_Class_CopyFunction*			_copy, 
		MPI_Comm				communicator, 
		Mesh*					mesh );

	/* Initialisation implementation */
	void _Snac_Parallel_Init( Snac_Parallel* self, MPI_Comm communicator, Mesh* mesh );


	/* Stg_Class_Delete implementation */
	void _Snac_Parallel_Delete( void* context );

	/* Print implementation */
	void Snac_Parallel_Print( void* context, Stream* stream );


	/* Build the Snac implementation */
	void Snac_Parallel_Build( void* context );

#endif /* __Snac_Parallel_h__ */
