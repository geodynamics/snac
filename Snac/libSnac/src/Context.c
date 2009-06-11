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
** $Id: Context.c 3275 2007-03-28 20:07:08Z EunseoChoi $
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>

#include "units.h"
#include "types.h"
#include "shortcuts.h"
#include "Material.h"
#include "Node.h"
#include "Mesh.h"
#include "Tetrahedra.h"
#include "TetrahedraTables.h"
#include "Element.h"
#include "EntryPoint.h"
#include "UpdateElement.h"
#include "StrainRate.h"
#include "Stress.h"
#include "Force.h"
#include "UpdateNode.h"
#include "Parallel.h"
#include "Context.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <assert.h>
#include <time.h>

#ifndef PATH_MAX
	#define PATH_MAX 1024
#endif

#define DEBUG

/* Textual name of this class */
const Type Snac_Context_Type = "Snac_Context";


/* Snac_Context entry point names */
const Name Snac_EP_LoopNodesEnergy =		"Snac_EP_LoopNodesEnergy";
const Name Snac_EP_LoopElementsEnergy =		"Snac_EP_LoopElementsEnergy";
const Name Snac_EP_CalcStresses =		"Snac_EP_CalcStresses";
const Name Snac_EP_StrainRate =			"Snac_EP_StrainRate";
const Name Snac_EP_Stress =			"Snac_EP_Stress";
const Name Snac_EP_Constitutive =		"Snac_EP_Constitutive";
const Name Snac_EP_LoopNodesMomentum =		"Snac_EP_LoopNodesMomentum";
const Name Snac_EP_Force =			"Snac_EP_Force";
const Name Snac_EP_ForceBC =			"Snac_EP_ForceBC";
const Name Snac_EP_UpdateNodeMomentum =		"Snac_EP_UpdateNodeMomentum";
const Name Snac_EP_LoopElementsMomentum =	"Snac_EP_LoopElementsMomentum";
const Name Snac_EP_UpdateElementMomentum =	"Snac_EP_UpdateElementMomentum";
const Name Snac_EP_RheologyUpdate =		"Snac_EP_RheologyUpdate";

/* Snac_Context dt type names */
const Name Snac_DtType_Constant =              "constant";
const Name Snac_DtType_Dynamic =               "dynamic";


Snac_Context* Snac_Context_New(
		double					startTime,
		double					stopTime,
		SizeT					nodeSize,
		SizeT					elementSize,
		MPI_Comm				communicator,
		Dictionary*				dictionary )
{
	return _Snac_Context_New( sizeof(Snac_Context), Snac_Context_Type, _Snac_Context_Delete, _Snac_Context_Print, NULL,
		_Snac_Context_SetDt, startTime, stopTime, nodeSize, elementSize, communicator, dictionary );
}



void Snac_Context_Init(
		Snac_Context*				self,
		double					startTime,
		double					stopTime,
		SizeT					nodeSize,
		SizeT					elementSize,
		MPI_Comm				communicator,
		Dictionary*				dictionary )
{
	Topology*		nTopology;
	ElementLayout*		eLayout;
	NodeLayout*		nLayout;
	MeshDecomp*		meshDecomp;
	MeshLayout*		meshLayout;
	Dictionary*		meshDict;
	Partition_Index		decompDims;
	Dimension_Index     dim         = 3;

	/* Create the layouts */
	meshDict = Dictionary_Entry_Value_AsDictionary( Dictionary_Get( dictionary, "mesh" ) );
	meshDict = meshDict ? meshDict : dictionary;
	decompDims = Dictionary_Entry_Value_AsUnsignedInt(
		Dictionary_GetDefault( meshDict, "decompDims", Dictionary_Entry_Value_FromUnsignedInt( 1 ) ) );
	nTopology = (Topology*)IJK6Topology_New( "topology", meshDict );
	eLayout = (ElementLayout*)ParallelPipedHexaEL_New( "elementLayout" , dim, meshDict );
	nLayout = (NodeLayout*)CornerNL_New( "nodeLayout", meshDict, eLayout, nTopology );
	meshDecomp = (MeshDecomp*)HexaMD_New_All( "meshDecomp", meshDict, communicator, eLayout, nLayout, decompDims );
	meshLayout = MeshLayout_New( "meshLayout", eLayout, nLayout, meshDecomp );

	/* General info */
	self->type = Snac_Context_Type;
	self->_sizeOfSelf = sizeof( Snac_Context );
	self->_deleteSelf = False;
	self->dictionary = dictionary;

	/* Virtual info */
	self->_delete = _Snac_Context_Delete;
	self->_print = _Snac_Context_Print;
	self->_copy = NULL;
	self->_setDt = _Snac_Context_SetDt;
	_Stg_Class_Init( (Stg_Class*)self );
	_AbstractContext_Init( (AbstractContext*)self, startTime, stopTime, communicator );
	_MeshContext_Init( (MeshContext*)self, meshLayout, nodeSize, elementSize );

	/* Snac_Context info */
	_Snac_Context_Init( self );

}



Snac_Context* _Snac_Context_New(
		SizeT					_sizeOfSelf,
		Type					type,
		Stg_Class_DeleteFunction*			_delete,
		Stg_Class_PrintFunction*			_print,
		Stg_Class_CopyFunction*			_copy,
		AbstractContext_SetDt*			_setDt,
		double					startTime,
		double					stopTime,
		SizeT					nodeSize,
		SizeT					elementSize,
		MPI_Comm				communicator,
		Dictionary*				dictionary )
{
	Snac_Context*		self;
	Topology*		nTopology;
	ElementLayout*		eLayout;
	NodeLayout*		nLayout;
	MeshDecomp*		meshDecomp;
	MeshLayout*		meshLayout;
	Dictionary*		meshDict;
	Partition_Index		decompDims;
	Dimension_Index     dim         = 3;

	/* Create the layouts */
	meshDict = Dictionary_Entry_Value_AsDictionary( Dictionary_Get( dictionary, "mesh" ) );
	meshDict = meshDict ? meshDict : dictionary;
	decompDims = Dictionary_Entry_Value_AsUnsignedInt(
		Dictionary_GetDefault( meshDict, "decompDims", Dictionary_Entry_Value_FromUnsignedInt( 1 ) ) );
	nTopology = (Topology*)IJK6Topology_New( "topology", meshDict );
	eLayout = (ElementLayout*)ParallelPipedHexaEL_New( "elementLayout", dim, meshDict );
	nLayout = (NodeLayout*)CornerNL_New( "nodeLayout", meshDict, eLayout, nTopology );
	meshDecomp = (MeshDecomp*)HexaMD_New_All( "meshDecomp", meshDict, communicator, eLayout, nLayout, decompDims );
	meshLayout = MeshLayout_New( "meshLayout", eLayout, nLayout, meshDecomp );

	/* Allocate memory */
	self = (Snac_Context*)_MeshContext_New( _sizeOfSelf, type, _delete, _print, _copy,
						NULL, _AbstractContext_Construct, _AbstractContext_Build, _AbstractContext_Initialise,
						_AbstractContext_Execute, _AbstractContext_Destroy, "context", True, _setDt,
						startTime, stopTime, meshLayout, nodeSize, elementSize, communicator, dictionary );

	/* General info */

	/* Virtual info */

	/* Snac_Context info */
	_Snac_Context_Init( self );

	return self;
}

void _Snac_Context_Init( Snac_Context* self ) {
	char			tmpBuf[PATH_MAX];
	Snac_Node		tmpNode;
	Snac_Element		tmpElement;
	Dictionary*		nodeICsDict;
	Dictionary*		elementICsDict;
	Dictionary*		velocityBCsDict;
	char*			tmpStr;

/* 	Mesh*           	mesh = self->mesh; */
/* 	MeshLayout*		layout = (MeshLayout*)self->layout; */
/* 	HexaMD*			decomp = (HexaMD*)self->decomp; */
	

	/* Building StGermain Variables of nodal quantities... we must the "complex" constructor for Variable... the
	 * info needs to be wrapped this generic way... */
	Index			nodeOffsetCount = 8;
	SizeT			nodeOffsets[] = {
					GetOffsetOfMember( tmpNode, velocity ),
					GetOffsetOfMember( tmpNode, velocity[0] ),
					GetOffsetOfMember( tmpNode, velocity[1] ),
					GetOffsetOfMember( tmpNode, velocity[2] ), 
					GetOffsetOfMember( tmpNode, force ),
					GetOffsetOfMember( tmpNode, force[0] ),
					GetOffsetOfMember( tmpNode, force[1] ),
					GetOffsetOfMember( tmpNode, force[2] ) };
	Variable_DataType	nodeDataTypes[] = {
					Variable_DataType_Double,
					Variable_DataType_Double,
					Variable_DataType_Double,
					Variable_DataType_Double,
					Variable_DataType_Double,
					Variable_DataType_Double,
					Variable_DataType_Double,
					Variable_DataType_Double };
	Index			nodeDataTypeCounts[] = {
					3,
					1,
					1,
					1,
					3,
					1,
					1,
					1 };
	Name			nodeNames[] = {
					"velocity",
					"vx",
					"vy",
					"vz",
					"force",
					"fx",
					"fy",
					"fz" };

	/* Building StGermain Variables of element quantities... we must the "complex" constructor for Variable... the
	 * info needs to be wrapped this generic way... */
	Index			elementOffsetCount = 4;
	SizeT			elementOffsets[] = {
					GetOffsetOfMember( tmpElement, material_I ),
					GetOffsetOfMember( tmpElement, strainRate ),
					GetOffsetOfMember( tmpElement, stress ),
					GetOffsetOfMember( tmpElement, hydroPressure )};
	Variable_DataType	elementDataTypes[] = {
					Variable_DataType_Int,
					Variable_DataType_Double,
					Variable_DataType_Double,
					Variable_DataType_Double};
	Index			elementDataTypeCounts[] = {
					1,
					1,
					1,
					1 };
	Name			elementNames[] = {
					"elementMaterial",
					"strainRate",
					"stress",
					"hydroPressure" };

	if( self->rank == 0 ) Journal_Printf( self->debug, "In: %s\n", __func__ );

	/* Generic Snac (but not context) streams */
	self->snacDebug = Journal_Register( DebugStream_Type, "Snac" );
	self->snacInfo = Journal_Register( InfoStream_Type, "Snac" );
	self->snacVerbose = Journal_Register( InfoStream_Type, "Snac-verbose" );
	if( !Dictionary_Get( self->dictionary, "Journal.info.Snac-verbose" ) ) {
		Journal_Enable_NamedStream( InfoStream_Type, "Snac-verbose", False );
	}
	self->snacError = Journal_Register( ErrorStream_Type, "Snac" );

	Mesh_ActivateNodeLocalToGlobalMap( self->mesh );
	Mesh_ActivateNodeElementTbl( self->mesh );
	Mesh_ActivateElementNodeTbl( self->mesh );

	/* Snac_Context info */
	/* Although we set this for safely (use of it will give 0s), it shouldn't be used... the new value should be set before
	    hand */
	self->restartTimestep = Dictionary_Entry_Value_AsUnsignedInt(
		Dictionary_GetDefault( self->dictionary, "restartStep", Dictionary_Entry_Value_FromUnsignedInt( 0 ) ) );

	tmpStr = Dictionary_Entry_Value_AsString( Dictionary_Get( self->dictionary, "dtType" ) );
	if( !strcmp( tmpStr, Snac_DtType_Constant ) ) {
		/* Set dtType to "constant" using known pointer/key... it means checks are pointer comparisons only, not strcmp */
		self->dtType = Snac_DtType_Constant;

		/* When constant Dt type is specified, Dt should be given an initial-non-zero value */
		self->dt = Dictionary_Entry_Value_AsDouble(
			Dictionary_GetDefault( self->dictionary, "timeStep", Dictionary_Entry_Value_FromDouble( 1.0 ) ) );
		if (self->rank==0)
		Journal_Printf( self->info, "\"dtType\" set by Dictionary to \"%s\"\n", self->dtType );
	}
	else if( !strcmp( tmpStr, Snac_DtType_Dynamic ) ) {
		/* Set dtType to "dynamic" using known pointer/key... it means checks are pointer comparisons only, not strcmp */
		self->dtType = Snac_DtType_Dynamic;

		/* When dynamic Dt type is specified, Dt will be calculated later, give 0 as starting value... legacy behaviour. */
		self->dt = Dictionary_Entry_Value_AsDouble(
			Dictionary_GetDefault( self->dictionary, "timeStep", Dictionary_Entry_Value_FromDouble( 0.0 ) ) );

		if (self->rank==0)
		Journal_Printf( self->info, "\"dtType\" set by Dictionary to \"%s\"\n", self->dtType );
	}
	else {
		/* Set dtType to "dynamic" using known pointer/key... it means checks are pointer comparisons only, not strcmp */
		/* Warn the user that this assumption has occured */
		self->dtType = Snac_DtType_Dynamic;

		/* When dynamic Dt type is specified, Dt will be calculated later, give 0 as starting value... legacy behaviour. */
		self->dt = Dictionary_Entry_Value_AsDouble(
			Dictionary_GetDefault( self->dictionary, "timeStep", Dictionary_Entry_Value_FromDouble( 0.0 ) ) );
		if (self->rank==0)
		Journal_Printf(
			self->info,
			"\"dtType\" not specified in Dictionary... assuming \"%s\", of init value: %g\n",
			self->dtType,
			self->dt );
	};
	self->speedOfSound = 0.0f;
	self->minLengthScale = 0.0f;
	self->density = 0.0f;
	self->gravity = 0.0f;
	self->demf = 0.0f;


	/* What method are we using to calculate forces? */
	/* "Complete" type should be always used unless there is a really good reason to use others. */
	self->forceCalcType = Snac_Force_Complete;
	if (self->rank==0)
	Journal_Printf( self->info, "\"forceCalcType\" set by Dictionary to \"complete\"\n" );

	/*
	 *  Record the number of "processors" in each direction in parallelized runs
	 *    - This info is routinely required by snac2vtk for postprocessing but has had to be hand-calculated
	 *      by the user from the numbers recorded in sim.x and in the original input xml.
	 *      Addition of these variables will enforce reporting of the processor geometry and speed postprocessing.
	 */
	self->numProcX = ((HexaMD*)self->mesh->layout->decomp)->partition3DCounts[0];
	self->numProcY = ((HexaMD*)self->mesh->layout->decomp)->partition3DCounts[1];
	self->numProcZ = ((HexaMD*)self->mesh->layout->decomp)->partition3DCounts[2];
	if (self->rank==0)
		Journal_Printf( self->info, "\nParallel processing geometry:  nX=%d  nY=%d  nZ=%d\n\n",
			self->numProcX,self->numProcY,self->numProcZ );

	/* Add initial condition managers */
	nodeICsDict = Dictionary_Entry_Value_AsDictionary( Dictionary_Get( self->dictionary, "nodeICs" ) );
	self->nodeICs = CompositeVC_New( "nodeIC", self->variable_Register, self->condFunc_Register, nodeICsDict, self->mesh );
	elementICsDict = Dictionary_Entry_Value_AsDictionary( Dictionary_Get( self->dictionary, "elementICs" ) );
	self->elementICs = CompositeVC_New( "elementIC", self->variable_Register, self->condFunc_Register, elementICsDict, self->mesh );

	/* Add boundary condition managers */
	velocityBCsDict = Dictionary_Entry_Value_AsDictionary( Dictionary_Get( self->dictionary, "velocityBCs" ) );

	self->velocityBCs = CompositeVC_New( "velocityBC", self->variable_Register, self->condFunc_Register, velocityBCsDict, self->mesh );

	/* Parallelisation information */
	self->parallel = Snac_Parallel_New( self->communicator, self->mesh );

	/* Outputs that can be written only for one processor (rank==0). */
	if( self->rank==0 ) {
		sprintf( tmpBuf, "%s/sim.%u", self->outputPath, self->rank );
		if( (self->simInfo = fopen( tmpBuf, "w+" )) == NULL ) {
			Journal_Firewall(
							 (ArithPointer)self->simInfo,
							 self->snacError,
							 "\"%s\"  failed to open file for writing", tmpBuf );
		}
		else {
			fprintf( self->simInfo, "%u %u %u %u %u %u %u %u %u\n(global element numbers)[3]=(proc numbers)[3]*(local element numbers)[3]\n",
					 ((HexaMD*)self->mesh->layout->decomp)->elementGlobal3DCounts[0],
					 ((HexaMD*)self->mesh->layout->decomp)->elementGlobal3DCounts[1],
					 ((HexaMD*)self->mesh->layout->decomp)->elementGlobal3DCounts[2], 
					 self->numProcX, self->numProcY, self->numProcZ,
					 ((HexaMD*)self->mesh->layout->decomp)->elementLocal3DCounts[self->rank][0],
					 ((HexaMD*)self->mesh->layout->decomp)->elementLocal3DCounts[self->rank][1],
					 ((HexaMD*)self->mesh->layout->decomp)->elementLocal3DCounts[self->rank][2] );
			fflush( self->simInfo );
		}
		sprintf( tmpBuf, "%s/timeStep.%u", self->outputPath, self->rank );
		if( (self->timeStepInfo = fopen( tmpBuf, "w+" )) == NULL ) {
			Journal_Firewall(
							 (ArithPointer)self->timeStepInfo,
							 self->snacError,
							 "\"%s\"  failed to open file for writing", tmpBuf );
		}
		sprintf( tmpBuf, "%s/checkpointTimeStep.%u", self->outputPath, self->rank );
		if( (self->checkpointTimeStepInfo = fopen( tmpBuf, "w+" )) == NULL ) {
			Journal_Firewall(
							 (ArithPointer)self->checkpointTimeStepInfo,
							 self->snacError,
							 "\"%s\"  failed to open file for writing", tmpBuf );
		}
	}

	/* Add hooks to exisiting entry points */
	EntryPoint_Append(
		Context_GetEntryPoint( self, AbstractContext_EP_Build ),
		"SnacBuild",
		_Snac_Context_Build,
		Snac_Context_Type );
	EntryPoint_Append(
		Context_GetEntryPoint( self, AbstractContext_EP_Initialise ),
		"SnacIC",
		_Snac_Context_InitialConditions,
		Snac_Context_Type );
	EntryPoint_Append(
		Context_GetEntryPoint( self, AbstractContext_EP_Dt ),
		"SnacDt",
		_Snac_Context_Dt,
		Snac_Context_Type );
	EntryPoint_Append(
		Context_GetEntryPoint( self, AbstractContext_EP_Initialise ),
		"SnacTimeStepZero",
		Snac_Context_TimeStepZero,
		Snac_Context_Type );
	EntryPoint_Append(
		Context_GetEntryPoint( self, AbstractContext_EP_Solve ),
		"SnacSolve",
		_Snac_Context_Solve,
		Snac_Context_Type );
	EntryPoint_Append(
		Context_GetEntryPoint( self, AbstractContext_EP_Sync ),
		"SnacSync",
		_Snac_Context_Sync,
		Snac_Context_Type );

	/* Add new entry points */
	self->loopElementsEnergyK =
		Context_AddEntryPoint(
			self,
			Snac_EntryPoint_New( Snac_EP_LoopElementsEnergy, EntryPoint_VoidPtr_CastType ) );
	self->loopNodesEnergyK =
		Context_AddEntryPoint(
			self,
			Snac_EntryPoint_New( Snac_EP_LoopNodesEnergy, EntryPoint_VoidPtr_CastType ) );
	self->calcStressesK =
		Context_AddEntryPoint(
			self,
			Snac_EntryPoint_New( Snac_EP_CalcStresses, EntryPoint_VoidPtr_CastType ) );
	self->strainRateK =
		Context_AddEntryPoint(
			self,
			Snac_EntryPoint_New( Snac_EP_StrainRate, Snac_StrainRate_CastType ) );
	self->stressK =
		Context_AddEntryPoint(
			self,
			Snac_EntryPoint_New( Snac_EP_Stress, Snac_Stress_CastType ) );
	self->constitutiveK =
		Context_AddEntryPoint(
			self,
			Snac_EntryPoint_New( Snac_EP_Constitutive, Snac_Constitutive_CastType ) );
	self->loopNodesMomentumK =
		Context_AddEntryPoint(
			self,
			Snac_EntryPoint_New( Snac_EP_LoopNodesMomentum, EntryPoint_VoidPtr_CastType ) );
	self->forceK =
		Context_AddEntryPoint(
			self,
			Snac_EntryPoint_New( Snac_EP_Force, Snac_Force_CastType ) );
	self->updateNodeK =
		Context_AddEntryPoint(
			self,
			Snac_EntryPoint_New( Snac_EP_UpdateNodeMomentum, Snac_UpdateNodeMomentum_CastType ) );
	self->loopElementsMomentumK =
		Context_AddEntryPoint(
			self,
			Snac_EntryPoint_New( Snac_EP_LoopElementsMomentum, EntryPoint_VoidPtr_CastType ) );
	self->updateElementK =
		Context_AddEntryPoint(
			self,
			Snac_EntryPoint_New( Snac_EP_UpdateElementMomentum, Snac_UpdateElementMomentum_CastType ) );

	/* Add hooks to new entry points... no energy hooks by default */
	EntryPoint_Append(
		Context_GetEntryPoint( self, Snac_EP_CalcStresses ),
		"default",
		_Snac_Context_CalcStresses, Snac_Context_Type );
	EntryPoint_Append(
		Context_GetEntryPoint( self, Snac_EP_StrainRate ),
		"default",
		Snac_StrainRate,
		Snac_Context_Type );
	EntryPoint_Append(
		Context_GetEntryPoint( self, Snac_EP_Stress ),
		"default",
		Snac_Stress,
		Snac_Context_Type );
	EntryPoint_Append(
		Context_GetEntryPoint( self, Snac_EP_LoopNodesMomentum ),
		"default",
		_Snac_Context_LoopNodes,
		Snac_Context_Type );
	EntryPoint_Append(
		Context_GetEntryPoint( self, Snac_EP_Force ),
		"default",
		Snac_Force,
		Snac_Context_Type );
	EntryPoint_Append(
		Context_GetEntryPoint( self, Snac_EP_UpdateNodeMomentum ),
		"default",
		Snac_UpdateNodeMomentum ,
		Snac_Context_Type );
	EntryPoint_Prepend(
		Context_GetEntryPoint( self, Snac_EP_UpdateNodeMomentum ),
		"default",
		Snac_UpdateNodeMomentum_PreProcess,
		Snac_Context_Type );
	/* Keep the order of appeding the following three functions. */
	EntryPoint_Append(
		Context_GetEntryPoint( self, Snac_EP_UpdateNodeMomentum ),
		"Snac_DiffTopo",
		Snac_UpdateNodeMomentum_DiffTopo,
		Snac_Context_Type );
	EntryPoint_Append(
		Context_GetEntryPoint( self, Snac_EP_UpdateNodeMomentum ),
		"Snac_DiffTopo",
		Snac_UpdateNodeMomentum_UpdateInteriorTopo,
		Snac_Context_Type );
	EntryPoint_Append(
		Context_GetEntryPoint( self, Snac_EP_UpdateNodeMomentum ),
		"Snac_DiffTopo",
		Snac_UpdateNodeMomentum_AdjustEdgeTopo,
		Snac_Context_Type );
	/*CCCCC*/
	EntryPoint_Append(
		Context_GetEntryPoint( self, Snac_EP_LoopElementsMomentum ),
		"default",
		_Snac_Context_LoopElements,
		Snac_Context_Type );
	EntryPoint_Append(
		Context_GetEntryPoint( self, Snac_EP_UpdateElementMomentum ),
		"default",
		Snac_UpdateElementMomentum,
		Snac_Context_Type );

	/* Create the coord variable */
	Variable_NewVector(
		"coord",
		Variable_DataType_Double,
		3,
		&self->mesh->layout->decomp->nodeLocalCount,
		(void**)&self->mesh->nodeCoord,
		self->variable_Register,
		"x",
		"y",
		"z" );

	/* Create node variables */
	Variable_New(
		"node",
		nodeOffsetCount,
		nodeOffsets,
		nodeDataTypes,
		nodeDataTypeCounts,
		nodeNames,
		&ExtensionManager_GetFinalSize( self->mesh->nodeExtensionMgr ),
		&self->mesh->layout->decomp->nodeLocalCount,
		(void**)&self->mesh->node,
		self->variable_Register );

	/* Create element material type variable */
	Variable_New(
		"element",
		elementOffsetCount,
		elementOffsets,
		elementDataTypes,
		elementDataTypeCounts,
		elementNames,
		&ExtensionManager_GetFinalSize( self->mesh->elementExtensionMgr ),
		&self->mesh->layout->decomp->elementLocalCount,
		(void**)&self->mesh->element,
		self->variable_Register );

	/* Initialize self->spherical as False */
	self->spherical = False;
}


void _Snac_Context_Delete( void* context ) {
	Snac_Context*	self = (Snac_Context*)context;
	MeshLayout*	meshLayout;

	if( self->rank == 0 ) Journal_Printf( self->debug, "In: %s\n", __func__ );

	/* Close the output files */
	fclose( self->stressTensorOut );
	fclose( self->stressTensorCheckpoint );
	if( self->rank==0 ) {
		fclose( self->timeStepInfo );
		fclose( self->checkpointTimeStepInfo );
		fclose( self->simInfo );
	}

	/* Parallelisation information */
	if( self->parallel ) {
		Stg_Class_Delete( self->parallel );
	}

	/* Intitial and Boundary condition managers */
	if( self->velocityBCs ) {
		Stg_Class_Delete( self->velocityBCs );
	}
	if( self->elementICs ) {
		Stg_Class_Delete( self->elementICs );
	}
	if( self->nodeICs ) {
		Stg_Class_Delete( self->nodeICs );
	}

	/* The parent classes don't assume ownership of these objects, so we have to delete them. */
	meshLayout = self->meshLayout;

	/* Stg_Class_Delete parent class */
	_MeshContext_Delete( context );

	/* Stg_Class_Delete layouts (were created before "self", so play safe and delete afterwards). */
	Stg_Class_Delete( meshLayout );
}



void _Snac_Context_Print( void* context, Stream* stream ) {
	Snac_Context* self = (Snac_Context*)context;

	/* General info */
	Journal_Printf( stream, "Snac_Context (%p):\n", self );

	/* Virtual info */

	/* Snac_Context info */

	/* Parent class info */
	_MeshContext_Print( context, stream );
}


void _Snac_Context_Build( void* context ) {
	Snac_Context*	self = (Snac_Context*)context;

	Dictionary_Entry_Value* materialList = Dictionary_Get( self->dictionary, "materials" );
	/*ccccc*/
	if( materialList ) {

		Dictionary_Entry_Value* materialEntry = Dictionary_Entry_Value_GetFirstElement( materialList );
		int numPhase = 0;

		/* first get the number of phses */
		while( materialEntry ) {
			numPhase++;
			materialEntry = materialEntry->next;
		}
		if( self->rank == 0 ) Journal_Printf( self->snacInfo,"%s: numPhase = %d\n",__func__,numPhase);
		self->materialProperty = (Snac_Material*)malloc( sizeof(Snac_Material) * numPhase );
	}

	if( self->rank == 0 ) Journal_Printf( self->debug, "In: %s\n", __func__ );

	Build( self->nodeICs, 0, False );
	Journal_Printf( self->verbose, "nodeICs:\n" );
	VariableCondition_PrintConcise( self->nodeICs, self->verbose );
	Build( self->elementICs, 0, False );
	Journal_Printf( self->verbose, "elementICs:\n" );
	VariableCondition_PrintConcise( self->elementICs, self->verbose );
	Build( self->velocityBCs, 0, False );
	Journal_Printf( self->verbose, "velocityBCs\n" );
	VariableCondition_PrintConcise( self->velocityBCs, self->verbose );

	Snac_Parallel_Build( self->parallel );

}

void _Snac_Context_InitialConditions( void* context ) {
	Snac_Context*	self = (Snac_Context*)context;

	if( self->rank == 0 ) Journal_Printf( self->debug, "In: %s\n", __func__ );


	/* Build all/remaining vars after all memory has been allocated */
	Variable_Register_BuildAll( self->variable_Register );

	/* scaling factor for dynamic time stepping */
	self->strain_inert = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( self->dictionary, "strain_inert", Dictionary_Entry_Value_FromDouble( 1.0e-5f ) ) );

	/* scaling factor for dynamic time stepping */
	self->computeThermalStress = Dictionary_Entry_Value_AsBool(
		Dictionary_GetDefault( self->dictionary, "computeThermalStress", Dictionary_Entry_Value_FromBool( False ) ) );

	/* initialize the coefficient of topo diffusion and topoGradMax. */
	self->topo_kappa = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( self->dictionary, "topo_kappa", Dictionary_Entry_Value_FromDouble( 5.0e-06f ) ) );
	self->topoGradMax = 0.0f;
	self->topoGradCriterion = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( self->dictionary, "topoGradCriterion", Dictionary_Entry_Value_FromDouble( 1.0e-16 ) ) );

	_Snac_Context_InitializeMaterialProperties( self );

	/* Apply the ICs */
	VariableCondition_Apply( self->nodeICs, self );
	VariableCondition_Apply( self->elementICs, self );

	/* Apply the velocity boundary conditions ... these are considered part of initial conditions */
	VariableCondition_Apply( self->velocityBCs, self );
}

void _Snac_Context_InitializeMaterialProperties( void* context ) {
	Snac_Context*   self = (Snac_Context*)context;
	Dictionary_Entry_Value*         materialList = NULL;
	void getMaterial_Dictionary_Entry_Values( void* _context, Dictionary* materialDict, int phaseI );
	void printMaterial_Properties( void* _context, int phaseI );

	materialList = Dictionary_Get( self->dictionary, "materials" );
	if( materialList ) {
		int phaseI = 0;
		int numPhase = 0;
		Dictionary_Entry_Value* materialEntry;

		/* Count the number of phases. */
		materialEntry = Dictionary_Entry_Value_GetFirstElement( materialList );
		while( materialEntry ) {
			numPhase++;
			materialEntry = materialEntry->next;
		}

		/* assign memory to materialProperty. */
		self->materialProperty = (Snac_Material*)malloc( sizeof(Snac_Material)*numPhase );

		/* Read in phase-dependent material properties */
		materialEntry = Dictionary_Entry_Value_GetFirstElement( materialList );
		while( materialEntry ) {
			Dictionary* materialDict = Dictionary_Entry_Value_AsDictionary( materialEntry );
			getMaterial_Dictionary_Entry_Values( self, materialDict, phaseI );
			if( self->rank == 0 ) printMaterial_Properties( self, phaseI );

			phaseI++;
			materialEntry = materialEntry->next;
		}
	}
	else { /* Read in single-phase material properties */
		self->materialProperty = (Snac_Material*)malloc( sizeof(Snac_Material) );
		getMaterial_Dictionary_Entry_Values( self, self->dictionary, 0 );
		if( self->rank == 0 ) printMaterial_Properties( self, 0 );
	}

	/* Phase-independent constants... */
	self->gravity = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( self->dictionary, "gravity", Dictionary_Entry_Value_FromDouble( 9.81f ) ) );
	self->demf = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( self->dictionary, "demf", Dictionary_Entry_Value_FromDouble( 0.8f ) ) );

	if( self->rank == 0 ) Journal_Printf( self->debug, "snac->gravity: %g\n", self->gravity );
	if( self->rank == 0 ) Journal_Printf( self->debug, "snac->demf: %g\n", self->demf );

}

void getMaterial_Dictionary_Entry_Values( void* _context, Dictionary* materialDict, int phaseI )
{
	Snac_Context* self = (Snac_Context*)_context;
	int             nseg;
	char		name[255];
	char*            plmodel;

	/* TODO: Hard wire the material properties for now */
	self->materialProperty[phaseI].rheology = Snac_Material_Elastic;

	/* density variation */
	self->materialProperty[phaseI].phsDensity = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( materialDict, "density", Dictionary_Entry_Value_FromDouble( 2700.0f ) ) );
	self->materialProperty[phaseI].alpha = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( materialDict, "alpha", Dictionary_Entry_Value_FromDouble( 3.0e-05f ) ) );
	self->materialProperty[phaseI].beta = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( materialDict, "beta", Dictionary_Entry_Value_FromDouble( 0.0f ) ) );

	/* elastic */
	self->materialProperty[phaseI].lambda = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( materialDict, "lambda", Dictionary_Entry_Value_FromDouble( 3.0e+10f ) ) );
	self->materialProperty[phaseI].mu = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( materialDict, "mu", Dictionary_Entry_Value_FromDouble( 3.0e+10f ) ) );

	/* plastic */
	self->materialProperty[phaseI].maxiterations = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( materialDict, "maxiterations", Dictionary_Entry_Value_FromDouble(1) ) );
	self->materialProperty[phaseI].constitutivetolerance = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( materialDict, "constitutivetolerance", Dictionary_Entry_Value_FromDouble(1.e-3) ) );
	plmodel = Dictionary_Entry_Value_AsString(
		Dictionary_GetDefault( materialDict, "yieldcriterion", Dictionary_Entry_Value_FromString("mohrcoulomb" ) ) );
	if( !strcmp( plmodel, "mohrcoulomb" ) )
		self->materialProperty[phaseI].yieldcriterion = mohrcoulomb;
	else if( !strcmp( plmodel, "druckerprager" ) )
		self->materialProperty[phaseI].yieldcriterion = druckerprager;
	self->materialProperty[phaseI].nsegments = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( materialDict, "nsegments",Dictionary_Entry_Value_FromDouble( 2 ) ) );
	self->materialProperty[phaseI].plstrain = (double*)malloc( sizeof(double) * (self->materialProperty[phaseI].nsegments+1) );
	self->materialProperty[phaseI].frictionAngle = (double*)malloc( sizeof(double) * (self->materialProperty[phaseI].nsegments+1) );
	self->materialProperty[phaseI].dilationAngle = (double*)malloc( sizeof(double) * (self->materialProperty[phaseI].nsegments+1) );
	self->materialProperty[phaseI].cohesion = (double*)malloc( sizeof(double) * (self->materialProperty[phaseI].nsegments+1) );
	/* to define n segmetns, n+1 data points are needed */
	for(nseg=0;nseg < self->materialProperty[phaseI].nsegments+1;nseg++) {
		sprintf(name,"plstrain%d",nseg);
		self->materialProperty[phaseI].plstrain[nseg] = Dictionary_Entry_Value_AsDouble(
			Dictionary_GetDefault( materialDict, name,Dictionary_Entry_Value_FromDouble( 0.0 ) ) );
		sprintf(name,"frictionAngle%d",nseg);
		self->materialProperty[phaseI].frictionAngle[nseg] = Dictionary_Entry_Value_AsDouble(
			Dictionary_GetDefault( materialDict, name,Dictionary_Entry_Value_FromDouble( 0.0 ) ) );
		sprintf(name,"dilationAngle%d",nseg);
		self->materialProperty[phaseI].dilationAngle[nseg] = Dictionary_Entry_Value_AsDouble(
			Dictionary_GetDefault( materialDict, name,Dictionary_Entry_Value_FromDouble( 0.0 ) ) );
		sprintf(name,"cohesion%d",nseg);
		self->materialProperty[phaseI].cohesion[nseg] = Dictionary_Entry_Value_AsDouble(
			Dictionary_GetDefault( materialDict, name,Dictionary_Entry_Value_FromDouble( 0.0 ) ) );
	}
	self->materialProperty[phaseI].ten_off = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( materialDict, "ten_off",Dictionary_Entry_Value_FromDouble( 0.0 ) ) );
	self->materialProperty[phaseI].putSeeds = Dictionary_Entry_Value_AsBool(
		Dictionary_GetDefault( materialDict, "putSeeds",Dictionary_Entry_Value_FromBool( False ) ) );

	/* Viscosity: in reality, materialProperty group should be different !!*/
	self->materialProperty[phaseI].vis_min = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( materialDict, "vis_min", Dictionary_Entry_Value_FromDouble( 1.0e18 ) ) );
	self->materialProperty[phaseI].vis_max = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( materialDict, "vis_max", Dictionary_Entry_Value_FromDouble( 3.0e27 ) ) );
	self->materialProperty[phaseI].refvisc = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( materialDict, "refvisc", Dictionary_Entry_Value_FromDouble( 1.0e19 ) ) );
	self->materialProperty[phaseI].refsrate = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( materialDict, "refsrate", Dictionary_Entry_Value_FromDouble( 1.0e-15 ) ) );
	self->materialProperty[phaseI].reftemp = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( materialDict, "reftemp", Dictionary_Entry_Value_FromDouble( 1400.0 ) ) );
	self->materialProperty[phaseI].activationE = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( materialDict, "activationE", Dictionary_Entry_Value_FromDouble( 5.4e03 ) ) );
	self->materialProperty[phaseI].activationV = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( materialDict, "activationV", Dictionary_Entry_Value_FromDouble( 2.5e-05 ) ) );
	self->materialProperty[phaseI].srexponent = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( materialDict, "srexponent", Dictionary_Entry_Value_FromDouble( 1.0 ) ) );
	self->materialProperty[phaseI].srexponent1 = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( materialDict, "srexponent1", Dictionary_Entry_Value_FromDouble( 1.0 ) ) );
	self->materialProperty[phaseI].srexponent2 = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( materialDict, "srexponent2", Dictionary_Entry_Value_FromDouble( 1.0 ) ) );
	/* Thermal properties. */
	self->materialProperty[phaseI].thermal_conduct = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( materialDict, "thermal_conduct", Dictionary_Entry_Value_FromDouble( 2.0 ) ) );
	self->materialProperty[phaseI].heatCapacity = Dictionary_Entry_Value_AsDouble(
		Dictionary_GetDefault( materialDict, "heatCapacity", Dictionary_Entry_Value_FromDouble( 1000.0 ) ) );

}

void printMaterial_Properties( void* _context, int phaseI )
{
	Snac_Context* self = (Snac_Context*)_context;
	int             nseg;

	Journal_Printf( self->snacInfo, "\nFor Material %d:\n",phaseI );

	Journal_Printf( self->snacInfo, "\trheology = %d\n", self->materialProperty[phaseI].rheology );

	/* density variation */
	Journal_Printf( self->snacInfo, "\talpha = %e\n", self->materialProperty[phaseI].alpha );
	Journal_Printf( self->snacInfo, "\tbeta = %e\n", self->materialProperty[phaseI].beta );

	Journal_Printf( self->snacInfo, "\n");

	/* elastic */
	Journal_Printf( self->snacInfo, "\tlambda = %e\n", self->materialProperty[phaseI].lambda );
	Journal_Printf( self->snacInfo, "\tmu = %e\n", self->materialProperty[phaseI].mu );

	Journal_Printf( self->snacInfo, "\n");

	/* plastic */
	Journal_Printf( self->snacInfo, "\tmaxiterations = %d\n", self->materialProperty[phaseI].maxiterations );
	Journal_Printf( self->snacInfo, "\tconstitutivetolerance = %e\n", self->materialProperty[phaseI].constitutivetolerance );
	Journal_Printf( self->snacInfo, "\tyieldcriterion = %d\n", self->materialProperty[phaseI].yieldcriterion );
	Journal_Printf( self->snacInfo, "\tnsegments = %d\n", self->materialProperty[phaseI].nsegments );
	/* to define n segmetns, n+1 data points are needed */
	for(nseg=0;nseg < self->materialProperty[phaseI].nsegments+1;nseg++) {
		Journal_Printf( self->snacInfo, "\t\tseg %d: plstrain = %e\n", nseg,self->materialProperty[phaseI].plstrain[nseg] );
		Journal_Printf( self->snacInfo, "\t\tseg %d: frictionAngle = %e\n",nseg,self->materialProperty[phaseI].frictionAngle[nseg] );
		Journal_Printf( self->snacInfo, "\t\tseg %d: dilationAngle = %e\n",nseg,self->materialProperty[phaseI].dilationAngle[nseg] );
		Journal_Printf( self->snacInfo, "\t\tseg %d: cohesion = %e\n",nseg,self->materialProperty[phaseI].cohesion[nseg] );
	}
	Journal_Printf( self->snacInfo, "\tten_off = %e\n", self->materialProperty[phaseI].ten_off );
	Journal_Printf( self->snacInfo, "\tpuSeeds = %d\n", self->materialProperty[phaseI].putSeeds );

	Journal_Printf( self->snacInfo, "\n");

	/* Viscosity: in reality, materialProperty group should be different !!*/
	Journal_Printf( self->snacInfo, "\tvis_min = %e\n", self->materialProperty[phaseI].vis_min );
	Journal_Printf( self->snacInfo, "\tvis_max = %e\n", self->materialProperty[phaseI].vis_max );
	Journal_Printf( self->snacInfo, "\trefvisc = %e\n", self->materialProperty[phaseI].refvisc );
	Journal_Printf( self->snacInfo, "\trefsrate = %e\n", self->materialProperty[phaseI].refsrate );
	Journal_Printf( self->snacInfo, "\treftemp = %e\n", self->materialProperty[phaseI].reftemp );
	Journal_Printf( self->snacInfo, "\tactivationE = %e\n", self->materialProperty[phaseI].activationE );
	Journal_Printf( self->snacInfo, "\tsrexponent = %e\n", self->materialProperty[phaseI].srexponent );
	Journal_Printf( self->snacInfo, "\tsrexponent1 = %e\n", self->materialProperty[phaseI].srexponent1 );
	Journal_Printf( self->snacInfo, "\tsrexponent2 = %e\n", self->materialProperty[phaseI].srexponent2 );

	Journal_Printf( self->snacInfo, "\n");

	Journal_Printf( self->snacInfo, "\tthermal conductivity = %e\n", self->materialProperty[phaseI].thermal_conduct );
	Journal_Printf( self->snacInfo, "\theat capacity = %e\n", self->materialProperty[phaseI].heatCapacity );

	/* temporary variable: will eventually replace self->density */
	Journal_Printf( self->snacInfo, "\tdensity = %e\n", self->materialProperty[phaseI].phsDensity );
}

/*------------------------------------------------------------------------------------------------------------------------*/
 /** Do a pass through "UpdateElementMomentum" now... this is because this should be the first thing in the solve loop, but
 * we place it as the last thing for optimisation reasons. Hence, we consider this "timeStep 0", as we dump all values for verifcation
 * purposes. A consequence of this is that timeSteps 0 and 1 are at time "self->startTime".*/
void Snac_Context_TimeStepZero( void* context ) {
	Snac_Context* self = (Snac_Context*)context;
	Element_LocalIndex	element_lI;

#ifdef DEBUGscaling
	fprintf(stderr, "TimeStepZero:  restartTimestep=%d,  timeStep=%d\n", self->restartTimestep, self->timeStep);
#endif

	_Snac_Context_InitOutput( self );

	if( self->rank == 0 ) Journal_Printf( self->snacInfo, "In: %s\n", __func__ );
	if( self->rank == 0 ) Journal_Printf(
		self->info,
		"self->timeStep: %u (update elements only)\nself->currentTime: %g\n",
		self->timeStep,
		self->currentTime );

	/* Write out timeStep info */
	_Snac_Context_WriteLoopInfo( self );

	if( self->restartTimestep == 0 ) {
		for( element_lI = 0; element_lI < self->mesh->elementLocalCount && self->restartTimestep == 0; element_lI++ ) {
			Snac_Element* element = Snac_Element_At( self, element_lI );
			Material_Index                  material_I = element->material_I;
			Snac_Material*          material = &self->materialProperty[material_I];
			Density                 phsDensity = material->phsDensity; // node->density
			Tetrahedra_Index	tetra_I;
			double sVolAvg = 0.0f;

			/* During IC the stress tensors on the tetrahedres may be initialised, but the element stress scalar are
			   not, as they require the tetrahedra volumes to be calculated (done above in Snac_UpdateElementMomentum,
			   and obviously after IC). For every other timeStep, this is calculated for us in Stress.c. Hence, we do
			   it now. */
			for( tetra_I = 0, element->stress = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
				sVolAvg +=
					element->tetra[tetra_I].stress[1][1] * element->tetra[tetra_I].stress[2][2] +
					element->tetra[tetra_I].stress[2][2] * element->tetra[tetra_I].stress[0][0] +
					element->tetra[tetra_I].stress[0][0] * element->tetra[tetra_I].stress[1][1];
				element->tetra[tetra_I].avgTemp = 0.0f;
				element->tetra[tetra_I].density = phsDensity;
			}
			element->stress = 0.5f * sqrt( 0.5f * fabs( sVolAvg/Tetrahedra_Count ) );
		}
	}

	/* Update all the elements, and in the process work out this processor's minLengthScale */
	KeyCall( self, self->loopElementsMomentumK, EntryPoint_VoidPtr_CallCast* )( KeyHandle(self,self->loopElementsMomentumK), self );

	_Snac_Context_WriteOutput( self );

	KeyCall( self, self->syncK, EntryPoint_Class_VoidPtr_CallCast* )( KeyHandle(self,self->syncK), self );
	/* _Snac_Context_Sync( self ); */
}


double _Snac_Context_Dt( void* context ) {
	Snac_Context*	self = (Snac_Context*)context;

	if( self->rank == 0 ) Journal_Printf( self->debug, "In: %s\n", __func__ );

	return self->dt;
}

void _Snac_Context_SetDt( void* context, double dt ) {
	Snac_Context*	self = (Snac_Context*)context;

	if( self->rank == 0 ) Journal_Printf( self->debug, "In: %s\n", __func__ );

	self->dt = dt;
}


void _Snac_Context_BoundaryConditions( void* context ) {
	Snac_Context* self = (Snac_Context*)context;

	if( self->rank == 0 ) Journal_Printf( self->debug, "In: %s\n", __func__ );
}


void _Snac_Context_Solve( void* context ) {
	Snac_Context* 		self = (Snac_Context*)context;

	if( self->rank == 0 ) Journal_DPrintf( self->debug, "In: %s\n", __func__ );

	_Snac_Context_WriteLoopInfo( self );

	/* Perform the Snac solve loop */
	/* update the energy solve */

	KeyCall( self, self->loopElementsEnergyK, EntryPoint_VoidPtr_CallCast* )( KeyHandle(self,self->loopElementsEnergyK), self );
	KeyCall( self, self->loopNodesEnergyK, EntryPoint_VoidPtr_CallCast* )( KeyHandle(self,self->loopNodesEnergyK), self );

	KeyCall( self, self->calcStressesK, EntryPoint_VoidPtr_CallCast* )( KeyHandle(self,self->calcStressesK), self );

	/* Synchronise */
	if( self->forceCalcType == Snac_Force_Complete &&
	    self->mesh->layout->decomp->procsInUse > 1 )
	{
		Mesh_Sync( self->mesh );
	}

	KeyCall( self, self->loopNodesMomentumK, EntryPoint_VoidPtr_CallCast* )( KeyHandle(self,self->loopNodesMomentumK), self );

	KeyCall( self, self->loopElementsMomentumK, EntryPoint_VoidPtr_CallCast* )( KeyHandle( self, self->loopElementsMomentumK ), self );

	_Snac_Context_WriteOutput( self );

#if 0
	/* Synchronise again, but really need to consolidate with the other one... */
	if( self->forceCalcType == Snac_Force_Complete &&
	    self->mesh->layout->decomp->procsInUse > 1 )
	{
		Mesh_Sync( self->mesh );
	}
#endif
}


void _Snac_Context_CalcStresses( void* context ) {
	Snac_Context* 		self = (Snac_Context*)context;
	Element_LocalIndex	element_dI;

	if( self->rank == 0 ) Journal_DPrintf( self->debug, "In: %s\n", __func__ );
	if( self->rank == 0 ) Journal_Printf( self->verbose, "For each element, calculating strain-rate and then stress\n" );

	/* For each element, calculate strain-rate and then stress */
	for( element_dI = 0; element_dI < self->mesh->elementLocalCount; element_dI++ ) {
		KeyCall( self, self->strainRateK, Snac_StrainRate_CallCast* )(
			KeyHandle( self, self->strainRateK ),
			self,
			element_dI );
		KeyCall( self, self->stressK, Snac_Stress_CallCast* )( KeyHandle( self, self->stressK ), self, element_dI );
	}
}


void _Snac_Context_LoopNodes( void* context ) {
	Snac_Context* 		self = (Snac_Context*)context;
	Node_LocalIndex		node_lI;

	if( self->rank == 0 ) Journal_DPrintf( self->debug, "In: %s\n", __func__ );
	if( self->rank == 0 ) Journal_Printf(
		self->verbose,
		"For each node, calculate mass, force, velocity and then coordinates\n" );

	for( node_lI = 0; node_lI < self->mesh->nodeLocalCount; node_lI++ ) {
		Snac_Node*		node = Snac_Node_At( self, node_lI );
		Mass			mass;
		Force			balance;
		
		KeyCall( self, self->forceK, Snac_Force_CallCast* )(
				KeyHandle(self,self->forceK),
				self,
				node_lI,
				self->speedOfSound,
				&mass,
				&(node->inertialMass),
				&(node->force),
				&balance );
	}
	for( node_lI = 0; node_lI < self->mesh->nodeLocalCount; node_lI++ ) {
		Snac_Node*		node = Snac_Node_At( self, node_lI );

		KeyCall( self, self->updateNodeK, Snac_UpdateNodeMomentum_CallCast* )(
				KeyHandle(self,self->updateNodeK),
				self,
				node_lI,
				node->inertialMass,
				node->force );
	}
}

void _Snac_Context_LoopElements( void* context ) {
	Snac_Context* 		self = (Snac_Context*)context;
	Element_LocalIndex	element_lI;

	if( self->rank == 0 ) Journal_DPrintf( self->debug, "In: %s\n", __func__ );
	if( self->rank == 0 ) Journal_Printf(
		self->verbose,
		"For each element, calc volume, surface vel, and then min length scale\n" );

	/* Update all the elements, and in the process work out this processor's minLengthScale */
	element_lI = 0;
	KeyCall( self, self->updateElementK, Snac_UpdateElementMomentum_CallCast* )(
		KeyHandle(self,self->updateElementK),
		self,
		element_lI,
		&self->minLengthScale );
	for( element_lI = 1; element_lI < self->mesh->elementLocalCount; element_lI++ ) {
		double elementMinLengthScale;

		KeyCall( self, self->updateElementK, Snac_UpdateElementMomentum_CallCast* )
			( KeyHandle(self,self->updateElementK),
			self,
			element_lI,
			&elementMinLengthScale );
		if( elementMinLengthScale < self->minLengthScale ) {
			self->minLengthScale = elementMinLengthScale;
		}
	}
}


void _Snac_Context_Sync( void* context ) {
	Snac_Context*		self = (Snac_Context*)context;
	Node_DomainIndex	node_dI;
	double			tmp,v,vx,vy,vz,vmax,vlb;
	IndexSet_Index		index;

	if( self->rank == 0 ) Journal_DPrintf( self->debug, "In: %s\n", __func__ );

	/* Find the global minLengthScale, and then calculate the new dt and speed of sound, assuming frac=0.45,
	    strain_inert=1.0e-5f & vbc_max=3.0e-10f */
	vmax=0.0f;
	vlb = 3.0e-11f; /* ~1 mm/yr */

	for( index = 0; index < self->velocityBCs->indexCount; index++ ) {
		Snac_Node* node;
		node_dI = self->velocityBCs->indexTbl[index];
		node = Snac_Node_At( self, node_dI );
		vx = node->velocity[0];
		vy = node->velocity[1];
		vz = node->velocity[2];

		v=sqrt(vx*vx + vy*vy + vz*vz);
		if(v>vmax) vmax=v;
	}

	MPI_Allreduce( &vmax, &v, 1, MPI_DOUBLE, MPI_MAX, self->communicator );
	vmax = v;
	if(vmax < vlb) vmax = vlb;

	MPI_Allreduce( &self->minLengthScale, &tmp, 1, MPI_DOUBLE, MPI_MIN, self->communicator );
	self->minLengthScale = tmp;
	if(self->timeStep==self->restartTimestep && self->restartTimestep==0) {
		FILE* fp;
		char fname[PATH_MAX];
		self->initMinLengthScale = self->minLengthScale;
		assert( self->initMinLengthScale > 0 );
		sprintf( fname, "%s/minLengthScale.0", self->outputPath );
		Journal_Firewall( (fp = fopen( fname, "w" )), "Failed to open %s!!\n", fname );
		fprintf(fp,"%e",self->initMinLengthScale);
		fclose( fp );
	}

	if( self->dtType == Snac_DtType_Dynamic ) {
		self->dt = self->minLengthScale * 0.45f * self->strain_inert / vmax;

		self->speedOfSound = self->minLengthScale * 0.5f / self->dt;
	}

	if( self->rank == 0 ) {
		Journal_Printf( self->debug, "new self->minlengthscale: %g\n", self->minLengthScale );
		Journal_Printf( self->debug, "new vmax: %g\n", vmax );
		Journal_Printf( self->debug, "new self->dt: %g L/Vmax = %g\n", self->dt,self->minLengthScale/vmax );
		Journal_Printf( self->debug, "new speed of sound: %g\n", self->speedOfSound );
	}
	
}


void _Snac_Context_WriteLoopInfo( void* context ) {
	Snac_Context* self = (Snac_Context*)context;
	/*
	time_t now=time(NULL);
	char *cnow;
	cnow=ctime(&now);

	if (self->rank==1)
            fprintf( self->timeStepInfo, "Internal time (%d) = %s\n", self->timeStep, cnow);
	*/

	if( self->rank >0 )
		return;
	
	if( isTimeToDump( self ) ) 
		_Snac_Context_DumpLoopInfo( self );

	if( isTimeToCheckpoint( self ) )
		_Snac_Context_CheckpointLoopInfo( self );

}


void _Snac_Context_DumpLoopInfo( void* context ) {
	Snac_Context* self = (Snac_Context*)context;

	/*
	fprintf( self->timeStepInfo, "%16u %16g %16g\n", self->timeStep, self->currentTime, self->dt );
	fflush( self->timeStepInfo );
	*/
}


void _Snac_Context_CheckpointLoopInfo( void* context ) {
	Snac_Context* self = (Snac_Context*)context;

	fprintf( self->checkpointTimeStepInfo, "%16u %16g %16g\n", self->timeStep, self->currentTime, self->dt );
	fflush( self->checkpointTimeStepInfo );
}


void _Snac_Context_InitOutput( void* context ) {
	Snac_Context* self = (Snac_Context*)context;

	_Snac_Context_InitDump( self );
	_Snac_Context_InitCheckpoint( self );

}


void _Snac_Context_InitDump( void* context ) {
	Snac_Context* self = (Snac_Context*)context;
	char tmpBuf[200];

	/* Create the strain rate dumping stream */
	self->strainRateOut = Journal_Register( VariableDumpStream_Type, "StrainRate" );
	/*
	sprintf( tmpBuf, "%s/strainRate.%u", self->outputPath, self->rank );
	VariableDumpStream_SetVariable(
		self->strainRateOut,
		Variable_Register_GetByName( self->variable_Register, "strainRate" ),
		self->mesh->elementLocalCount,
		self->dumpEvery,
		tmpBuf );
	*/

	/* Create the stress dumping stream */
	self->stressOut = Journal_Register( VariableDumpStream_Type, "Stress" );
	/*
	sprintf( tmpBuf, "%s/stress.%u", self->outputPath, self->rank );
	VariableDumpStream_SetVariable(
		self->stressOut,
		Variable_Register_GetByName( self->variable_Register, "stress" ),
		self->mesh->elementLocalCount,
		self->dumpEvery,
		tmpBuf );
	*/

	/* Create the pressure  dumping stream */
	self->hydroPressureOut = Journal_Register( VariableDumpStream_Type, "HydroPressure" );
	/*
	sprintf( tmpBuf, "%s/hydroPressure.%u", self->outputPath, self->rank );
	VariableDumpStream_SetVariable(
		self->hydroPressureOut,
		Variable_Register_GetByName( self->variable_Register, "hydroPressure" ),
		self->mesh->elementLocalCount,
		self->dumpEvery,
		tmpBuf );
	*/

	/* Create the phase index dumping stream */
	self->phaseIndexOut = Journal_Register( VariableDumpStream_Type, "PhaseIndex" );
	/*	
	sprintf( tmpBuf, "%s/phaseIndex.%u", self->outputPath, self->rank );
	VariableDumpStream_SetVariable(
		self->phaseIndexOut,
		Variable_Register_GetByName( self->variable_Register, "elementMaterial" ),
		self->mesh->elementLocalCount,
		self->dumpEvery,
		tmpBuf );
	*/

	/* Create the coords dumping stream */
	self->coordOut = Journal_Register( VariableDumpStream_Type, "Coord" );
	/*
	sprintf( tmpBuf, "%s/coord.%u", self->outputPath, self->rank );
	VariableDumpStream_SetVariable(
		self->coordOut,
		Variable_Register_GetByName( self->variable_Register, "coord" ),
		self->mesh->nodeLocalCount,
		self->dumpEvery,
		tmpBuf );
	*/

	/* Create the velocity dumping stream */
	self->velOut = Journal_Register( VariableDumpStream_Type, "Velocity" );
	/*	
	sprintf( tmpBuf, "%s/vel.%u", self->outputPath, self->rank );
	VariableDumpStream_SetVariable(
		self->velOut,
		Variable_Register_GetByName( self->variable_Register, "velocity" ),
		self->mesh->nodeLocalCount,
		self->dumpEvery,
		tmpBuf );
	*/

	/* Create the force dumping stream */
	self->forceOut = Journal_Register( VariableDumpStream_Type, "Forces" );

	/*
	sprintf( tmpBuf, "%s/force.%u", self->outputPath, self->rank );
	VariableDumpStream_SetVariable(
		self->forceOut,
		Variable_Register_GetByName( self->variable_Register, "force" ),
		self->mesh->nodeLocalCount,
		self->dumpEvery,
		tmpBuf );
	*/

	/* Create the stressTensor dump file */
	/*
	sprintf( tmpBuf, "%s/stressTensor.%u", self->outputPath, self->rank );
	if( (self->stressTensorOut = fopen( tmpBuf, "w+" )) == NULL ) {
		assert( self->stressTensorOut );
	}
	*/

}


void _Snac_Context_InitCheckpoint( void* context ) {
	Snac_Context* self = (Snac_Context*)context;
	char tmpBuf[200];

	/* Create the phase index dumping stream */
	self->phaseIndexCheckpoint = Journal_Register( VariableDumpStream_Type, "PhaseIndexCP" );
	
	/*
	sprintf( tmpBuf, "%s/phaseIndexCP.%u", self->outputPath, self->rank );
	VariableDumpStream_SetVariable(
		self->phaseIndexCheckpoint,
		Variable_Register_GetByName( self->variable_Register, "elementMaterial" ),
		self->mesh->elementLocalCount,
		self->checkpointEvery,
		tmpBuf );
	*/

	/* Create the coords dumping stream */
	self->coordCheckpoint = Journal_Register( VariableDumpStream_Type, "CoordCP" );

	/*
	sprintf( tmpBuf, "%s/coordCP.%u", self->outputPath, self->rank );
	VariableDumpStream_SetVariable(
		self->coordCheckpoint,
		Variable_Register_GetByName( self->variable_Register, "coord" ),
		self->mesh->nodeLocalCount,
		self->checkpointEvery,
		tmpBuf );
	*/

	/* Create the velocity dumping stream */
	self->velCheckpoint = Journal_Register( VariableDumpStream_Type, "VelocityCP" );
	/*
	sprintf( tmpBuf, "%s/velCP.%u", self->outputPath, self->rank );
	VariableDumpStream_SetVariable(
		self->velCheckpoint,
		Variable_Register_GetByName( self->variable_Register, "velocity" ),
		self->mesh->nodeLocalCount,
		self->checkpointEvery,
		tmpBuf );
	*/

	/* Create the stressTensor dump file */
	/*
	sprintf( tmpBuf, "%s/stressTensorCP.%u", self->outputPath, self->rank );
	if( (self->stressTensorCheckpoint = fopen( tmpBuf, "w+" )) == NULL ) {
		assert( self->stressTensorCheckpoint );
		abort();
	}
	*/
}


void _Snac_Context_WriteOutput( void* context ) {
	Snac_Context* self = (Snac_Context*)context;

	return;

	if( isTimeToDump( self ) )
		_Snac_Context_Dump( self );

	if( isTimeToCheckpoint( self ) )
		_Snac_Context_Checkpoint( self );
}


void _Snac_Context_Dump( void* context ) {
	Snac_Context* self = (Snac_Context*)context;
	
	Journal_Dump( self->strainRateOut, NULL );
	Journal_Dump( self->stressOut, NULL );
	Journal_Dump( self->hydroPressureOut, NULL );
	_Snac_Context_DumpStressTensor( self );
	Journal_Dump( self->coordOut, NULL );
	Journal_Dump( self->velOut, NULL );
	Journal_Dump( self->forceOut, NULL );
	Journal_Dump( self->phaseIndexOut, NULL );

	Stream_Flush( self->strainRateOut );
	Stream_Flush( self->stressOut );
	Stream_Flush( self->hydroPressureOut );
	Stream_Flush( self->phaseIndexOut );
	Stream_Flush( self->coordOut );
	Stream_Flush( self->velOut );
	Stream_Flush( self->forceOut );
	fflush(	self->stressTensorOut );
	fflush( NULL );
}


void _Snac_Context_Checkpoint( void* context ) {
	Snac_Context* self = (Snac_Context*)context;

	_Snac_Context_CheckpointStressTensor( self );
	Journal_Dump( self->coordCheckpoint, NULL );
	Journal_Dump( self->velCheckpoint, NULL );
	Journal_Dump( self->phaseIndexCheckpoint, NULL );

	fflush( self->stressTensorCheckpoint );
	Stream_Flush( self->coordCheckpoint );
	Stream_Flush( self->velCheckpoint );
	Stream_Flush( self->phaseIndexCheckpoint );
}


void _Snac_Context_DumpStressTensor( void* context ) {
	Snac_Context* self = (Snac_Context*)context;
	
	Element_LocalIndex			element_lI;
	
	for( element_lI = 0; element_lI < self->mesh->elementLocalCount; element_lI++ ) {
		Snac_Element* 				element = Snac_Element_At( self, element_lI );
		Tetrahedra_Index		tetra_I;
		float stressVector[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
		float totalVolume = 0.0;
		
		/* Take average of tetra stress for the element */
		for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) 
			totalVolume += element->tetra[tetra_I].volume;
		
		for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
			stressVector[0] += element->tetra[tetra_I].stress[0][0]*element->tetra[tetra_I].volume/totalVolume;
			stressVector[1] += element->tetra[tetra_I].stress[1][1]*element->tetra[tetra_I].volume/totalVolume;
			stressVector[2] += element->tetra[tetra_I].stress[2][2]*element->tetra[tetra_I].volume/totalVolume;
			stressVector[3] += element->tetra[tetra_I].stress[0][1]*element->tetra[tetra_I].volume/totalVolume;
			stressVector[4] += element->tetra[tetra_I].stress[0][2]*element->tetra[tetra_I].volume/totalVolume;
			stressVector[5] += element->tetra[tetra_I].stress[1][2]*element->tetra[tetra_I].volume/totalVolume;
		}
		fwrite( &stressVector, sizeof(float), 6, self->stressTensorOut );
	}
}


void _Snac_Context_CheckpointStressTensor( void* context ) {
	Snac_Context* self = (Snac_Context*)context;
	
	Element_LocalIndex			element_lI;
	
	for( element_lI = 0; element_lI < self->mesh->elementLocalCount; element_lI++ ) {
		Snac_Element* 				element = Snac_Element_At( self, element_lI );
		/* Take average of tetra viscosity for the element */
		Tetrahedra_Index		tetra_I;
		
		/* Write the stress vector for all the tets. This is for restarting. */
		for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
			float stressVector[6] = { element->tetra[tetra_I].stress[0][0], element->tetra[tetra_I].stress[1][1], element->tetra[tetra_I].stress[2][2],
									  element->tetra[tetra_I].stress[0][1], element->tetra[tetra_I].stress[0][2], element->tetra[tetra_I].stress[1][2]};
			fwrite( &stressVector, sizeof(float), 6, self->stressTensorCheckpoint );
		}
	}
}


Bool isTimeToDump( void* context ) {
	Snac_Context* self = (Snac_Context*)context;

	return False;
	
	if( (self->timeStep==1) || ((self->timeStep>1)&&(self->timeStep%self->dumpEvery==0)) ) 
		return True;
	else
		return False;
	
}


Bool isTimeToCheckpoint( void* context ) {
	Snac_Context* self = (Snac_Context*)context;

	return False;

	if( (self->timeStep==self->maxTimeSteps) )
		return True;
	else if( (self->timeStep==0) )
		return False;
	else {
		if( self->checkpointEvery == 0 )
			return False;
		else if( (self->timeStep%self->checkpointEvery==0) )
			return True;
		else
			return False;
	}
}
