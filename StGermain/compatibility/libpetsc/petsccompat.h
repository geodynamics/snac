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
**	Interface between StGermain and different versions of PETSc, so we can modify default
**	PETSc functionality where appropriate, and also allow users with slightly out-of-date
**	versions of PETSc to keep using the code.
**
** Assumptions:
**
** Comments:
**	Please add new macros to the appropriate section: modifications for desired functionality,
**	backward compatibility or forward compatibility. Please also add a comment about the 
**	purpose of the macro where this isn't fairly obvious, and the versions of PETSc it applies
**	to.
**	It is intended we will refactor the Linear Algebra module soon to always use the latest
**	PETSc interface, thereby removing the need for the forward compatibility section.
**
**
** $Id: petsccompat.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __PETSCCOMPAT_H
#define __PETSCCOMPAT_H

/**************************               PETSC INCLUDES            ******************************************/

/* Need to make sure these are first, so we don't try and over-ride the symbols within PETSc itself */
#if (( PETSC_VERSION_MAJOR == 2 ) && ( PETSC_VERSION_MINOR >= 2 )) || ( PETSC_VERSION_MAJOR > 2 )
	#include <petscksp.h>
#else	
	#include <petscsles.h>
#endif	

/************************** MODIFICATIONS FOR DESIRED FUNCTIONALITY ******************************************/

/* Specific modifications needed for desired functionality: PETSc 2.3.0 and onwards */
#if (( PETSC_VERSION_MAJOR == 2 ) && ( PETSC_VERSION_MINOR >= 3 )) || ( PETSC_VERSION_MAJOR > 2 )
	/* PetscLogInfo compatibility between for 2.3.0 and previous -- AlanLo */
	#undef PetscLogInfo
	#if defined(PETSC_USE_DEBUG)
		#define PetscLogInfo(...) PetscLogInfo_Private( __VA_ARGS__ )
	#else
		#define PetscLogInfo(...)
	#endif

#endif

/* Specific modifications needed for desired functionality: PETSc 2.2.0 and onwards */
#if (( PETSC_VERSION_MAJOR == 2 ) && ( PETSC_VERSION_MINOR >= 2 )) || ( PETSC_VERSION_MAJOR > 2 )
	/* Being specific about Matrix creation, since the internal rules behind this changed after 2.2 :
		"This was needed for petsc 2.2.1 when building matrices (in a finite amount of time) 
		for use wih either MUMPs or SuperLU" -- DavidMay */
	#define MatCreateMPIAIJ( MPI, globalRow, globalCol, localRow, localCol, maxPerRow, diag, maxPerCol, offDiag, mat ) 	\
		do {										\
			int numProcs = 0; \
			MatCreate( MPI, globalRow, globalCol, localRow, localCol, mat ); 					\
			MatSetFromOptions( *mat ); 										\
			MPI_Comm_size( MPI_COMM_WORLD, &numProcs );								\
			if( numProcs > 1 ) {											\
				MatMPIAIJSetPreallocation( *mat, maxPerRow, diag, maxPerCol, offDiag );				\
			}													\
			else {													\
				MatSeqAIJSetPreallocation( *mat, maxPerRow, diag );						\
			} \
		} \
		while( 0 )

#endif		
/*---------------------------------------------------------------------------------------------------*/


/************************** BACKWARD COMPATIBILITY ****************************************************/

/* For PETSc 2.2.x and earlier: backward compatibility for needed functions from 2.3.x onwards */
#if (( PETSC_VERSION_MAJOR == 2 ) && ( PETSC_VERSION_MINOR <= 2 )) 

	/* Backward compatibility from current -> old interface */
	#define PetscMallocGetCurrentUsage( space )\
			PetscTrSpace( (space), NULL, NULL )

	#define PetscMallocGetMaximumUsage( maxs )\
			PetscTrSpace( NULL, NULL, (maxs) )

	/* These macros for non-essential functions allow a PETSc 2.2.x or earlier user to compile
	   the code, but not to run the multigrid component, which requires the following new
	   functions from PETSc 2.3.0 onwards */
	#define MatMatMult( a1, a2, a3, a4, a5 ) \
		fprintf( stderr, "Must use at least PETSc 2.3.0.\n" )

	#define MatPtAP( a1, a2, a3, a4, a5 )					\
		fprintf( stderr, "Must use at least PETSc 2.3.0 for 'MatPtAP'.\n" );

	#define PCMGSetLevels( a1, a2, a3 ) \
		fprintf( stderr, "Must use at least PETSc 2.3.0.\n" )

	#define PCMGSetType( a1, a2 ) \
		fprintf( stderr, "Must use at least PETSc 2.3.0.\n" )

	#define PCMGSetNumberSmoothUp( a1, a2 ) \
		fprintf( stderr, "Must use at least PETSc 2.3.0.\n" )

	#define PCMGSetNumberSmoothDown( a1, a2 ) \
		fprintf( stderr, "Must use at least PETSc 2.3.0.\n" )

	#define PCMGGetSmoother( a1, a2, a3 ) \
		fprintf( stderr, "Must use at least PETSc 2.3.0.\n" )

	#define PCMGGetSmootherDown( a1, a2, a3 ) \
		fprintf( stderr, "Must use at least PETSc 2.3.0.\n" )

	#define PCMGGetSmootherUp( a1, a2, a3 ) \
		fprintf( stderr, "Must use at least PETSc 2.3.0.\n" )

	#define PCMGSetResidual( a1, a2, a3, a4 ) \
		fprintf( stderr, "Must use at least PETSc 2.3.0.\n" )

	#define PCMGSetRestriction( a1, a2, a3 ) \
		fprintf( stderr, "Must use at least PETSc 2.3.0.\n" )

	#define PCMGSetInterpolate( a1, a2, a3 ) \
		fprintf( stderr, "Must use at least PETSc 2.3.0.\n" )

	#define PCMGSetRhs( a1, a2, a3 ) \
		fprintf( stderr, "Must use at least PETSc 2.3.0.\n" )

	#define PCMGSetX( a1, a2, a3 ) \
		fprintf( stderr, "Must use at least PETSc 2.3.0.\n" )

	#define PCMGSetR( a1, a2, a3 ) \
		fprintf( stderr, "Must use at least PETSc 2.3.0.\n" )

	#define PCMGSetCyclesOnLevel( a1, a2, a3 ) \
		fprintf( stderr, "Must use at least PETSc 2.3.0.\n" )

	#define PCMGGetLevels( a1, a2 ) \
		fprintf( stderr, "Must use at least PETSc 2.3.0.\n" )

	#define VecGetValues( vector, nInds, inds, array ) \
		fprintf( stderr, "Must use at least PETSc 2.3.0.\n" )

#endif	/* (( PETSC_VERSION_MAJOR == 2 ) && ( PETSC_VERSION_MINOR < 3 )) */

/* For PETSc 2.1.x and earlier: backward compatibility for features from newer PETSc versions we want to use */
#if (( PETSC_VERSION_MAJOR == 2 ) && ( PETSC_VERSION_MINOR <= 1 ))

	/* These typedefs don't appear till 2.2... */
	typedef int PetscErrorCode;
	typedef int PetscInt;

	#define PETSC_FILE_RDONLY	PETSC_BINARY_RDONLY
	#define PETSC_FILE_WRONLY	PETSC_BINARY_WRONLY
	#define PETSC_FILE_CREATE	PETSC_BINARY_CREATE

	#define KSPGetOperators(ksp,Amat,Pmat,flag )			\
		do {										\
			PC	__pc;							\
												\
			KSPGetPC( (ksp), &__pc );				\
			PCGetOperators( __pc, Amat, Pmat, flag );	\
		}										\
		while( 0 )
	
	/* Functions only necessary for multigrid */
	#define PCBJacobiGetSubKSP( a, b, c, d )					\
		fprintf( stderr, "Must at least PETSc version 2.2.0.\n" )

#endif
/*---------------------------------------------------------------------------------------------------*/


/************************** FORWARD COMPATIBILITY ****************************************************/

/* We keep using the interface as at 2.3.0 until 2.3.1 is released with the new interface */
#if (( PETSC_VERSION_MAJOR == 2 ) && ( PETSC_VERSION_MINOR == 3 ) && (PETSC_VERSION_SUBMINOR == 0 ) && (PETSC_VERSION_PATCH >= 44 )) \
    || (( PETSC_VERSION_MAJOR == 2 ) && ( PETSC_VERSION_MINOR == 3 ) && (PETSC_VERSION_SUBMINOR > 0 )) \
    || (( PETSC_VERSION_MAJOR == 2 ) && ( PETSC_VERSION_MINOR > 3 )) \
	|| ( PETSC_VERSION_MAJOR > 2 )
	
	#define PETSC_FILE_RDONLY	FILE_MODE_READ
	#define PETSC_FILE_WRONLY	FILE_MODE_APPEND
	#define PETSC_FILE_CREATE	FILE_MODE_WRITE
	
#endif


/* Forward compatibility from 2.1.x to PETSc 2.2.x and greater */
#if (( PETSC_VERSION_MAJOR == 2 ) && ( PETSC_VERSION_MINOR >= 2 )) || ( PETSC_VERSION_MAJOR > 2 )

	/* Forward compatibility: we use the old SLES interface in the Linear Algebra code, so the newer 
	   versions need to convert to use it */

	/* Object */
	#define SLES KSP

	/* Methods */
	#define SLESCreate KSPCreate
	#define SLESSetUp KSPSetUp
	#define SLESSolveTranspose KSPSolveTranspose
	#define SLESDestroy KSPDestroy
	#define SLESGetTolerances KSPGetTolerances
	#define SLESSetTolerances KSPSetTolerances
	#define SLESSetInitialGuessNonzero KSPSetInitialGuessNonzero
	#define SLESGetRhs KSPGetRhs
	#define SLESGetSolution KSPGetSolution
	#define SLESSetPC KSPSetPC
	#define SLESGetPC KSPGetPC
	#define SLESSetMonitor KSPSetMonitor
	#define SLESClearMonitor KSPClearMonitor
	#define SLESGetMonitorContext KSPGetMonitorContext
	#define SLESSetConvergenceTest KSPSetConvergenceTest
	#define SLESGetConvergenceContext KSPGetConvergenceContext
	#define SLESSetOperators KSPSetOperators
	#define SLESSetFromOptions KSPSetFromOptions
	#define SLESGetIterationNumber KSPGetIterationNumber

	#define SLESGetKSP( sles, kspPtr ) \
		( (*(kspPtr)) = (sles) )

	/* Special rule for PETSc 2.2.0, thereafter it changed */
	#if (PETSC_VERSION_MAJOR == 2) && (PETSC_VERSION_MINOR == 2) && (PETSC_VERSION_SUBMINOR == 0)
		#define SLESSolve(s,b,u,its) KSPSetRhs(s,b);KSPSetSolution(s,u);KSPSolve(s);KSPGetIterationNumber(s,its)
	#else	
		#define SLESSolve(s,b,u,its) KSPSolve(s,b,u);KSPGetIterationNumber(s,its)
	#endif

	/* In Snark2, we use the old MatMerge interface */
	#define MatMerge( comm, inMat, outMatPtr ) \
		MatMerge( comm, inMat, PETSC_DECIDE, MAT_INITIAL_MATRIX, outMatPtr )

#endif


/* Additional forward compatibility to 2.3.x and greater */
#if (( PETSC_VERSION_MAJOR == 2 ) && ( PETSC_VERSION_MINOR >= 3 )) || ( PETSC_VERSION_MAJOR > 2 )

	#define VecSet( value, vec ) \
			VecSet( vec, *value )

	#define PetscObjectGetName( obj, name ) \
			PetscObjectGetName( obj, (const char**)name )

	#define VecScale( value, vec ) \
			VecScale( vec, *value )

	#define VecAXPY( a, x, y ) \
			VecAXPY( y, *a, x )

	#define VecAYPX( a, x, y ) \
			VecAYPX( y, *a, x )

	#define MatCreate( comm, rowLocal, colLocal, rowGlobal, colGlobal, mat ) \
			MatCreate( comm, mat ); MatSetSizes( *mat, rowLocal, colLocal, rowGlobal, colGlobal )

	#define VecPointwiseDivide( dividend, denominator, destination ) \
			VecPointwiseDivide( destination, dividend, denominator )

	#define MatAXPY( a, x, y, matStructure ) \
			MatAXPY( y, *a, x, matStructure )

	#define MatAYPX( a, x, y ) \
			MatAYPX( y, *a, x )
	
	#define MatScale( v, m ) \
		MatScale( m, *v )

#endif



#endif /* ndef __PETSCCOMPAT_H */

