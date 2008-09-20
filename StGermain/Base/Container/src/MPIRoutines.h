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
/** \file
** <b>Role:</b>
**	Collection of commonly used functions	
**
** <b>Assumptions:</b>
**	None
**
** <b>Comments:</b>
**	None
**
** $Id: MPIRoutines.h 2276 2004-11-04 02:01:18Z AlanLo $
**
**/

#ifndef __Base_Foundation_MPIRoutines_h__
#define __Base_Foundation_MPIRoutines_h__


void MPIArray_Bcast( unsigned* arraySize, void** array, size_t itemSize, 
		     unsigned root, MPI_Comm comm );

void MPIArray_Gather( unsigned arraySize, void* array, 
		      unsigned** dstSizes, void*** dstArrays, 
		      size_t itemSize, unsigned root, MPI_Comm comm );

void MPIArray_Allgather( unsigned arraySize, void* array, 
			 unsigned** dstSizes, void*** dstArrays, 
			 size_t itemSize, MPI_Comm comm );

void MPIArray2D_Alltoall( unsigned* arraySizes, void** arrays, 
			  unsigned** dstSizes, void*** dstArrays, 
			  size_t itemSize, MPI_Comm comm );

void Array_1DTo2D( unsigned nBlocks, unsigned* sizes, void* srcArray, 
		   void*** dstArrays, size_t itemSize );

void Array_2DTo1D( unsigned nBlocks, unsigned* sizes, void** srcArrays, 
		   void** dstArray, size_t itemSize, unsigned** disps );


#endif /* __Base_Foundation_MPIRoutines_h__ */
