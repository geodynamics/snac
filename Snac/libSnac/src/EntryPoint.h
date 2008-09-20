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
** $Id: EntryPoint.h 3274 2007-03-27 20:25:29Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Snac_EntryPoint_h__
#define __Snac_EntryPoint_h__

	/* Templates for default entry point type */
	typedef void			(Snac_UpdateElementMomentum_Cast)	( void* context, Element_LocalIndex element_lI, double* elementMinLengthScale );
	typedef void			(Snac_UpdateElementMomentum_CallCast)	( void* entryPoint, void* context, Element_LocalIndex element_lI, double* elementMinLengthScale );
	typedef void			(Snac_StrainRate_Cast)			( void* context, Element_LocalIndex element_lI );
	typedef void			(Snac_StrainRate_CallCast)		( void* entryPoint, void* context, Element_LocalIndex element_lI );
	typedef void			(Snac_Stress_Cast)			( void* context, Element_LocalIndex element_lI );
	typedef void			(Snac_Stress_CallCast)			( void* entryPoint, void* context, Element_LocalIndex element_lI );
	typedef void			(Snac_Force_Cast)			( void* context, Node_LocalIndex node_lI, double speedOfSnd, Mass* mass, Mass* inertialMass, Force* force, Force* balance );
	typedef void			(Snac_Force_CallCast)			( void* entryPoint, void* context, Node_LocalIndex node_lI, double speedOfSnd, Mass* mass, Mass* inertialMass, Force* force, Force* balance );
	typedef void			(Snac_UpdateNodeMomentum_Cast)		( void* context, Node_LocalIndex node_lI, Mass inertialMass, Force force );
	typedef void			(Snac_UpdateNodeMomentum_CallCast)	( void* entryPoint, void* context, Node_LocalIndex node_lI, Mass inertialMass, Force force );
	typedef void			(Snac_Constitutive_Cast)		( void* context, Element_LocalIndex element_lI );
	typedef void			(Snac_Constitutive_CallCast)		( void* entryPoint, void* context, Element_LocalIndex element_lI );
	#define 			Snac_UpdateElementMomentum_CastType	(ContextEntryPoint_CastType_MAX+1)
	#define 			Snac_StrainRate_CastType		(Snac_UpdateElementMomentum_CastType+1)
	#define 			Snac_Stress_CastType			(Snac_StrainRate_CastType+1)
	#define 			Snac_Force_CastType			(Snac_Stress_CastType+1)
	#define 			Snac_UpdateNodeMomentum_CastType	(Snac_Force_CastType+1)
	#define 			Snac_Constitutive_CastType		(Snac_UpdateNodeMomentum_CastType+1)
	#define 			Snac_Temperature_CastType		(Snac_Constitutive_CastType+1)
	#define 			Snac_CastType_MAX			(Snac_Temperature_CastType+1)

	/** Textual name of this class */
	extern const Type Snac_EntryPoint_Type;

	/** Snac_EntryPoint info */
	#define __Snac_EntryPoint \
		/* General info */ \
		__ContextEntryPoint \
		\
		/* Virtual info */ \
		\
		/* Snac_EntryPoint info */ \
		Snac_Constitutive_CallCast*		constitutiveRun;
	struct _Snac_EntryPoint { __Snac_EntryPoint };

	/* Create a new Snac_EntryPoint */
	Snac_EntryPoint* Snac_EntryPoint_New( const Name name, unsigned int castType );

	/* Initialise an Snac_EntryPoint */
	void Snac_EntryPoint_Init( void* snac_EntryPoint, const Name name, unsigned int castType );

	/* Creation implementation */
	Snac_EntryPoint* _Snac_EntryPoint_New(
		SizeT					_sizeOfSelf,
		Type					type,
		Stg_Class_DeleteFunction*			_delete,
		Stg_Class_PrintFunction*			_print,
		Stg_Class_CopyFunction*			_copy, 
		EntryPoint_GetRunFunction*		_getRun,
		const Name				name,
		unsigned int				castType);

	/* Initialisation implementation */
	void _Snac_EntryPoint_Init( Snac_EntryPoint* self );


	/* Default GetRun implementation */
	Func_Ptr _Snac_EntryPoint_GetRun( void* snac_EntryPoint );

	/* Snac entry point run... for update element */
	void _Snac_EntryPoint_Run_UpdateElementMomentum( 
		void*					entryPoint, 
		void*					context, 
		Element_LocalIndex			element_lI, 
		double*					elementMinLengthScale );

	/* Snac entry point run... for strain rate */
	void _Snac_EntryPoint_Run_StrainRate( 
		void*					entryPoint, 
		void*					context, 
		Element_LocalIndex			element_lI );

	/* Snac entry point run... for stress */
	void _Snac_EntryPoint_Run_Stress( 
		void*					entryPoint, 
		void*					context, 
		Element_LocalIndex			element_lI );

	/* Snac entry point run... for force */
	void _Snac_EntryPoint_Run_Force( 
		void*					entryPoint, 
		void*					context, 
		Node_LocalIndex				node_lI, 
		double					speedOfSnd, 
		Mass*					mass, 
		Mass*					inertialMass, 
		Force*					force, 
		Force*					balance );

	/* Snac entry point run... for update node */
	void _Snac_EntryPoint_Run_UpdateNodeMomentum( 
		void*					entryPoint, 
		void*					context,
		Node_LocalIndex				node_lI,
		Mass					inertialMass, 
		Force					force );

	/* Snac entry point run... for rheology */
	void _Snac_EntryPoint_Run_Constitutive( 
		void*					entryPoint, 
		void*					context, 
		Element_LocalIndex			element_lI );

	/* Snac entry point run... for rheology */
	void Snac_EntryPoint_ChangeRunConststutive( void* entryPoint, Snac_Constitutive_CallCast* constitutiveRun );

#endif /* __Snac_EntryPoint_h__ */
