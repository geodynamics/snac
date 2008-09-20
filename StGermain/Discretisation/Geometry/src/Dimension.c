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
** $Id: Dimension.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "units.h"
#include "types.h"
#include "Dimension.h"

void Dimension_3DTo1D_3_Func( Index dim0, Index dim1, Index dim2, 
		Index dim0Size, Index dim1Size, Index dim2Size, 
		Index* outputIndexPtr )
{		
	Index dimSizes[3];
	dimSizes[0] = (dim0Size) ? (dim0Size) : 1;
	dimSizes[1] = (dim1Size) ? (dim1Size) : 1;
	dimSizes[2] = (dim2Size) ? (dim2Size) : 1;
	
	#if DEBUG 
	{
		Stream* dimensionError = Journal_Register( Error_Type, "DimensionMacros" );
		Journal_Firewall( (dim0 < dimSizes[0]), dimensionError, "Error: Given coordinate[0] value %d >= "
			"regionSize[0] %d\n", dim0, dimSizes[0] );
		Journal_Firewall( (dim1 < dimSizes[1]), dimensionError, "Error: Given coordinate[1] value %d >= "
			"regionSize[1] %d\n", dim1, dimSizes[1] );
		Journal_Firewall( (dim2 < dimSizes[2]), dimensionError, "Error: Given coordinate[2] value %d >= "
			"regionSize[2] %d\n", dim2, dimSizes[2] );
	}	
	#endif
	
	Dimension_3DTo1D_3_Macro( dim0, dim1, dim2, dim0Size, dim1Size, dim2Size, outputIndexPtr );
}	

void Dimension_1DTo3D_3_Func( Index index, Index dim0Size, Index dim1Size, Index dim2Size,
		Index* dim0Ptr, Index* dim1Ptr, Index* dim2Ptr ) 
{
	Dimension_1DTo3D_3_Macro( index, dim0Size, dim1Size, dim2Size, dim0Ptr, dim1Ptr, dim2Ptr );
}
