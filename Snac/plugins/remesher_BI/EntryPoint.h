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
** $Id: EntryPoint.h 3250 2006-10-23 06:15:18Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __SnacRemesher_EntryPoint_h__
#define __SnacRemesher_EntryPoint_h__

	/* Templates for default entry point type */
	typedef void (SnacRemesher_InterpolateNode_Cast)( void* context, 
							  unsigned nodeInd, unsigned elementInd, unsigned tetInd, 
							  unsigned* tetNodeInds, double* weights, 
							  Snac_Node* dstNodes );

	typedef void (SnacRemesher_InterpolateElement_Cast)( void* context, 
							     unsigned dstElementInd, unsigned dstTetInd, 
							     SnacRemesher_Element* elementArray, 
							     unsigned srcElementInd, unsigned srcTetInd );

	typedef void (SnacRemesher_CopyElement_Cast)( void* context, 
												  unsigned elementInd, unsigned tetInd, 
												  SnacRemesher_Element* elementArray );

	typedef void (SnacRemesher_InterpolateNode_CallCast)( void* entryPoint, void* context, 
							      unsigned nodeInd, unsigned elementInd, unsigned tetInd, 
							      unsigned* tetNodeInds, double* weights, 
							      Snac_Node* dstNodes );

	typedef void (SnacRemesher_InterpolateElement_CallCast)( void* entryPoint, void* context, 
								 unsigned dstElementInd, unsigned dstTetInd, 
								 SnacRemesher_Element* elementArray, 
								 unsigned srcElementInd, unsigned srcTetInd );

	typedef void (SnacRemesher_CopyElement_CallCast)( void* entryPoint, void* context, 
													  unsigned elementInd, unsigned tetInd, 
													  SnacRemesher_Element* elementArray );

	#define SnacRemesher_InterpolateNode_CastType		(Snac_CastType_MAX + 1)
	#define SnacRemesher_InterpolateElement_CastType	(SnacRemesher_InterpolateNode_CastType + 1)
	#define SnacRemesher_CopyElement_CastType			(SnacRemesher_InterpolateElement_CastType + 1)
	#define SnacRemesher_CastType_MAX					(SnacRemesher_CopyElement_CastType + 1)

	/** Textual name of this class */
	extern const Type SnacRemesher_EntryPoint_Type;

	/** SnacRemesher_EntryPoint info */
	#define __SnacRemesher_EntryPoint \
		/* General info */ \
		__Snac_EntryPoint \
		\
		/* Virtual info */ \
		\
		/* SnacRemesher_EntryPoint info */

	struct _SnacRemesher_EntryPoint { __SnacRemesher_EntryPoint };

	/* Create a new SnacRemesher_EntryPoint */
	SnacRemesher_EntryPoint* SnacRemesher_EntryPoint_New( const Name name, unsigned int castType );

	/* Initialise an SnacRemesher_EntryPoint */
	void SnacRemesher_EntryPoint_Init( void* snac_EntryPoint, const Name name, unsigned int castType );

	/* Creation implementation */
	SnacRemesher_EntryPoint* _SnacRemesher_EntryPoint_New(
		SizeT					_sizeOfSelf,
		Type					type,
		Stg_Class_DeleteFunction*			_delete,
		Stg_Class_PrintFunction*			_print,
		Stg_Class_CopyFunction*			_copy, 
		EntryPoint_GetRunFunction*		_getRun,
		const Name				name,
		unsigned int				castType);

	/* Initialisation implementation */
	void _SnacRemesher_EntryPoint_Init( SnacRemesher_EntryPoint* self );

	/* Default GetRun implementation */
	Func_Ptr _SnacRemesher_EntryPoint_GetRun( void* snac_EntryPoint );

	/* Snac remesher entry point run... for interpolating a node */
	void _SnacRemesher_EntryPoint_Run_InterpolateNode( void* entryPoint, void* context, 
							   unsigned nodeInd, unsigned elementInd, unsigned tetInd, 
							   unsigned* tetNodeInds, double* weights, 
							   Snac_Node* dstNodes );
	
	/* Snac remesher entry point run... for interpolating an element */
	void _SnacRemesher_EntryPoint_Run_InterpolateElement( void* entryPoint, void* context, 
							      unsigned dstElementInd, unsigned dstTetInd, 
							      SnacRemesher_Element* elementArray, 
							      unsigned srcElementInd, unsigned srcTetInd );

	/* Snac remesher entry point run... for copying interpolated values to an element */
	void _SnacRemesher_EntryPoint_Run_CopyElement( void* entryPoint, void* context, 
									unsigned ElementInd, unsigned TetInd, 
									SnacRemesher_Element* elementArray );

#endif /* __SnacRemesher_EntryPoint_h__ */
