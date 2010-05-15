/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003,
**	Steve Quenette, 110 Victoria Street, Melbourne, Victoria, 3053, Australia.
**	Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
**	University of Texas, 1 University Station, Austin, Texas, 78712, USA.
**
** Authors:
**           Colin Stark, Doherty Research Scientist, Lamont-Doherty Earth Observatory (cstark@ldeo.columbia.edu)
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
** $Id: ConstructExtensions.c $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Context.h"
#include "Constitutive.h"
#include "Register.h"
#include "ConstructExtensions.h"
#include <assert.h>
#include <limits.h>

//#define DEBUG

void _SnacDikeInjection_ConstructExtensions( void* _context, void* data ) {
	Snac_Context*				context = (Snac_Context*)_context;
	SnacDikeInjection_Context*			contextExt = ExtensionManager_Get(
																		  context->extensionMgr,
																		  context,
																		  SnacDikeInjection_ContextHandle );
#ifdef DEBUG
	printf( "In %s()\n", __func__ );
#endif
	
	/* DikeInjection variables */
	contextExt->startX = Dictionary_Entry_Value_AsDouble(
			Dictionary_GetDefault( context->dictionary, "startX", 
								   Dictionary_Entry_Value_FromDouble( 0.0f ) ) );
	contextExt->startZ = Dictionary_Entry_Value_AsDouble(
			Dictionary_GetDefault( context->dictionary, "startZ", 
								   Dictionary_Entry_Value_FromDouble( 0.0f ) ) );
	contextExt->endX = Dictionary_Entry_Value_AsDouble(
			Dictionary_GetDefault( context->dictionary, "endX", 
								   Dictionary_Entry_Value_FromDouble( 0.0f ) ) );
	contextExt->endZ = Dictionary_Entry_Value_AsDouble(
			Dictionary_GetDefault( context->dictionary, "endZ", 
								   Dictionary_Entry_Value_FromDouble( 1.0e+04 ) ) );
	contextExt->dikeDepth = Dictionary_Entry_Value_AsDouble(
			Dictionary_GetDefault( context->dictionary, "dikeDepth", 
								   Dictionary_Entry_Value_FromDouble( 1.0e+04 ) ) );
	contextExt->dikeWidth = Dictionary_Entry_Value_AsDouble(
			Dictionary_GetDefault( context->dictionary, "dikeWidth", 
								   Dictionary_Entry_Value_FromDouble( 1.8e+03 ) ) ); /* 1.8 * dx looks appropriate. */
	contextExt->injectionRate = Dictionary_Entry_Value_AsDouble(
			Dictionary_GetDefault( context->dictionary, "injectionRate", 
								   Dictionary_Entry_Value_FromDouble( 4.8e+03 ) ) ); /* a fraction of applied plate vel. */

}
