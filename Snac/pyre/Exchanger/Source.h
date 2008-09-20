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

class Source {
protected:
	MPI_Comm comm;
	const int sink;
	std::auto_ptr<Interpolator> interp;
	Array2D<int,1> meshNode_;
	Array2D<double,DIM> X_;

public:
	Source( MPI_Comm comm, int sink, BoundedMesh& mesh, const void* _context, const BoundedBox& mybbox );
	virtual ~Source() {};

	inline int size() const { return meshNode_.size(); }
	void interpolateT( Array2D<double,1>& T, const void* _context ) const;
	void interpolateV( Array2D<double,DIM>& V, const void* _context ) const;
	void interpolateF( Array2D<double,DIM>& F, const void* _context ) const;

	template <class T, int N> void sendArray2D( const Array2D<T,N>& array ) const;

	template <class T1, int N1, class T2, int N2> void sendArray2D( const Array2D<T1,N1>& array1, const Array2D<T2,N2>& array2 ) const;

private:
	void recvMesh( BoundedMesh& mesh );
	void sendMeshNode() const;
	// added to be consistent with Citcom
	void initX(const BoundedMesh& mesh);
};


template <class T, int N>
void Source::sendArray2D( const Array2D<T,N>& array ) const {
	#ifdef DEBUG
		if( size() != array.size() ) {
			journal::firewall_t firewall( "Source" );
			firewall << journal::loc(__HERE__) << "Source: inconsistenet array size" << journal::end;
			throw std::out_of_range( "Source" );
		}
	#endif

	if( size() ) {
		MPI_Request request;
		array.send( comm, sink, 0, array.size(), request );
		util::waitRequest( request );
	}
}


template <class T1, int N1, class T2, int N2>
void Source::sendArray2D( const Array2D<T1,N1>& array1, const Array2D<T2,N2>& array2 ) const {
	#ifdef DEBUG
		if( size() != array1.size() || size() != array2.size() ) {
			journal::firewall_t firewall( "Source" );
			firewall << journal::loc(__HERE__) << "Source: inconsistenet array size" << journal::end;
			throw std::out_of_range( "Source" );
		}
	#endif

	if( size() ) {
		std::vector<MPI_Request> request;
		request.reserve( 2 );

		request.push_back( MPI_Request() );
		array1.send( comm, sink, 0, array1.size(), request.back() );

		request.push_back( MPI_Request() );
		array2.send( comm, sink, 0, array2.size(), request.back() );

		util::waitRequest( request );
	}
}

#endif/* __SnacExchanger_Source_h__ */
