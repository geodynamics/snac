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
**	External header file to this library.
**
** Assumptions:
**	None so far.
**
** Comments:
**	None so far.
**
** $Id: Geometry.h 3728 2006-07-31 03:14:51Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Geometry_h__
#define __Discretisation_Geometry_h__
	
	#include "units.h"
	#include "types.h"
	#include "isinf.h"
	#include "Dimension.h"
	#include "VectorMath.h"
	#include "TensorMath.h"
	#include "TrigMath.h"
	#include "ComplexMath.h"
	#include "Plane.h"
	#include "Edge.h"
	#include "Line.h"
	#include "RMatrix.h"
	#include "Topology.h"
	#include "IJKTopology.h"
	#include "IJK6Topology.h"
	#include "IJK26Topology.h"
	#include "IrregTopology.h"
	#include "GeometryClass.h"
	#include "BlockGeometry.h"
	#include "RefinedRegionsGeometry.h"
	#include "ShellGeometry.h"
	#include "IrregGeometry.h"
	#include "QuadEdge.h"
	#include "Delaunay.h"
	#include "ParallelDelaunay.h"
	#include "ComplexVectorMath.h"
	#include "stg_lapack.h"
	#include "FullTensorMath.h"
	#include "TensorMultMath.h"
	#include "Init.h"
	#include "Finalise.h"

#endif /* __Discretisation_Geometry_h__ */
