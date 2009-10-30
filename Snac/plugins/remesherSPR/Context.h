/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003,
**	Steve Quenette, 110 Victoria Street, Melbourne, Victoria, 3053, Australia.
**	Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
**	University of Texas, 1 University Station, Austin, Texas, 78712, USA.
**
** Authors:
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
*/
/** \file
** Role:
**
** Assumptions:
**
** Comments:
**
** $Id: Context.h 3250 2006-10-23 06:15:18Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __SnacRemesher_Context_h__
#define __SnacRemesher_Context_h__


	typedef enum {
		SnacRemesher_Off, 
		SnacRemesher_OnTimeStep, 
		SnacRemesher_OnMinLengthScale, 
		SnacRemesher_OnBothTimeStepLength, 
		SnacRemesher_ConditionMax
	} SnacRemesher_Condition;
	

	/* Context Information */
	struct _SnacRemesher_Context {
		SnacRemesher_Condition	condition;	/* Type of remeshing criterion condition */
		unsigned				bottomRestore;	/* Type of remeshing criterion condition */
		Index 				OnTimeStep;	/* Time step multiple to remesh on */
		double 				onMinLengthScale;	/* Min length scale to remesh on */
		
		EntryPoint_Index		recoverNodeK;	/* Key to node recovery entry point (for speed) */
		EntryPoint_Index		interpolateNodeK;	/* Key to node interpolation entry point (for speed) */
		EntryPoint_Index		interpolateElementK;	/* Key to element interpolation entry point (for speed) */

		Stream*				debugIC;
		Stream*				debugCoords;
		Stream*				debugNodes;
		Stream*				debugElements;
		Stream*				debugSync;

		/* Dumping */
		FILE*               remesherOut;
		Index               remeshingCount;
	};

	/* Print the contents of the context extension */
	void SnacRemesher_Context_Print( void* _context );

	/* Run the recover node entry point */
	#define SnacRemesher_RecoverNodeMacro( context, self, nodeInd ) \
		( KeyCall( context, self->recoverNodeK, SnacRemesher_RecoverNode_CallCast* ) ( \
			KeyHandle( context, self->recoverNodeK ), \
			context, nodeInd ) )

	void SnacRemesher_RecoverNodeFunction( void* context, SnacRemesher_Context* self, unsigned nodeInd );

	#ifdef MACRO_AS_FUNC
		#define SnacRemesher_RecoverNode SnacRemesher_RecoverNodeFunction
	#else
		#define SnacRemesher_RecoverNode SnacRemesher_RecoverNodeMacro
	#endif

	/* Run the interpolate node entry point */
	#define SnacRemesher_InterpolateNodeMacro( context, self, \
						   nodeInd, elementInd, tetInd, \
						   tetNodeInds, weights, \
						   dstNodes ) \
		( KeyCall( context, self->interpolateNodeK, SnacRemesher_InterpolateNode_CallCast* ) ( \
			KeyHandle( context, self->interpolateNodeK ), \
			context, \
			nodeInd, elementInd, tetInd, \
			tetNodeInds, weights, \
			dstNodes ) )

	void SnacRemesher_InterpolateNodeFunction( void* context, SnacRemesher_Context* self, 
						   unsigned nodeInd, unsigned elementInd, unsigned tetInd, 
						   unsigned* tetNodeInds, double* weights, 
						   Snac_Node* dstNodes );

	#ifdef MACRO_AS_FUNC
		#define SnacRemesher_InterpolateNode SnacRemesher_InterpolateNodeFunction
	#else
		#define SnacRemesher_InterpolateNode SnacRemesher_InterpolateNodeMacro
	#endif

	/* Run the interpolate element entry point */
	#define SnacRemesher_InterpolateElementMacro( context, self, dstElementInd, dstTetInd ) \
		( KeyCall( context, self->interpolateElementK, SnacRemesher_InterpolateElement_CallCast* ) ( \
			KeyHandle( context, self->interpolateElementK ), \
			context, \
			dstElementInd, \
			dstTetInd ) )
	void SnacRemesher_InterpolateElementFunction( void*	_context, SnacRemesher_Context* self, 
												  Element_LocalIndex	dstEltInd, 
												  Tetrahedra_Index		dstTetInd );

	#ifdef MACRO_AS_FUNC
		#define SnacRemesher_InterpolateElement SnacRemesher_InterpolateElementFunction
	#else
		#define SnacRemesher_InterpolateElement SnacRemesher_InterpolateElementMacro
	#endif

#endif /* __SnacRemesher_Context_h__ */
