/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003,
**	Steve Quenette, 110 Victoria Street, Melbourne, Victoria, 3053, Australia.
**	Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
**	University of Texas, 1 University Station, Austin, Texas, 78712, USA.
**
** Authors:
**	Eh Tan, ?, Caltech (tan2@gps.caltech.edu)
**	Pururav Thoutireddy, ?, Caltech (puru@cacr.caltech.edu)
**	Eun-seo Choi, ?, Caltech (ces74@gps.caltech.edu)
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
** $Id: Source.h 662 2004-01-20 12:34:17Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __SnacExchanger_Source_h__
#define __SnacExchanger_Source_h__

#include <vector>
#include "mpi.h"
#include "Exchanger/Source.h"
#include "SnacInterpolator.h"

class SnacSource : public Exchanger::Source {

	const void* _context;

 public:
	SnacSource( MPI_Comm comm, int sink,
		    Exchanger::BoundedMesh& mesh,
		    const Exchanger::BoundedBox& mybbox,
		    const void* __context );
	virtual ~SnacSource();

	virtual void interpolateVelocity(Exchanger::Array2D<double,Exchanger::DIM>& V) const;
	virtual void interpolateTemperature(Exchanger::Array2D<double,1>& T) const;
	virtual void interpolateStress(Exchanger::Array2D<double,Exchanger::STRESS_DIM>& S) const;
	virtual void createInterpolator(const Exchanger::BoundedMesh& mesh);

 private:
	SnacInterpolator* snac_interp;

	// Disable
	virtual void interpolateDisplacement(Exchanger::Array2D<double,Exchanger::DIM>& D) const {};
	virtual void interpolateForce(Exchanger::Array2D<double,Exchanger::DIM>& F) const {};
	virtual void interpolateHeatflux(Exchanger::Array2D<double,Exchanger::DIM>& H) const {};
	virtual void interpolatePressure(Exchanger::Array2D<double,1>& P) const {};
	virtual void interpolateTraction(Exchanger::Array2D<double,Exchanger::DIM>& F) const {};

};

#endif/* __SnacExchanger_Source_h__ */
