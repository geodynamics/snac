/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
** Role:
**	Handles the temperature initial and boundary conditions
**
** Assumptions:
**	None as yet.
**
** Comments:
**	None as yet.
**
** $Id: VariableConditions.h 1247 2004-04-20 00:40:15Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Snac_WinklerG3Force_VariableConditions_h__
#define __Snac_WinklerG3Force_VariableConditions_h__

	void _SnacWinklerG3Force_Apply(
		void*				context,
		Node_LocalIndex			node_lI,
		double				speedOfSound,
		Mass*				mass,
		Mass*				inertialMass,
		Force*				force,
		Force*				balance );
	void _SnacWinklerG3Force_Apply_Spherical(
		void*				context,
		Node_LocalIndex			node_lI,
		double				speedOfSound,
		Mass*				mass,
		Mass*				inertialMass,
		Force*				force,
		Force*				balance );
	double Spherical_RMin;
	double Spherical_RMax;

#endif /* __Snac_WinklerG3Force_VariableConditions_h__ */
