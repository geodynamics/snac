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
** $Id: Interpolator.h 662 2004-01-20 12:34:17Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __SnacExchanger_SnacInterpolator_h__
#define __SnacExchanger_SnacInterpolator_h__

#include "Exchanger/Interpolator.h"
#include "Exchanger/Array2D.h"

class SnacInterpolator : public Exchanger::Interpolator {

	double xtol;
	double ytol;
	double ztol;

 protected:
	Exchanger::Array2D<int,1> tetra_;  // tetra # from which fields are interpolated

 public:
	SnacInterpolator( const Exchanger::BoundedMesh& b, const void* _context, Exchanger::Array2D<int,1>& meshNode );
	virtual ~SnacInterpolator();

	void interpolateV( Exchanger::Array2D<double,Exchanger::DIM>& V, const void* _context) const;
	void interpolateT( Exchanger::Array2D<double,1>& T, const void* _context) const;
	void interpolateS( Exchanger::Array2D<double,Exchanger::STRESS_DIM>& S, const void* _context ) const;

 private:
	void init( const Exchanger::BoundedMesh& boundedMesh,
		   const void* _context,
		   Exchanger::Array2D<int,1>& meshNode );
	void selfTest( const Exchanger::BoundedMesh& boundedMesh,
		       const void* _context,
		       const Exchanger::Array2D<int,1>& meshNode ) const;

	void findMaxGridSpacing( const void* _context );
	bool isCandidate( const double* xc, const Exchanger::BoundedBox& bbox ) const;
	double TetrahedronVolume( double *x1, double *x2, double *x3, double *x4 ) const;
	double det3_sub( double *x1, double *x2, double *x3 ) const;
	void appendFoundElement( int el, int ntetra, const double* det, double dett );

	// diable
	virtual void interpolateDisplacement(Exchanger::Array2D<double,Exchanger::DIM>& D) {};
	virtual void interpolateForce(Exchanger::Array2D<double,Exchanger::DIM>& F) {};
	virtual void interpolateHeatflux(Exchanger::Array2D<double,Exchanger::DIM>& H) {};
	virtual void interpolateTraction(Exchanger::Array2D<double,Exchanger::DIM>& F) {};
	virtual void interpolatePressure(Exchanger::Array2D<double,1>& P) {};
	virtual void interpolateStress(Exchanger::Array2D<double,Exchanger::STRESS_DIM>& S) {};
	virtual void interpolateTemperature(Exchanger::Array2D<double,1>& T) {};
	virtual void interpolateVelocity(Exchanger::Array2D<double,Exchanger::DIM>& V) {};

};
#endif /* __SnacExchanger_Interpolator_h__ */
