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
** Comments:
**
**	$Id: SobolGenerator.h 3742 2006-08-02 10:48:21Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Utils_SobolGenerator_h__
#define __Discretisation_Utils_SobolGenerator_h__

	/** Textual name of this class */
	extern const Type SobolGenerator_Type;
	
	/** SobolGenerator contents */
	#define __SobolGenerator \
		/* General info */ \
		__Stg_Object \
		\
		/* Virtual info */ \
		\
		/* Object Info */ \
		unsigned int        polynomialDegree;                     \
		unsigned int        polynomialCoefficient;                \
		Bool*               directionalNumberIsCalc;              \
		unsigned int *      directionalNumberList;                \
		unsigned int        directionalNumbersAlloced;            \
		unsigned int        lastCalculated_I;                     \
		unsigned int        lastCalculatedNumber;                 \


	struct SobolGenerator { __SobolGenerator };	

	/** Public constructor */
	SobolGenerator* SobolGenerator_NewFromTable( Name name ) ;
	
	SobolGenerator* SobolGenerator_New( 
			Name                                               name,
			unsigned int                                       polynomialDegree,
			unsigned int                                       polynomialCoefficient,
			const unsigned int *                               initialDirectionNumbers );

	/** Private Constructor */
	SobolGenerator* _SobolGenerator_New(
			SizeT                                              _sizeOfSelf, 
			Type                                               type,
			Stg_Class_DeleteFunction*                          _delete,
			Stg_Class_PrintFunction*                           _print, 
			Stg_Class_CopyFunction*                            _copy, 
			Name                                               name );

	void _SobolGenerator_Init( 
		SobolGenerator*                                    self,
		unsigned int                                       polynomialDegree,
		unsigned int                                       polynomialCoefficient,
		const unsigned int *                               initialDirectionNumbers )	;		

	/** Stg_Class Virtual Functions */
	void _SobolGenerator_Delete( void* sobolGenerator ) ;
	void _SobolGenerator_Print( void* sobolGenerator, Stream* stream ) ;
	void* _SobolGenerator_Copy( void* sobolGenerator, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) ;
	
	/** +++ Private Functions +++ **/
	Bit_Index _SobolGenerator_FindRightmostZeroBit( unsigned int number ) ;

	/** +++ Public Functions +++ **/
	/** See  B.L. Fox P. Bratley. Implementing Sobol's Quasirandom Sequence Generator. 
	 * ACM Transactions on Mathematical Software, 14(1):88 - 100, March 1988. For details of algorithm */
	unsigned int SobolGenerator_GetDirectionalNumber( void* sobolGenerator, unsigned int directionalNum_I ) ;
	double SobolGenerator_GetNextNumber( void* sobolGenerator ) ;
	double SobolGenerator_GetNumberByIndex( void* sobolGenerator, unsigned int index ) ;

	double SobolGenerator_GetNextNumber_WithMinMax( void* sobolGenerator, double min, double max ) ;
	double SobolGenerator_GetNumberByIndex_WithMinMax( void* sobolGenerator, unsigned int index, double min, double max ) ;

#endif /* __Discretisation_Utils_SobolGenerator_h__ */
