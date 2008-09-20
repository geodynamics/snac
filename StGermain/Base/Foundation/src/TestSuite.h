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
**
** Assumptions:
**
** Invariants:
**
** Comments:
**
** $Id: RangeSet.h 3584 2006-05-16 11:11:07Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Foundation_TestSuite_h__
#define __Base_Foundation_TestSuite_h__

	/** Textual name of this class */
	extern const Type TestSuite_Type;

	/* Forward declaring Stream */
	struct Stream;
	struct PtrMap;

	/** Virtual function types */

	/** Class contents */
	typedef Bool (TestSuite_TestFunc)( unsigned rank, unsigned nProcs, unsigned watch );

	typedef struct {
		const char*		name;
		TestSuite_TestFunc*	func;
		unsigned		nReps;
	} TestSuite_Test;

	#define __TestSuite				\
		/* General info */			\
		__Stg_Class				\
							\
		/* Virtual info */			\
							\
		/* TestSuite info */			\
		unsigned		nTests;		\
		TestSuite_Test*		tests;		\
		unsigned		watch;

	struct TestSuite { __TestSuite };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/

	#define TESTSUITE_DEFARGS		\
		STG_CLASS_DEFARGS

	#define TESTSUITE_PASSARGS	\
		STG_CLASS_PASSARGS

	TestSuite* TestSuite_New();
	TestSuite* _TestSuite_New( TESTSUITE_DEFARGS );
	void _TestSuite_Init( TestSuite* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _TestSuite_Delete( void* testSuite );
	void _TestSuite_Print( void* testSuite, struct Stream* stream );

	#define TestSuite_Copy( self ) \
		(TestSuite*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define TestSuite_DeepCopy( self ) \
		(TestSuite*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _TestSuite_Copy( void* testSuite, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	void TestSuite_SetTests( void* testSuite, unsigned nTests, TestSuite_Test* tests );
	void TestSuite_SetProcToWatch( void* testSuite, unsigned watch );

	void TestSuite_Run( void* testSuite );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	void TestSuite_Destruct( TestSuite* self );

#endif /* __Base_Foundation_TestSuite_h__ */
