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
** $Id: Heat.h 1095 2004-03-28 00:51:42Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Snac_Temperature_VariableConditions_h__
#define __Snac_Temperature_VariableConditions_h__

	void _SnacTemperature_InitialConditions( void* _context );
	void SnacTemperature_BoundaryConditions( void* _context );
	void SnacTemperature_HeatFluxBC( void* _context );
	void _SnacTemperature_Top2BottomSweep( Node_LocalIndex node_lI, Variable_Index var_I, void* context, void* result );
	void _SnacTemperature_Top2BottomSweep_Spherical( 
		Node_LocalIndex			node_lI, 
		Variable_Index			var_I, 
		void*				context, 
		void*				result );
	void _SnacTemperature_Citcom_Compatible( 
		Node_LocalIndex			node_lI, 
		Variable_Index			var_I, 
		void*				context, 
		void*				result );

#endif
