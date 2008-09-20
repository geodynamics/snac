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
**	Defines a particle in StGermain. Collectively, a group of particles deemed a swarm.
**
**	All particles in StGermain have the notion of being within a cell - a owning cell.
**	Particles can then be categorised into using a local coordinate system (local to the owning cell)
**	or a global coordinate system (to the whole domain).
**
** Assumptions:
**
** Comments:
**
** $Id: StandardParticle.h 3526 2006-04-13 03:59:17Z AlanLo $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Swarm_StandardParticle_h__
#define __Discretisation_Swarm_StandardParticle_h__

	extern const Type StandardParticle_Type;
	extern const Type LocalParticle_Type;
	extern const Type GlobalParticle_Type;

	/** Standard base class for particle */
	#define __BaseParticle \
		Cell_DomainIndex	owningCell;	/**< The cell in a mesh which this particle currently lives in */

	struct BaseParticle { __BaseParticle };

	/* For backward compatibility */
	#if 0
	#define __StandardParticle \
		__BaseParticle \
		Coord                   coord;
		
	struct StandardParticle { __StandardParticle };
	#endif

	struct StandardParticle { __BaseParticle };

	/** A particle with a local coordinate system */
	#define __LocalParticle \
		__BaseParticle \
		Coord                   xi;             /**< Local coordinates within element/cell */

	struct LocalParticle { __LocalParticle };

	/** A particle with a global coordinate system */
	#define __GlobalParticle \
		__BaseParticle \
		Coord                   coord;          /**< The global coordinates of this particle */ 

	struct GlobalParticle { __GlobalParticle };

#endif
