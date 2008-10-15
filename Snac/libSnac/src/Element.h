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
**	Snac Element.
**
** Assumptions:
**
** Comments:
**
** $Id: Element.h 3275 2007-03-28 20:07:08Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Snac_Element_h__
#define __Snac_Element_h__

	/* Element_Tetrahedra_Surface */
	#define __Snac_Element_Tetrahedra_Surface \
		Area				area; \
		Normal				normal;
	struct _Snac_Element_Tetrahedra_Surface { __Snac_Element_Tetrahedra_Surface };

	/* Element_Tetrahedra */
	#define __Snac_Element_Tetrahedra \
		Volume				old_volume; \
		Volume				volume; \
		Snac_Element_Tetrahedra_Surface	surface[Tetrahedra_Surface_Count]; \
		StrainRateTensor		strainRate; \
		StrainTensor			strain;  \
		StressTensor			stress;  \
		Density				density; \
		double				avgTemp; \
		Material_Index		material_I;
	struct _Snac_Element_Tetrahedra { __Snac_Element_Tetrahedra };

	typedef StressTensor			Snac_TetraStressTensor[Tetrahedra_Count];

	/* Element Information */
	#define __Snac_Element \
		Material_Index		material_I; \
		Volume				volume; \
		StrainRate			strainRate; \
		Stress				stress; \
		double				hydroPressure; \
		double              rzbo; \
		double              bottomPressure; \
		int                 irheology; \
		\
		Snac_Element_Tetrahedra		tetra[Tetrahedra_Count]; 
	struct _Snac_Element { __Snac_Element };

	/* Print the contents of an Element's tetrahedra's surface */
	void Snac_Element_Tetrahedra_Surface_Print( void* surface, Stream* stream );

	/* Print the contents of an Element's tetrahedra */
	void Snac_Element_Tetrahedra_Print( void* tetra, Stream* stream );

	/* Print the contents of an Element */
	void Snac_Element_Print( void* element, Stream* stream );

#endif /* __Snac_Element_h__ */
