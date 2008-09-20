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
**	Snac types.
**
** Assumptions:
**	None as yet.
**
** Comments:
**	None as yet.
**
** $Id: types.h 3173 2005-11-21 23:47:09Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Snac_types_h__
#define __Snac_types_h__
	
	/* Material types */
	typedef unsigned int				Material_Index;
	typedef unsigned int				Snac_Material_Rheology;
	
	
	/* Plastic */
	typedef struct _Plastic_Constitutive 		Plastic_Constitutive;
	
	
	/* Tetrahedra types */
	typedef unsigned int				Tetrahedra_Index;
	typedef unsigned int				Tetrahedra_Surface_Index;
	
	/* Snac sync mode */
	typedef enum					{ Snac_SyncType_Interpolate, Snac_SyncType_Remesh } Snac_SyncType;
	
	/* Force calculation mode */
	typedef enum					{ 
								Snac_Force_Quick, 
								Snac_Force_Normal, 
								Snac_Force_Accurate,
								Snac_Force_Complete
							} Snac_ForceCalcType;
	
	/* Context types/classes */
	typedef struct _Snac_Material			Snac_Material;
	typedef struct _Snac_Node			Snac_Node;
	typedef struct _Snac_Element_Tetrahedra_Surface	Snac_Element_Tetrahedra_Surface;
	typedef struct _Snac_Element_Tetrahedra		Snac_Element_Tetrahedra;
	typedef struct _Snac_Element			Snac_Element;
	typedef struct _Snac_Particle			Snac_Particle;
	typedef struct _Snac_EntryPoint			Snac_EntryPoint;
	typedef struct _Snac_Parallel			Snac_Parallel;
	typedef struct _Snac_Context			Snac_Context;
	typedef struct _SnacSync			SnacSync;

	/* Extensions */
	typedef struct _Snac_Mesh			Snac_Mesh;
	
#endif /* __Snac_types_h__ */
