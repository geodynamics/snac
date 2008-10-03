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
**	Represents a Variable that is a discretisation of a field-like physical property over a spatial domain.
**
** Assumptions:
**	The function interfaces assume the spatially disc. variable is stored as a double
**	(it can't be an int because its an approximation to a continuous variable right?)
**
** Comments:
**	Abstract class that defines an interface to use when accessing spatially discretised
**	field variables.
**
**	This means that e.g. visualisation code can be written to use this class,
**	and doesn't have to worry exactly how the variable is discretised - that will be
**	done by the back-end implementation of this class.
**
**	The name comes from the definition of "field" in the physics domain: A region of space
**	characterized by a physical property, such as gravitational or electromagnetic force or
**	fluid pressure, having a determinable value at every point in the region.
**
**	TODO: should it have a ptr to the Variable its based on at this level?
**	doesn't make sense at the moment as the FeVariable used a \
**	doflayout rather than a variable -> but may in future... 
**
**	$Id: LinearRegression.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Utils_LinearRegression_h__
#define __Discretisation_Utils_LinearRegression_h__

	extern const Type LinearRegression_Type;

	/** LinearRegression contents */
	#define __LinearRegression \
		/* General info */ \
		__Stg_Class \
		\
		/* Virtual info */ \
		\
		/* Other Info */\
		Variable* xVariable;                \
		Variable* yVariable;                \
		double    xMean;                    \
		double    yMean;                    \
		double    slope;                    \
		double    interceptor;              \
		double    slopeStandardError;       \
		double    interceptorStandardError; \
		double    correlationCoefficient;   

	struct LinearRegression { __LinearRegression };

	LinearRegression* LinearRegression_New(Variable* xVariable, Variable* yVariable) ;

	LinearRegression* _LinearRegression_New(
					SizeT                           _sizeOfSelf,
					Type                            type,
					Stg_Class_DeleteFunction*       _delete,
					Stg_Class_PrintFunction*        _print,
					Stg_Class_CopyFunction*         _copy,
					Variable*                       xVariable,
					Variable*                       yVariable );

	void _LinearRegression_Delete( void* linearRegression ) ;
	void _LinearRegression_Print( void* _linearRegression, Stream* stream ) ;
	void* _LinearRegression_Copy( void* linearRegression, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) ;

	/* Based on least squares proceedure described in
	 * Eric W. Weisstein. "Least Squares Fitting." From MathWorld--A Wolfram Web Resource.
	 * http://mathworld.wolfram.com/LeastSquaresFitting.html */
	void LinearRegression_Calculate( void* linearRegression ) ;

#endif /* __Discretisation_Utils_LinearRegression_h__ */
