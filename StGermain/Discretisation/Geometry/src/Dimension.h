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
** Role:
**	Functions for converting from 1D indices to 3D co-ordinate numbers,
**	and back again.
**
** Assumptions:
**
** Comments:
**	These macros should be used anywhere where IJK-style conversions are 
**	required. It was created since the FeEquationNumber needed to use 
**	new co-ordinate systems.
**
** $Id: Dimension.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Geometry_Dimension_h__
#define __Discretisation_Geometry_Dimension_h__

	/** Converts 3 3d coordinates into the sequential number for a given sized region. */ 
	#define Dimension_3DTo1D_3_Macro( dim0, dim1, dim2, dim0Size, dim1Size, dim2Size, indexPtr ) \
	do { \
		Index dimSizes[3]; \
		dimSizes[0] = (dim0Size) ? (dim0Size) : 1; \
		dimSizes[1] = (dim1Size) ? (dim1Size) : 1; \
		dimSizes[2] = (dim2Size) ? (dim2Size) : 1; \
		\
		*(indexPtr) = ( ((dim2)*dimSizes[0]*dimSizes[1]) + ((dim1)*dimSizes[0]) + (dim0) ); \
	} while (0) 
	
	void Dimension_3DTo1D_3_Func( Index dim0, Index dim1, Index dim2, 
		Index dim0Size, Index dim1Size, Index dim2Size, 
		Index* outputIndexPtr );

	#ifdef MACRO_AS_FUNC
		#define Dimension_3DTo1D_3 Dimension_3DTo1D_3_Func
	#else	
		#define Dimension_3DTo1D_3 Dimension_3DTo1D_3_Macro
	#endif	


	/** Converts a 3d coordinate into the sequential number for a given sized region. */ 
	#define Dimension_3DTo1D( coord, regionSizes, indexPtr ) \
		Dimension_3DTo1D_3( (coord)[0], (coord)[1], (coord)[2], (regionSizes)[0], (regionSizes)[1], (regionSizes)[2], indexPtr )

	/** Converts a sequential number into the 3d co-ordinates for a given region, returned separately */ 
	#define Dimension_1DTo3D_3_Macro( index, dim0Size, dim1Size, dim2Size, dim0Ptr, dim1Ptr, dim2Ptr ) \
	do { \
		Index dimSizes[3]; \
		dimSizes[0] = (dim0Size) ? (dim0Size) : 1; \
		dimSizes[1] = (dim1Size) ? (dim1Size) : 1; \
		dimSizes[2] = (dim2Size) ? (dim2Size) : 1; \
		\
		*(dim0Ptr) = (index) % (dimSizes[0]); \
		*(dim1Ptr) = ((index)/(dimSizes[0])) % (dimSizes[1]); \
		*(dim2Ptr) = ((index)/((dimSizes[0])*(dimSizes[1]))) % (dimSizes[2]); \
	} while (0) 

	void Dimension_1DTo3D_3_Func( Index index, Index dim0Size, Index dim1Size, Index dim2Size,
		Index* dim0Ptr, Index* dim1Ptr, Index* dim2Ptr );
	#ifdef MACRO_AS_FUNC
		#define Dimension_1DTo3D_3 Dimension_1DTo3D_3_Func
	#else
		#define Dimension_1DTo3D_3 Dimension_1DTo3D_3_Macro
	#endif

		
	/** Converts a sequential number into the 3d co-ordinates for a given region, returned as one */ 
	#define Dimension_1DTo3D( index, regionSizes, coord ) \
		Dimension_1DTo3D_3( index, (regionSizes)[0], (regionSizes)[1], (regionSizes)[2], &(coord)[0], &(coord)[1], &(coord)[2] )

#endif
