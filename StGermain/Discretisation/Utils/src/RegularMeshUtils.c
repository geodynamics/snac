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
** $Id: RegularMeshUtils.c 3574 2006-05-15 11:30:33Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"

#include "types.h"
#include "RegularMeshUtils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


Index RegularMeshUtils_ascendingIJK_ToHughesNodeNumberMap[8] = { 0, 1, 3, 2, 4, 5, 7, 6 };


/*----------------------------------------------------------------------------------------------------------------------------------
** Mapping functions
*/

Node_LocalIndex RegularMeshUtils_Node_Global3DToLocal1D( HexaMD* hexaMD, Index i, Index j, Index k ) {		
	Dimension_Index        dim_I;
	IJK                    ijk = { i, j, k };

	for ( dim_I = 0; dim_I < 3; dim_I++ ) {
		if ( ijk[dim_I] < (hexaMD)->_nodeOffsets[(hexaMD)->rank][dim_I] ||
			ijk[dim_I]  >= ((hexaMD)->_nodeOffsets[(hexaMD)->rank][dim_I]
				+ (hexaMD)->nodeLocal3DCounts[(hexaMD)->rank][dim_I] ) )
		{
			return MD_N_Invalid( hexaMD );
		}
	}
	
	return RegularMeshUtils_Node_Local3DTo1D( hexaMD,
		i - (hexaMD)->_nodeOffsets[(hexaMD)->rank][I_AXIS],
		j - (hexaMD)->_nodeOffsets[(hexaMD)->rank][J_AXIS],
		k - (hexaMD)->_nodeOffsets[(hexaMD)->rank][K_AXIS] );
}			

/*----------------------------------------------------------------------------------------------------------------------------------
** Set functions
*/

/* TODO: There are more efficient ways to do the below, given me know the number of nodes in each direction */

IndexSet* RegularMeshUtils_CreateGlobalTopSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
	Node_DomainIndex	node_I;
	Node_DomainIndex	nodeDomainCount;
	IndexSet*		is;
	
	nodeDomainCount = decomp->nodeDomainCount;
	is = IndexSet_New( nodeDomainCount );
	for( node_I = 0; node_I < nodeDomainCount; node_I++ ) {
		IJK			ijk;
		
		RegularMeshUtils_Node_1DTo3D( decomp, Mesh_NodeMapDomainToGlobal( mesh, node_I ),
			&ijk[0], &ijk[1], &ijk[2] );
		if( ijk[1] == ( decomp->nodeGlobal3DCounts[1] - 1 ) ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}

IndexSet* RegularMeshUtils_CreateGlobalBottomSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
	Node_DomainIndex	node_I;
	Node_DomainIndex	nodeDomainCount;
	IndexSet*		is;
	
	nodeDomainCount = decomp->nodeDomainCount;
	is = IndexSet_New( nodeDomainCount );
	for( node_I = 0; node_I < nodeDomainCount; node_I++ ) {
		IJK			ijk;
		
		RegularMeshUtils_Node_1DTo3D( decomp, Mesh_NodeMapDomainToGlobal( mesh, node_I ),
			&ijk[0], &ijk[1], &ijk[2] );      
		if( ijk[1] == 0 ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}

IndexSet* RegularMeshUtils_CreateGlobalLeftSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
	Node_DomainIndex	node_I;
	Node_DomainIndex	nodeDomainCount;
	IndexSet*		is;
	
	nodeDomainCount = decomp->nodeDomainCount;
	is = IndexSet_New( nodeDomainCount );
	for( node_I = 0; node_I < nodeDomainCount; node_I++ ) {
		IJK			ijk;
		
		RegularMeshUtils_Node_1DTo3D( decomp, Mesh_NodeMapDomainToGlobal( mesh, node_I ), 
			&ijk[0], &ijk[1], &ijk[2] );
		if( ijk[0] == 0 ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}

IndexSet* RegularMeshUtils_CreateGlobalRightSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
	Node_DomainIndex	node_I;
	Node_DomainIndex	nodeDomainCount;
	IndexSet*		is;
	
	nodeDomainCount = decomp->nodeDomainCount;
	is = IndexSet_New( nodeDomainCount );
	for( node_I = 0; node_I < nodeDomainCount; node_I++ ) {
		IJK			ijk;
		
		RegularMeshUtils_Node_1DTo3D( decomp, Mesh_NodeMapDomainToGlobal( mesh, node_I ), 
			&ijk[0], &ijk[1], &ijk[2] );      
		if( ijk[0] == ( decomp->nodeGlobal3DCounts[0] - 1 ) ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}

IndexSet* RegularMeshUtils_CreateGlobalFrontSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
	Node_DomainIndex	node_I;
	Node_DomainIndex	nodeDomainCount;
	IndexSet*		is;
	
	nodeDomainCount = decomp->nodeDomainCount;
	is = IndexSet_New( nodeDomainCount );
	for( node_I = 0; node_I < nodeDomainCount; node_I++ ) {
		IJK			ijk;
		
		RegularMeshUtils_Node_1DTo3D( decomp, Mesh_NodeMapDomainToGlobal( mesh, node_I ), 
			&ijk[0], &ijk[1], &ijk[2] );      
		if( ijk[2] == ( decomp->nodeGlobal3DCounts[2] - 1 ) ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}

IndexSet* RegularMeshUtils_CreateGlobalBackSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
	Node_DomainIndex	node_I;
	Node_DomainIndex	nodeDomainCount;
	IndexSet*		is;
	
	nodeDomainCount = decomp->nodeDomainCount;
	is = IndexSet_New( nodeDomainCount );
	for( node_I = 0; node_I < nodeDomainCount; node_I++ ) {
		IJK			ijk;
		
		RegularMeshUtils_Node_1DTo3D( decomp, Mesh_NodeMapDomainToGlobal( mesh, node_I ), 
			&ijk[0], &ijk[1], &ijk[2] );      
		if( ijk[2] == 0 ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}



IndexSet* RegularMeshUtils_CreateGlobalInnerTopSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
	Node_DomainIndex	node_I;
	Node_DomainIndex	nodeDomainCount;
	IndexSet*		is;
	
	nodeDomainCount = decomp->nodeDomainCount;
	is = IndexSet_New( nodeDomainCount );
	for( node_I = 0; node_I < nodeDomainCount; node_I++ ) {
		IJK			ijk;
		
		RegularMeshUtils_Node_1DTo3D( decomp, Mesh_NodeMapDomainToGlobal( mesh, node_I ),
			&ijk[0], &ijk[1], &ijk[2] );
		if( ijk[1] == ( decomp->nodeGlobal3DCounts[1] - 1 ) 
			&& ( ijk[0] != ( decomp->nodeGlobal3DCounts[0] -1 ) || ijk[2] != ( decomp->nodeGlobal3DCounts[2] - 1 ) )
			&& ( ijk[0] != 0 || ijk[2] != ( decomp->nodeGlobal3DCounts[2] - 1 ) )
			&& ( ijk[0] != ( decomp->nodeGlobal3DCounts[0] -1 ) || ijk[2] != 0 )
			&& ( ijk[0] != 0 || ijk[2] != 0 ) ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}

IndexSet* RegularMeshUtils_CreateGlobalInnerBottomSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
	Node_DomainIndex	node_I;
	Node_DomainIndex	nodeDomainCount;
	IndexSet*		is;
	
	nodeDomainCount = decomp->nodeDomainCount;
	is = IndexSet_New( nodeDomainCount );
	for( node_I = 0; node_I < nodeDomainCount; node_I++ ) {
		IJK			ijk;
		
		RegularMeshUtils_Node_1DTo3D( decomp, Mesh_NodeMapDomainToGlobal( mesh, node_I ),
			&ijk[0], &ijk[1], &ijk[2] );      
		if( ijk[1] == 0 
			&& ( ijk[0] != ( decomp->nodeGlobal3DCounts[0] - 1 ) || ijk[2] != ( decomp->nodeGlobal3DCounts[2] - 1 ) )
			&& ( ijk[0] != 0 || ijk[2] != ( decomp->nodeGlobal3DCounts[2] - 1 ) )
			&& ( ijk[0] != ( decomp->nodeGlobal3DCounts[0] - 1 ) || ijk[2] != 0 )
			&& ( ijk[0] != 0 || ijk[2] != 0 ) ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}

IndexSet* RegularMeshUtils_CreateGlobalInnerLeftSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
	Node_DomainIndex	node_I;
	Node_DomainIndex	nodeDomainCount;
	IndexSet*		is;
	
	nodeDomainCount = decomp->nodeDomainCount;
	is = IndexSet_New( nodeDomainCount );
	for( node_I = 0; node_I < nodeDomainCount; node_I++ ) {
		IJK			ijk;
		
		RegularMeshUtils_Node_1DTo3D( decomp, Mesh_NodeMapDomainToGlobal( mesh, node_I ), 
			&ijk[0], &ijk[1], &ijk[2] );
		if( ijk[0] == 0 
			&& ( ijk[1] != ( decomp->nodeGlobal3DCounts[1] - 1 ) || ijk[2] != ( decomp->nodeGlobal3DCounts[2] - 1 ) )
			&& ( ijk[1] != 0 || ijk[2] != ( decomp->nodeGlobal3DCounts[2] - 1 ) )
			&& ( ijk[1] != ( decomp->nodeGlobal3DCounts[1] - 1 ) || ijk[2] != 0 )
			&& ( ijk[1] != 0 || ijk[2] != 0 ) ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}

IndexSet* RegularMeshUtils_CreateGlobalInnerRightSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
	Node_DomainIndex	node_I;
	Node_DomainIndex	nodeDomainCount;
	IndexSet*		is;
	
	nodeDomainCount = decomp->nodeDomainCount;
	is = IndexSet_New( nodeDomainCount );
	for( node_I = 0; node_I < nodeDomainCount; node_I++ ) {
		IJK			ijk;
		
		RegularMeshUtils_Node_1DTo3D( decomp, Mesh_NodeMapDomainToGlobal( mesh, node_I ), 
			&ijk[0], &ijk[1], &ijk[2] );      
		if( ijk[0] == ( decomp->nodeGlobal3DCounts[0] - 1 ) 
			&& ( ijk[1] != ( decomp->nodeGlobal3DCounts[1] - 1 ) || ijk[2] != ( decomp->nodeGlobal3DCounts[2] - 1 ) )
			&& ( ijk[1] != 0 || ijk[2] != ( decomp->nodeGlobal3DCounts[2] - 1 ) )
			&& ( ijk[1] != ( decomp->nodeGlobal3DCounts[1] - 1 ) || ijk[2] != 0 )
			&& ( ijk[1] != 0 || ijk[2] != 0 ) ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}

IndexSet* RegularMeshUtils_CreateGlobalInnerFrontSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
	Node_DomainIndex	node_I;
	Node_DomainIndex	nodeDomainCount;
	IndexSet*		is;
	
	nodeDomainCount = decomp->nodeDomainCount;
	is = IndexSet_New( nodeDomainCount );
	for( node_I = 0; node_I < nodeDomainCount; node_I++ ) {
		IJK			ijk;
		
		RegularMeshUtils_Node_1DTo3D( decomp, Mesh_NodeMapDomainToGlobal( mesh, node_I ), 
			&ijk[0], &ijk[1], &ijk[2] );      
		if( ijk[2] == ( decomp->nodeGlobal3DCounts[2] - 1 ) 
			&& ( ijk[0] != ( decomp->nodeGlobal3DCounts[0] - 1 ) || ijk[1] != ( decomp->nodeGlobal3DCounts[1] - 1 ) )
			&& ( ijk[0] != 0 || ijk[1] != ( decomp->nodeGlobal3DCounts[1] - 1 ) )
			&& ( ijk[0] != ( decomp->nodeGlobal3DCounts[0] - 1 ) || ijk[1] != 0 )
			&& ( ijk[0] != 0 || ijk[1] != 0 ) ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}

IndexSet* RegularMeshUtils_CreateGlobalInnerBackSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
	Node_DomainIndex	node_I;
	Node_DomainIndex	nodeDomainCount;
	IndexSet*		is;
	
	nodeDomainCount = decomp->nodeDomainCount;
	is = IndexSet_New( nodeDomainCount );
	for( node_I = 0; node_I < nodeDomainCount; node_I++ ) {
		IJK			ijk;
		
		RegularMeshUtils_Node_1DTo3D( decomp, Mesh_NodeMapDomainToGlobal( mesh, node_I ), 
			&ijk[0], &ijk[1], &ijk[2] );      
		if( ijk[2] == 0 
			&& ( ijk[0] != ( decomp->nodeGlobal3DCounts[0] - 1 ) || ijk[1] != ( decomp->nodeGlobal3DCounts[1] - 1 ) )
			&& ( ijk[0] != 0 || ijk[1] != ( decomp->nodeGlobal3DCounts[1] - 1 ) )
			&& ( ijk[0] != ( decomp->nodeGlobal3DCounts[0] - 1 ) || ijk[1] != 0 )
			&& ( ijk[0] != 0 || ijk[1] != 0 ) ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}


IndexSet* RegularMeshUtils_CreateGlobalBottomLeftFrontSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
	Node_DomainIndex	node_I;
	Node_DomainIndex	nodeDomainCount;
	IndexSet*		is;
	
	nodeDomainCount = decomp->nodeDomainCount;
	is = IndexSet_New( nodeDomainCount );
	for( node_I = 0; node_I < nodeDomainCount; node_I++ ) {
		IJK			ijk;
		
		RegularMeshUtils_Node_1DTo3D( decomp, Mesh_NodeMapDomainToGlobal( mesh, node_I ), 
			&ijk[0], &ijk[1], &ijk[2] );      
		if( ijk[0] == 0 
			&& ijk[1] == 0
			&& ijk[2] == ( decomp->nodeGlobal3DCounts[2] - 1 ) ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}	
IndexSet* RegularMeshUtils_CreateGlobalBottomRightFrontSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
	Node_DomainIndex	node_I;
	Node_DomainIndex	nodeDomainCount;
	IndexSet*		is;
	
	nodeDomainCount = decomp->nodeDomainCount;
	is = IndexSet_New( nodeDomainCount );
	for( node_I = 0; node_I < nodeDomainCount; node_I++ ) {
		IJK			ijk;
		
		RegularMeshUtils_Node_1DTo3D( decomp, Mesh_NodeMapDomainToGlobal( mesh, node_I ), 
			&ijk[0], &ijk[1], &ijk[2] );      
		if( ijk[0] == ( decomp->nodeGlobal3DCounts[0] - 1 )
			&& ijk[1] == 0
			&& ijk[2] == (decomp->nodeGlobal3DCounts[2] - 1 ) ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}
IndexSet* RegularMeshUtils_CreateGlobalTopLeftFrontSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
	Node_DomainIndex	node_I;
	Node_DomainIndex	nodeDomainCount;
	IndexSet*		is;
	
	nodeDomainCount = decomp->nodeDomainCount;
	is = IndexSet_New( nodeDomainCount );
	for( node_I = 0; node_I < nodeDomainCount; node_I++ ) {
		IJK			ijk;
		
		RegularMeshUtils_Node_1DTo3D( decomp, Mesh_NodeMapDomainToGlobal( mesh, node_I ), 
			&ijk[0], &ijk[1], &ijk[2] );      
		if( ijk[0] == 0 
			&& ijk[1] == ( decomp->nodeGlobal3DCounts[1] - 1 )
			&& ijk[2] == ( decomp->nodeGlobal3DCounts[2] - 1 ) ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}
IndexSet* RegularMeshUtils_CreateGlobalTopRightFrontSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
	Node_DomainIndex	node_I;
	Node_DomainIndex	nodeDomainCount;
	IndexSet*		is;
	
	nodeDomainCount = decomp->nodeDomainCount;
	is = IndexSet_New( nodeDomainCount );
	for( node_I = 0; node_I < nodeDomainCount; node_I++ ) {
		IJK			ijk;
		
		RegularMeshUtils_Node_1DTo3D( decomp, Mesh_NodeMapDomainToGlobal( mesh, node_I ), 
			&ijk[0], &ijk[1], &ijk[2] );      
		if( ijk[0] == ( decomp->nodeGlobal3DCounts[0] - 1 )
			&& ijk[1] == ( decomp->nodeGlobal3DCounts[1] - 1 )
			&& ijk[2] == ( decomp->nodeGlobal3DCounts[2] - 1 ) ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}
IndexSet* RegularMeshUtils_CreateGlobalBottomLeftBackSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
	Node_DomainIndex	node_I;
	Node_DomainIndex	nodeDomainCount;
	IndexSet*		is;
	
	nodeDomainCount = decomp->nodeDomainCount;
	is = IndexSet_New( nodeDomainCount );
	for( node_I = 0; node_I < nodeDomainCount; node_I++ ) {
		IJK			ijk;
		
		RegularMeshUtils_Node_1DTo3D( decomp, Mesh_NodeMapDomainToGlobal( mesh, node_I ), 
			&ijk[0], &ijk[1], &ijk[2] );      
		if( ijk[0] == 0 
			&& ijk[1] == 0
			&& ijk[2] == 0 ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}
IndexSet* RegularMeshUtils_CreateGlobalBottomRightBackSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
	Node_DomainIndex	node_I;
	Node_DomainIndex	nodeDomainCount;
	IndexSet*		is;
	
	nodeDomainCount = decomp->nodeDomainCount;
	is = IndexSet_New( nodeDomainCount );
	for( node_I = 0; node_I < nodeDomainCount; node_I++ ) {
		IJK			ijk;
		
		RegularMeshUtils_Node_1DTo3D( decomp, Mesh_NodeMapDomainToGlobal( mesh, node_I ), 
			&ijk[0], &ijk[1], &ijk[2] );      
		if( ijk[0] == ( decomp->nodeGlobal3DCounts[0] - 1 )
			&& ijk[1] == 0
			&& ijk[2] == 0 ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}
IndexSet* RegularMeshUtils_CreateGlobalTopLeftBackSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
	Node_DomainIndex	node_I;
	Node_DomainIndex	nodeDomainCount;
	IndexSet*		is;
	
	nodeDomainCount = decomp->nodeDomainCount;
	is = IndexSet_New( nodeDomainCount );
	for( node_I = 0; node_I < nodeDomainCount; node_I++ ) {
		IJK			ijk;
		
		RegularMeshUtils_Node_1DTo3D( decomp, Mesh_NodeMapDomainToGlobal( mesh, node_I ), 
			&ijk[0], &ijk[1], &ijk[2] );      
		if( ijk[0] == 0
			&& ijk[1] == ( decomp->nodeGlobal3DCounts[1] - 1 )
			&& ijk[2] == 0 ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}
IndexSet* RegularMeshUtils_CreateGlobalTopRightBackSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
	Node_DomainIndex	node_I;
	Node_DomainIndex	nodeDomainCount;
	IndexSet*		is;
	
	nodeDomainCount = decomp->nodeDomainCount;
	is = IndexSet_New( nodeDomainCount );
	for( node_I = 0; node_I < nodeDomainCount; node_I++ ) {
		IJK			ijk;
		
		RegularMeshUtils_Node_1DTo3D( decomp, Mesh_NodeMapDomainToGlobal( mesh, node_I ), 
			&ijk[0], &ijk[1], &ijk[2] );      
		if( ijk[0] == ( decomp->nodeGlobal3DCounts[0] - 1 )
			&& ijk[1] == ( decomp->nodeGlobal3DCounts[1] - 1 )
			&& ijk[2] == 0 ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}



IndexSet* RegularMeshUtils_CreateLocalTopSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD *)meshLayout->decomp;
	Node_LocalIndex		node_I;
	Node_LocalIndex		nodeLocalCount;
	IndexSet*		is;
	
	nodeLocalCount = decomp->nodeLocalCount;
	is = IndexSet_New( nodeLocalCount );
	for( node_I = 0; node_I < nodeLocalCount; node_I++ ) {
		IJK			ijk;
		
		RegularMeshUtils_Node_Local1DTo3D( decomp, node_I, &ijk[0], &ijk[1], &ijk[2] );
		if( ijk[1] == ( decomp->nodeLocal3DCounts[decomp->rank][1] - 1 ) ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}

IndexSet* RegularMeshUtils_CreateLocalBottomSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD *)meshLayout->decomp;
	Node_LocalIndex		node_I;
	Node_LocalIndex		nodeLocalCount;
	IndexSet*		is;
	
	nodeLocalCount = decomp->nodeLocalCount;
	is = IndexSet_New( nodeLocalCount );
	for( node_I = 0; node_I < nodeLocalCount; node_I++ ) {
		IJK			ijk;
		
		RegularMeshUtils_Node_Local1DTo3D( decomp, node_I, &ijk[0], &ijk[1], &ijk[2] );      
		if( ijk[1] == 0 ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}

IndexSet* RegularMeshUtils_CreateLocalLeftSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD *)meshLayout->decomp;
	Node_LocalIndex		node_I;
	Node_LocalIndex		nodeLocalCount;
	IndexSet*		is;
	
	nodeLocalCount = decomp->nodeLocalCount;
	is = IndexSet_New( nodeLocalCount );
	for( node_I = 0; node_I < nodeLocalCount; node_I++ ) {
		IJK			ijk;
		
		RegularMeshUtils_Node_Local1DTo3D( decomp, node_I, &ijk[0], &ijk[1], &ijk[2] );      
		if( ijk[0] == 0 ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}

IndexSet* RegularMeshUtils_CreateLocalRightSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
	Node_LocalIndex		node_I;
	Node_LocalIndex		nodeLocalCount;
	IndexSet*		is;
	
	nodeLocalCount = decomp->nodeLocalCount;
	is = IndexSet_New( nodeLocalCount );
	for( node_I = 0; node_I < nodeLocalCount; node_I++ ) {
		IJK			ijk;
		
		RegularMeshUtils_Node_Local1DTo3D( decomp, node_I, &ijk[0], &ijk[1], &ijk[2] );      
		if( ijk[0] == ( decomp->nodeLocal3DCounts[decomp->rank][0] - 1 ) ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}

IndexSet* RegularMeshUtils_CreateLocalFrontSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD *)meshLayout->decomp;
	Node_LocalIndex		node_I;
	Node_LocalIndex		nodeLocalCount;
	IndexSet*		is;
	
	nodeLocalCount = decomp->nodeLocalCount;
	is = IndexSet_New( nodeLocalCount );
	for( node_I = 0; node_I < nodeLocalCount; node_I++ ) {
		IJK			ijk;
		
		RegularMeshUtils_Node_Local1DTo3D( decomp, node_I, &ijk[0], &ijk[1], &ijk[2] );      
		if( ijk[2] == ( decomp->nodeLocal3DCounts[decomp->rank][2] - 1 ) ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}

IndexSet* RegularMeshUtils_CreateLocalBackSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD *)meshLayout->decomp;
	Node_LocalIndex		node_I;
	Node_LocalIndex		nodeLocalCount;
	IndexSet*		is;
	
	nodeLocalCount = decomp->nodeLocalCount;
	is = IndexSet_New( nodeLocalCount );
	for( node_I = 0; node_I < nodeLocalCount; node_I++ ) {
		IJK			ijk;
		
		RegularMeshUtils_Node_Local1DTo3D( decomp, node_I, &ijk[0], &ijk[1], &ijk[2] );      
		if( ijk[2] == 0 ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}

/* These fellas do the same as the 'Local' set creation, but isntead of
   using the local walls, it maps the local index to global walls.  This
   is useful for the exchanger. */

IndexSet* RegularMeshUtils_CreateLocalInGlobalTopSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD *)meshLayout->decomp;
	Node_LocalIndex		node_I;
	Node_LocalIndex		nodeLocalCount;
	IndexSet*		is;
	
	nodeLocalCount = decomp->nodeLocalCount;
	is = IndexSet_New( nodeLocalCount );
	for( node_I = 0; node_I < nodeLocalCount; node_I++ ) {
		IJK			ijk;
		Node_GlobalIndex	node_gI;
		
		node_gI = decomp->nodeMapLocalToGlobal( decomp, node_I );
		RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );
		if( ijk[1] == ( decomp->nodeGlobal3DCounts[1] - 1 ) ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}

IndexSet* RegularMeshUtils_CreateLocalInGlobalBottomSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD *)meshLayout->decomp;
	Node_LocalIndex		node_I;
	Node_LocalIndex		nodeLocalCount;
	IndexSet*		is;
	
	nodeLocalCount = decomp->nodeLocalCount;
	is = IndexSet_New( nodeLocalCount );
	for( node_I = 0; node_I < nodeLocalCount; node_I++ ) {
		IJK			ijk;
		Node_GlobalIndex	node_gI;
		
		node_gI = decomp->nodeMapLocalToGlobal( decomp, node_I );
		RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );
		if( ijk[1] == ( 0 ) ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}

IndexSet* RegularMeshUtils_CreateLocalInGlobalRightSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD *)meshLayout->decomp;
	Node_LocalIndex		node_I;
	Node_LocalIndex		nodeLocalCount;
	IndexSet*		is;
	
	nodeLocalCount = decomp->nodeLocalCount;
	is = IndexSet_New( nodeLocalCount );
	for( node_I = 0; node_I < nodeLocalCount; node_I++ ) {
		IJK			ijk;
		Node_GlobalIndex	node_gI;
		
		node_gI = decomp->nodeMapLocalToGlobal( decomp, node_I );
		RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );
		if( ijk[0] == ( decomp->nodeGlobal3DCounts[0] - 1 ) ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}

IndexSet* RegularMeshUtils_CreateLocalInGlobalLeftSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD *)meshLayout->decomp;
	Node_LocalIndex		node_I;
	Node_LocalIndex		nodeLocalCount;
	IndexSet*		is;
	
	nodeLocalCount = decomp->nodeLocalCount;
	is = IndexSet_New( nodeLocalCount );
	for( node_I = 0; node_I < nodeLocalCount; node_I++ ) {
		IJK			ijk;
		Node_GlobalIndex	node_gI;
		
		node_gI = decomp->nodeMapLocalToGlobal( decomp, node_I );
		RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );
		if( ijk[0] == ( 0 ) ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}

IndexSet* RegularMeshUtils_CreateLocalInGlobalFrontSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD *)meshLayout->decomp;
	Node_LocalIndex		node_I;
	Node_LocalIndex		nodeLocalCount;
	IndexSet*		is;
	
	nodeLocalCount = decomp->nodeLocalCount;
	is = IndexSet_New( nodeLocalCount );
	for( node_I = 0; node_I < nodeLocalCount; node_I++ ) {
		IJK			ijk;
		Node_GlobalIndex	node_gI;
		
		node_gI = decomp->nodeMapLocalToGlobal( decomp, node_I );
		RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );
		if( ijk[2] == ( decomp->nodeGlobal3DCounts[2] - 1 ) ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}

IndexSet* RegularMeshUtils_CreateLocalInGlobalBackSet( void* _mesh ) {
	Mesh*			mesh = (Mesh*)_mesh;
	MeshLayout*		meshLayout = mesh->layout;
	HexaMD*			decomp = (HexaMD *)meshLayout->decomp;
	Node_LocalIndex		node_I;
	Node_LocalIndex		nodeLocalCount;
	IndexSet*		is;
	
	nodeLocalCount = decomp->nodeLocalCount;
	is = IndexSet_New( nodeLocalCount );
	for( node_I = 0; node_I < nodeLocalCount; node_I++ ) {
		IJK			ijk;
		Node_GlobalIndex	node_gI;
		
		node_gI = decomp->nodeMapLocalToGlobal( decomp, node_I );
		RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );
		if( ijk[2] == ( 0 ) ) {
			IndexSet_Add( is, node_I );
		}
	}
	
	return is;
}

Node_DomainIndex RegularMeshUtils_GetDiagOppositeAcrossElementNodeIndex( void* _mesh, Element_DomainIndex refElement_dI, Node_DomainIndex refNode_dI ) {
	Mesh*              mesh = (Mesh*)_mesh;
	const Node_Index   oppositeNodesMap2D[] = { 2, 3, 0, 1 };
	Node_Index         oppositeNodesMap3D[] = { 6, 7, 4, 5, 2, 3, 0, 1 };
	Node_DomainIndex*  currElementNodes = NULL;
	Node_Index         currElementNodeCount = 0;
	Node_Index         refNode_eI = 0;
	Node_DomainIndex   oppositeNode_dI = 0;
	Node_Index         oppositeNode_eI = 0;
	Stream*            errorStr = Journal_Register( Error_Type, "RegularMeshUtils" );

	Journal_Firewall( CornerNL_Type == mesh->layout->nodeLayout->type , errorStr,
		"Error- in %s: Given mesh has node layout of type \"%s\", different to "
		"required type \"%s\".\n", __func__, mesh->layout->nodeLayout->type, CornerNL_Type );

	currElementNodes = mesh->elementNodeTbl[refElement_dI];
	currElementNodeCount = mesh->elementNodeCountTbl[refElement_dI];

	/* Find index of reference node within reference element */
	for( refNode_eI = 0; refNode_eI < currElementNodeCount; refNode_eI++ ) {
		if ( refNode_dI == currElementNodes[refNode_eI] )
			break;
	}
	Journal_Firewall( refNode_eI < currElementNodeCount, errorStr,
		"Error - in %s(): Reference node %d (domain) not found within reference element %d (domain).\n",
		__func__, refNode_dI, refElement_dI );

	/* Use mapping table to get diagonally opposite node, then convert back to domain index */
	
	if ( ((HexaEL*)mesh->layout->elementLayout)->dim == 2 ) {	
		oppositeNode_eI = oppositeNodesMap2D[refNode_eI];
	}
	else {
		oppositeNode_eI = oppositeNodesMap3D[refNode_eI];
	}

	oppositeNode_dI = currElementNodes[oppositeNode_eI];
	return oppositeNode_dI;
}

