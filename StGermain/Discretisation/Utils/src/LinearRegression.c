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
** $Id: LinearRegression.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"

#include "types.h"
#include "LinearRegression.h"

#include <assert.h>

const Type LinearRegression_Type = "LinearRegression";

LinearRegression* LinearRegression_New(Variable* xVariable, Variable* yVariable) {
	return _LinearRegression_New(
			sizeof(LinearRegression),
			LinearRegression_Type,
			_LinearRegression_Delete,
			_LinearRegression_Print,
			_LinearRegression_Copy, 
			xVariable,
			yVariable);
}

LinearRegression* _LinearRegression_New(
 		SizeT                           _sizeOfSelf, 
		Type                            type,
		Stg_Class_DeleteFunction*       _delete,
		Stg_Class_PrintFunction*        _print, 
		Stg_Class_CopyFunction*         _copy,
		Variable*                       xVariable, 
		Variable*                       yVariable )		
{
	LinearRegression*		self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(LinearRegression) );
	self = (LinearRegression*)_Stg_Class_New( _sizeOfSelf, type, _delete, _print, _copy );
	
	self->xVariable = xVariable;
	self->yVariable = yVariable;
	
	return self;
}

void _LinearRegression_Delete( void* linearRegression ) {
	LinearRegression* self = (LinearRegression*) linearRegression;
	_Stg_Class_Delete( self );
}

void _LinearRegression_Print( void* _linearRegression, Stream* stream ) {
	LinearRegression* self = (LinearRegression*) _linearRegression;
	Journal_Printf( stream, "LinearRegression\n" );

	Journal_PrintValue( stream, self->xMean );
	Journal_PrintValue( stream, self->yMean );
	Journal_PrintValue( stream, self->slope );
	Journal_PrintValue( stream, self->interceptor );
	Journal_PrintValue( stream, self->slopeStandardError );
	Journal_PrintValue( stream, self->interceptorStandardError );
	Journal_PrintValue( stream, self->correlationCoefficient );
}

void* _LinearRegression_Copy( void* linearRegression, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	LinearRegression*	self = (LinearRegression*)linearRegression;
	LinearRegression*	newLinearRegression;
	
	newLinearRegression = _Stg_Class_Copy( self, dest, deep, nameExt, ptrMap );
				
	return (void*)newLinearRegression;
}

/* Based on least squares proceedure described in 
 * Eric W. Weisstein. "Least Squares Fitting." From MathWorld--A Wolfram Web Resource.
 * http://mathworld.wolfram.com/LeastSquaresFitting.html */
void LinearRegression_Calculate( void* linearRegression ) {
	LinearRegression*	self      = (LinearRegression*)linearRegression;
	Variable*           xVariable = self->xVariable;
	Variable*           yVariable = self->yVariable;
	Index               array_I;
	Index               arraySize;
	double              xMean     = 0.0;
	double              yMean     = 0.0;
	double              xValue;
	double              yValue;
	double              SS_xx     = 0.0;
	double              SS_yy     = 0.0;
	double              SS_xy     = 0.0;
	double              slope;
	double              errorVariance;

	Variable_Update( xVariable );
	Variable_Update( yVariable );

	arraySize = self->xVariable->arraySize;
	assert( arraySize == yVariable->arraySize );

	for ( array_I = 0 ; array_I < arraySize ; array_I++ ) {
		xValue = Variable_GetValueDouble( xVariable, array_I );
		yValue = Variable_GetValueDouble( yVariable, array_I );

		xMean += xValue;
		yMean += yValue;

		/* Sum intermediate values - Equations 17, 19, 21 */
		SS_xx += xValue * xValue;
		SS_yy += yValue * yValue;
		SS_xy += xValue * yValue;
	}

	/* Divide by array size to obtain true mean */
	xMean /= (double) arraySize;
	yMean /= (double) arraySize;
	self->xMean = xMean;
	self->yMean = yMean;

	/* Calculate intermediate values - Equations 17, 19, 21 */
	SS_xx -= (double) arraySize * xMean * xMean;
	SS_yy -= (double) arraySize * yMean * yMean;
	SS_xy -= (double) arraySize * xMean * yMean;

	/* Finalise calculation coefficients of linear equations y ~= slope * x + interceptor - Equation 27 - 28 */
	self->slope = slope = SS_xy / SS_xx;
	self->interceptor   =         yMean - slope * xMean;

	/* Calculate the correlation coefficient r^2 - Equation 29 */
	self->correlationCoefficient = SS_xy * SS_xy / ( SS_xx * SS_yy );

	/* Calculate Standard errors - Equations 33-35 */
	errorVariance = sqrt( ( SS_yy - slope * SS_xy ) / (double) (arraySize - 2 ) );
	self->interceptorStandardError = errorVariance * sqrt( 1.0/(double)arraySize + xMean*xMean/SS_xx );
	self->slopeStandardError       = errorVariance / sqrt( SS_xx );
}




	
