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
** $Id: petsccompat.h 3403 2006-01-13 08:33:58Z RobertTurnbull $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __MPIMESSAGING_H__
#define __MPIMESSAGING_H__

#include "mpi.h"
#include "stgmessaging.h"

#define MPI_Send( buf, count, datatype, dest, tag, comm ) \
	Stg_MPI_Send( __FILE__, __LINE__, buf, count, datatype, dest, tag, comm )

#define MPI_Ssend( buf, count, datatype, dest, tag, comm ) \
	Stg_MPI_Ssend( __FILE__, __LINE__, buf, count, datatype, dest, tag, comm )

#define MPI_Isend( buf, count, datatype, dest, tag, comm, request ) \
	Stg_MPI_Isend( __FILE__, __LINE__, buf, count, datatype, dest, tag, comm, request )

#define MPI_Recv( buf, count, datatype, source, tag, comm, status ) \
	Stg_MPI_Recv( __FILE__, __LINE__, buf, count, datatype, source, tag, comm, status )

#define MPI_Irecv( buf, count, datatype, source, tag, comm, request ) \
	Stg_MPI_Irecv( __FILE__, __LINE__, buf, count, datatype, source, tag, comm, request )

#define MPI_Reduce( sendbuf, recvbuf, count, datatype, op, root, comm ) \
	Stg_MPI_Reduce( __FILE__, __LINE__, sendbuf, recvbuf, count, datatype, op, root, comm )

#define MPI_Allreduce( sendbuf, recvbuf, count, datatype, op, comm ) \
	Stg_MPI_Allreduce( __FILE__, __LINE__, sendbuf, recvbuf, count, datatype, op, comm )

#define MPI_Gather( sendbuf, sendcnt, sendtype, recvbuf, recvcount, recvtype, root, comm ) \
	Stg_MPI_Gather( __FILE__, __LINE__, sendbuf, sendcnt, sendtype, recvbuf, recvcount, recvtype, root, comm )

#define MPI_Allgather( sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm ) \
	Stg_MPI_Allgather( __FILE__, __LINE__, sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm )
       
#define MPI_Wait( request, status ) \
	Stg_MPI_Wait( __FILE__, __LINE__, request, status )

#define MPI_Test( request, flag, status ) \
	Stg_MPI_Test( __FILE__, __LINE__, request, flag, status )




#endif
