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
**
** Assumptions:
**
** Comments:
**
** $Id: types.h 3986 2007-01-29 07:31:46Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Swarm_types_h__
#define __Discretisation_Swarm_types_h__
	
	/* Types user needs to define */
	typedef struct _Particle			Particle;

	/** Types of coordinate system a particle/swarm is using */
	typedef enum {
		LocalCoordSystem = 0,
		GlobalCoordSystem
	} CoordSystem;

	/* Cell types ... for readability */
	typedef Index					Cell_Index;
	typedef Index					Cell_LocalIndex;
	typedef Index					Cell_DomainIndex;
	typedef Index					Cell_ShadowIndex;
	typedef Index					Cell_ShadowTransferIndex;
	typedef Coord*					Cell_Point;
	typedef Cell_Point*				Cell_Points;
	typedef Cell_Points*				Cell_PointsList;
	typedef Index					Cell_PointIndex;
	typedef Cell_PointIndex*			Cell_PointIndexList;
	
	/* Particle types ... for readability */
	typedef Index					Particle_Index;
	typedef Index					Particle_InCellIndex;
	typedef Particle_Index*				Cell_Particles;
	typedef Cell_Particles*				Cell_ParticlesList;
	typedef Particle_InCellIndex*			Cell_ParticlesIndexList;
	typedef char                                    Particle_Bool;

	typedef Particle*				Particle_List;
	
	/* Discretisation_Swarm types/classes */
	typedef struct CellLayout			CellLayout;
	typedef struct SingleCellLayout			SingleCellLayout;
	typedef struct TriSingleCellLayout		TriSingleCellLayout;
	typedef struct ElementCellLayout		ElementCellLayout;
	typedef struct ParticleLayout			ParticleLayout;
	typedef struct PerCellParticleLayout		PerCellParticleLayout;
	typedef struct RandomParticleLayout		RandomParticleLayout;
	typedef struct MeshParticleLayout		MeshParticleLayout;
	typedef struct GaussParticleLayout		GaussParticleLayout;
	typedef struct TriGaussParticleLayout		TriGaussParticleLayout;
	typedef struct GlobalParticleLayout		GlobalParticleLayout;
	typedef struct SpaceFillerParticleLayout        SpaceFillerParticleLayout;
	typedef struct WithinShapeParticleLayout        WithinShapeParticleLayout;
	typedef struct PlaneParticleLayout	        PlaneParticleLayout;
	typedef struct UnionParticleLayout	        UnionParticleLayout;
	typedef struct ManualParticleLayout		ManualParticleLayout;
	typedef struct LineParticleLayout		LineParticleLayout;
	typedef struct FileParticleLayout		FileParticleLayout;
	typedef struct Swarm				Swarm;
	typedef struct Swarm_Register			Swarm_Register;
	typedef struct SwarmDump			SwarmDump;
	typedef struct ParticleCommHandler		ParticleCommHandler;
	typedef struct SwarmVariable_Register	SwarmVariable_Register;
	typedef struct SwarmVariable			SwarmVariable;
	typedef struct OperatorSwarmVariable		OperatorSwarmVariable;
	typedef struct SwarmOutput			SwarmOutput;

	typedef struct BaseParticle                     BaseParticle;
	typedef struct StandardParticle			StandardParticle;
	typedef struct LocalParticle			LocalParticle;
	typedef struct GlobalParticle			GlobalParticle;
	typedef struct IntegrationPoint			IntegrationPoint;
	
	/* output streams: initialised in Swarm_Init() */
	extern Stream* Swarm_VerboseConfig;
	extern Stream* Swarm_Debug;
	extern Stream* Swarm_Warning;
	extern Stream* Swarm_Error;
	
#endif /* __Discretisation_Swarm_types_h__ */
