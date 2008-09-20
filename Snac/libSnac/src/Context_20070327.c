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
** $Id: Context.c 3267 2006-11-15 21:51:36Z EunseoChoi $
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
#ifndef PATH_MAX
	#define PATH_MAX 1024
#endif

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

	/* Building StGermain Variables of nodal quantities... we must the "complex" constructor for Variable... the
	 * info needs to be wrapped this generic way... */
	Index			nodeOffsetCount = 4;
	SizeT			nodeOffsets[] = {
					GetOffsetOfMember( tmpNode, velocity ),
					GetOffsetOfMember( tmpNode, velocity[0] ),
					GetOffsetOfMember( tmpNode, velocity[1] ),
					GetOffsetOfMember( tmpNode, velocity[2] ) };
	Variable_DataType	nodeDataTypes[] = {
					Variable_DataType_Double,
					Variable_DataType_Double,
					Variable_DataType_Double,
					Variable_DataType_Double };
	Index			nodeDataTypeCounts[] = {
					3,
					1,
					1,
					1 };
	Name			nodeNames[] = {
					"velocity",
					"vx",
					"vy",
					"vz" };

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
	self->restartStep = Dictionary_Entry_Value_AsUnsignedInt(
		Dictionary_GetDefault( self->dictionary, "restartStep", Dictionary_Entry_Value_FromUnsignedInt( 0 ) ) );

	tmpStr = Dictionary_Entry_Value_AsString( Dictionary_Get( self->dictionary, "dtType" ) );
	if( !strcmp( tmpStr, Snac_DtType_Constant ) ) {
		/* Set dtType to "constant" using known pointer/key... it means checks are pointer comparisons only, not strcmp */
		self->dtType = Snac_DtType_Constant;

		/* When constant Dt type is specified, Dt should be given an initial-non-zero value */
		self->dt = Dictionary_Entry_Value_AsDouble(
			Dictionary_GetDefault( self->dictionary, "timeStep", Dictionary_Entry_Value_FromDouble( 1.0 ) ) );

		Journal_Printf( self->info, "\"dtType\" set by Dictionary to \"%s\"\n", self->dtType );
	}
	else if( !strcmp( tmpStr, Snac_DtType_Dynamic ) ) {
		/* Set dtType to "dynamic" using known pointer/key... it means checks are pointer comparisons only, not strcmp */
		self->dtType = Snac_DtType_Dynamic;

		/* When dynamic Dt type is specified, Dt will be calculated later, give 0 as starting value... legacy behaviour. */
		self->dt = Dictionary_Entry_Value_AsDouble(
			Dictionary_GetDefault( self->dictionary, "timeStep", Dictionary_Entry_Value_FromDouble( 0.0 ) ) );

		Journal_Printf( self->info, "\"dtType\" set by Dictionary to \"%s\"\n", self->dtType );
	}
	else {
		/* Set dtType to "dynamic" using known pointer/key... it means checks are pointer comparisons only, not strcmp */
		/* Warn the user that this assumption has occured */
		self->dtType = Snac_DtType_Dynamic;

		/* When dynamic Dt type is specified, Dt will be calculated later, give 0 as starting value... legacy behaviour. */
		self->dt = Dictionary_Entry_Value_AsDouble(
			Dictionary_GetDefault( self->dictionary, "timeStep", Dictionary_Entry_Value_FromDouble( 0.0 ) ) );

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

	/* Store forces on nodes? is so, do it before variables are registered, such that input can assign to it too. */
	self->storeForces = Dictionary_Entry_Value_AsBool(
		Dictionary_GetDefault( self->dictionary, "storeForces", Dictionary_Entry_Value_FromBool( True )) );
	if( self->storeForces ) {
		self->storeForcesMeshHandle = ExtensionManager_Add( self->meshExtensionMgr, Snac_Context_Type, sizeof(Snac_Mesh) );
	}
	else {
		self->storeForcesMeshHandle = (unsigned)-1;
	}

	/* What method are we using to calculate forces? */
	tmpStr = Dictionary_Entry_Value_AsString(
		Dictionary_GetDefault( self->dictionary, "forceCalcType", Dictionary_Entry_Value_FromString( "quick" ) ) );
	if( !strcmp( "quick", tmpStr ) ) {
		self->forceCalcType = Snac_Force_Quick;
		Journal_Printf( self->info, "\"forceCalcType\" set by Dictionary to \"quick\"\n" );
	}
	else if( !strcmp( "accurate", tmpStr ) ) {
		self->forceCalcType = Snac_Force_Accurate;
		Journal_Printf( self->info, "\"forceCalcType\" set by Dictionary to \"accurate\"\n" );
	}
	else if( !strcmp( "complete", tmpStr ) ) {
		self->forceCalcType = Snac_Force_Complete;
		Journal_Printf( self->info, "\"forceCalcType\" set by Dictionary to \"complete\"\n" );
	}
	else { /* "normal" */
		self->forceCalcType = Snac_Force_Normal;
		Journal_Printf( self->info, "\"forceCalcType\" set by Dictionary to \"normal\"\n" );
	}
	self->decomposedAxis = Dictionary_Entry_Value_AsUnsignedInt(
		Dictionary_GetDefault( self->dictionary, "decomposedAxis", Dictionary_Entry_Value_FromUnsignedInt( 0 ) ) );
	if( self->nproc > 1 ) {
		Journal_Printf( self->info, "\"decomposedAxis\" set by Dictionary to: %u\n", self->decomposedAxis );
	}

	/* Are we performing the  force checksum? */
	self->doForceCheckSum = Dictionary_Entry_Value_AsBool(
		Dictionary_GetDefault( self->dictionary, "forceCheckSum", Dictionary_Entry_Value_FromBool( False )) );
	if( self->doForceCheckSum ) {
		/* Store forces is required, and so is the building of local/domain to global tables */
		Journal_Firewall( self->storeForces, self->snacError, "doForceCheckSum without storeForces set" );
		if( !self->mesh->buildNodeDomainToGlobalMap ) {
			self->mesh->buildNodeDomainToGlobalMap = True;
			Dictionary_Set(
				self->mesh->dictionary,
				"buildNodeDomainToGlobalMap",
				Dictionary_Entry_Value_FromBool( True ) );
		}
	}
	self->forceCheckSum[0] = 0.0f;
	self->forceCheckSum[1] = 0.0f;
	self->forceCheckSum[2] = 0.0f;
	self->nCount = 0;
	self->forceTmp = 0;
	self->forceTmpArray = 0;
	self->force = 0;

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

	/* Output */
	sprintf( tmpBuf, "%s/sim.%u", self->outputPath, self->rank );
	if( (self->simInfo = fopen( tmpBuf, "w+" )) == NULL ) {
		Journal_Firewall(
			(ArithPointer)self->simInfo,
			self->snacError,
			"\"%s\"  failed to open file for writing", tmpBuf );
	}
	else {
		fprintf( self->simInfo, "%u %u %u\n",
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
	sprintf( tmpBuf, "%s/forceCheckSum.%u", self->outputPath, self->rank );
	if( (self->forceCheckSumOut = fopen( tmpBuf, "w+" )) == NULL ) {
		Journal_Firewall(
			(ArithPointer)self->forceCheckSumOut,
			self->snacError,
			"\"%s\"  failed to open file for writing", tmpBuf );
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
	self->forceBC_K =
		Context_AddEntryPoint(
			self,
			Snac_EntryPoint_New( Snac_EP_ForceBC, Snac_Force_CastType ) );
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

	/* Add hooks to new entry points... no energy or forceBC hooks by default */
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

	fclose( self->forceCheckSumOut );
	fclose( self->timeStepInfo );
	fclose( self->simInfo );
	fclose( self->stressTensorOut );
	fclose( self->phaseIndexOut );

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

	/* if storing forces, delete it */
	if( self->storeForces ) {
		Snac_Mesh_Delete( ExtensionManager_Get( self->meshExtensionMgr, self->mesh, self->storeForcesMeshHandle ) );
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

	/* If storing forces on nodes, build it */
	if( self->storeForces ) {
		Snac_Mesh* meshExt = ExtensionManager_Get( self->meshExtensionMgr, self->mesh, self->storeForcesMeshHandle );
		Snac_Mesh_Build( meshExt, self );
	}

	if( self->doForceCheckSum ) {
		Snac_Mesh*		meshExt = ExtensionManager_Get( self->meshExtensionMgr, self->mesh, self->storeForcesMeshHandle );

		/* Obtain the node local counts from the other ranks, and allocate memory */
		if( self->rank == 0 ) {
			MPI_Status		s;
			PartitionIndex		rank_I;

			self->nCount = (Node_LocalIndex*)malloc( sizeof(Node_LocalIndex) * self->nproc );
			self->nCount[0] = meshExt->nodeLocalCount;
			self->forceTmp = (Force**)malloc( sizeof(Force*) * self->nproc );
			self->forceTmp[0] = meshExt->force;
			self->forceTmpArray = (Node_GlobalIndex**)malloc( sizeof(Node_GlobalIndex*) * self->nproc );
			self->forceTmpArray[0] = self->mesh->nodeL2G;
			for( rank_I = 1; rank_I < self->nproc; rank_I++ ) {
				MPI_Recv( &self->nCount[rank_I], 1, MPI_UNSIGNED, rank_I, 1026, self->communicator, &s );
				self->forceTmp[rank_I] = (Force*)malloc( sizeof(Force) * self->nCount[rank_I] * 3 );
				self->forceTmpArray[rank_I] = (Node_GlobalIndex*)malloc( sizeof(Node_GlobalIndex) *
					self->nCount[rank_I] );
			}

			self->force = (Force*)malloc( sizeof(Force) * self->mesh->nodeGlobalCount );

			/* Build the node mapping table... seperate loop to keep messages in order */
			for( rank_I = 1; rank_I < self->nproc; rank_I++ ) {
				MPI_Recv(
					self->forceTmpArray[rank_I],
					self->nCount[rank_I],
					MPI_UNSIGNED,
					rank_I,
					1027,
					self->communicator,
					&s );
			}
		}
		else {
			MPI_Send( &meshExt->nodeLocalCount, 1, MPI_UNSIGNED, 0, 1026, self->communicator );
			MPI_Send( self->mesh->nodeL2G, meshExt->nodeLocalCount, MPI_UNSIGNED, 0, 1027, self->communicator );
		}
	}

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

	/* If storing forces on nodes, build it */
	if( self->storeForces ) {
		Snac_Mesh* meshExt = ExtensionManager_Get( self->meshExtensionMgr, self->mesh, self->storeForcesMeshHandle );
		Snac_Mesh_IC( meshExt );
	}

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

	Journal_Printf( self->snacInfo, "For Material %d:\n",phaseI );

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
	FILE* fp;
	char fname[PATH_MAX];

	/* if restarting, reset the time step counter */
	if( self->restartStep > 0 )
		self->timeStep = self->restartStep;

	_Snac_Context_InitDump( self );


	if( self->rank == 0 ) Journal_Printf( self->snacInfo, "In: %s\n", __func__ );
	if( self->rank == 0 ) Journal_Printf(
		self->info,
		"self->timeStep: %u (update elements only)\nself->currentTime: %g\n",
		self->timeStep,
		self->currentTime );

	/* Write out timeStep info */
	DumpLoopInfo( self );

	if( self->restartStep == 0 ) {
		for( element_lI = 0; element_lI < self->mesh->elementLocalCount && self->restartStep == 0; element_lI++ ) {
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
					element->tetraStress[tetra_I][1][1] * element->tetraStress[tetra_I][2][2] +
					element->tetraStress[tetra_I][2][2] * element->tetraStress[tetra_I][0][0] +
					element->tetraStress[tetra_I][0][0] * element->tetraStress[tetra_I][1][1];
				element->tetra[tetra_I].avgTemp = 0.0f;
				element->tetra[tetra_I].density = phsDensity;
			}
			element->stress = 0.5f * sqrt( 0.5f * fabs( sVolAvg/Tetrahedra_Count ) );
		}

		sprintf( fname, "%s/pisos.%d.restart", self->outputPath, self->rank );
		Journal_Firewall( (fp = fopen( fname, "w" )), "Failed to open %s!!\n", fname );
		for( element_lI = 0; element_lI < self->mesh->elementLocalCount && self->restartStep == 0; element_lI++ ) {
			Snac_Element* element = Snac_Element_At( self, element_lI );
			fprintf(fp,"%e\n",element->pisos);
		}
		fclose( fp );
	}

	Journal_Dump( self->strainRateOut, &(self->timeStep) );

	Journal_Dump( self->stressOut, &(self->timeStep) );

	Journal_Dump( self->hydroPressureOut, &(self->timeStep) );

	_Snac_Context_DumpStressTensor( self );


	/* Update all the elements, and in the process work out this processor's minLengthScale */
	KeyCall( self, self->loopElementsMomentumK, EntryPoint_VoidPtr_CallCast* )( KeyHandle(self,self->loopElementsMomentumK), self );

	Journal_Dump( self->coordOut, &(self->timeStep) );

	Journal_Dump( self->velOut, &(self->timeStep) );

	if ( self->storeForces )
	{
		Journal_Dump( self->forceOut, &(self->timeStep) );
	}

	_Snac_Context_DumpPhaseIndex( self );

	KeyCall( self, self->syncK, EntryPoint_VoidPtr_CallCast* )( KeyHandle(self,self->syncK), self );
}


double _Snac_Context_Dt( void* context ) {
	Snac_Context*	self = (Snac_Context*)context;

	if( self->rank == 0 ) Journal_Printf( self->debug, "In: %s\n", __func__ );

	/* if restarting, reset the time step counter */
	if( self->restartStep > 0 && self->timeStep <= 1)
		self->timeStep = self->restartStep+1;

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

	if( self->timeStep == 0 || ((self->timeStep-1) % self->dumpEvery == 0) ) {
		DumpLoopInfo( self );
	}

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

	Journal_Dump( self->strainRateOut, &(self->timeStep) );
	Journal_Dump( self->stressOut, &(self->timeStep) );
        Journal_Dump( self->hydroPressureOut, &(self->timeStep) );
	_Snac_Context_DumpStressTensor( self );

	KeyCall( self, self->loopNodesMomentumK, EntryPoint_VoidPtr_CallCast* )( KeyHandle(self,self->loopNodesMomentumK), self );

	Journal_Dump( self->coordOut, &(self->timeStep) );
	Journal_Dump( self->velOut, &(self->timeStep) );
	if ( self->storeForces )
	{
		Journal_Dump( self->forceOut, &(self->timeStep) );
	}

	KeyCall( self, self->loopElementsMomentumK, EntryPoint_VoidPtr_CallCast* )( KeyHandle( self, self->loopElementsMomentumK ), self );

	/* Synchronise again, but really need to consolidate with the other one... */
	if( self->forceCalcType == Snac_Force_Complete &&
	    self->mesh->layout->decomp->procsInUse > 1 )
	{
		Mesh_Sync( self->mesh );
	}

	_Snac_Context_DumpPhaseIndex( self );
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


void _Snac_Context_LoopBoundaryNodesPre( void* context ) {
	Snac_Context* 		self = (Snac_Context*)context;
	Snac_Parallel*		parallel = self->parallel;
	Partition_Index		rn_I;

	if( self->rank == 0 ) Journal_DPrintf( self->debug, "In: %s\n", __func__ );
	if( self->rank == 0 ) Journal_Printf(
		self->verbose,
		"For each boundary node, calc & send the mass & force (partial calc)\n" );

	/* For each boundary node, calculate the mass and force, and send to neighbour (will only be a partial calculation) */
	for( rn_I = 0; rn_I < parallel->rnCount; rn_I++ ) {
		IndexSet_Index		index;

		for( index = 0; index < parallel->boundaryCount[rn_I]; index++ ) {
			Mass			mass;
			Force			balance;

			parallel->boundaryIMass[rn_I][index] = 0.0f;
			KeyCall( self, self->forceK, Snac_Force_CallCast* )(
				KeyHandle(self,self->forceK),
				self,
				parallel->boundaryArray[rn_I][index],
				self->speedOfSound,
				&mass,
				&parallel->boundaryIMass[rn_I][index],
				&parallel->boundaryForce[rn_I][index],
				&balance );
		}

		MPI_Send(
			parallel->boundaryForce[rn_I],
			3 * parallel->boundaryCount[rn_I],
			MPI_DOUBLE,
			parallel->rn[rn_I],
			1024,
			parallel->communicator );
		MPI_Send(
			parallel->boundaryIMass[rn_I],
			parallel->boundaryCount[rn_I],
			MPI_DOUBLE,
			parallel->rn[rn_I],
			1025,
			parallel->communicator );
	}
}

void _Snac_Context_LoopBoundaryNodesPost( void* context ) {
	Snac_Context* 		self = (Snac_Context*)context;
	Snac_Mesh*		meshExt = self->storeForces ?
					ExtensionManager_Get( self->meshExtensionMgr, self->mesh, self->storeForcesMeshHandle ) : 0;
	Snac_Parallel*		parallel = self->parallel;
	Partition_Index		rn_I;

	if( self->rank == 0 ) Journal_DPrintf( self->debug, "In: %s\n", __func__ );
	if( self->rank == 0 ) Journal_Printf(
		self->verbose,
		"For each boundary node, recv partial mass & force, and calc vel & coord.\n" );

	/* For each boundary node, recieve from nieghbour, complete mass and force calculation. */
	for( rn_I = 0; rn_I < parallel->rnCount; rn_I++ ) {
		IndexSet_Index		index;
		MPI_Status		status;

		MPI_Recv(
			parallel->boundaryForceRemote[rn_I],
			3 * parallel->boundaryCount[rn_I],
			MPI_DOUBLE,
			parallel->rn[rn_I],
			1024,
			parallel->communicator,
			&status );
		MPI_Recv(
			parallel->boundaryIMassRemote[rn_I],
			parallel->boundaryCount[rn_I],
			MPI_DOUBLE,
			parallel->rn[rn_I],
			1025,
			parallel->communicator,
			&status );

		for( index = 0; index < parallel->boundaryCount[rn_I]; index++ ) {
			parallel->boundaryForce[rn_I][index][0] += parallel->boundaryForceRemote[rn_I][index][0];
			parallel->boundaryForce[rn_I][index][1] += parallel->boundaryForceRemote[rn_I][index][1];
			parallel->boundaryForce[rn_I][index][2] += parallel->boundaryForceRemote[rn_I][index][2];
			parallel->boundaryIMass[rn_I][index]    += parallel->boundaryIMassRemote[rn_I][index];

			KeyCall( self, self->updateNodeK, Snac_UpdateNodeMomentum_CallCast* )(
				KeyHandle( self,self->updateNodeK ),
				self,
				parallel->boundaryArray[rn_I][index],
				parallel->boundaryIMass[rn_I][index],
				parallel->boundaryForce[rn_I][index] );

			/* Store forces on nodes? */
			if( self->storeForces ) {
				meshExt->force[parallel->boundaryArray[rn_I][index]][0] = parallel->boundaryForce[rn_I][index][0];
				meshExt->force[parallel->boundaryArray[rn_I][index]][1] = parallel->boundaryForce[rn_I][index][1];
				meshExt->force[parallel->boundaryArray[rn_I][index]][2] = parallel->boundaryForce[rn_I][index][2];
			}
		}
	}
}

void _Snac_Context_LoopNodes( void* context ) {
	Snac_Context* 		self = (Snac_Context*)context;
	Node_LocalIndex		node_lI;
	Snac_Mesh*		meshExt = self->storeForces ?
					ExtensionManager_Get( self->meshExtensionMgr, self->mesh, self->storeForcesMeshHandle ) : 0;
	Snac_Parallel*		parallel = self->parallel;

	if( self->rank == 0 ) Journal_DPrintf( self->debug, "In: %s\n", __func__ );
	if( self->rank == 0 ) Journal_Printf(
		self->verbose,
		"For each node, calculate mass, force, velocity and then coordinates\n" );

	/* For each node, calculate the mass, force, veloity and coordinates */
	if( self->forceCalcType != Snac_Force_Complete ) {
		_Snac_Context_LoopBoundaryNodesPre( self );
	}

	for( node_lI = 0; node_lI < self->mesh->nodeLocalCount; node_lI++ ) {
		Partition_Index		rn_I;
		Bool			notForceBC;
		Snac_Node*		node = Snac_Node_At( self, node_lI );

		notForceBC = True;
		if( self->forceCalcType != Snac_Force_Complete ) {
			for( rn_I = 0; rn_I < parallel->rnCount; rn_I++ ) {
				if( parallel->boundarySet[rn_I] && IndexSet_IsMember( parallel->boundarySet[rn_I], node_lI ) ) {
					notForceBC = False;
				}
			}
		}

		if( notForceBC ) {
			Mass			mass;
			Force			balance;
			Force*			forcePtr;

			forcePtr = &meshExt->force[node_lI];

			node->inertialMass=0.0;
			KeyCall( self, self->forceK, Snac_Force_CallCast* )(
				KeyHandle(self,self->forceK),
				self,
				node_lI,
				self->speedOfSound,
				&mass,
				&(node->inertialMass),
				forcePtr,
				&balance );
		}
	}
	for( node_lI = 0; node_lI < self->mesh->nodeLocalCount; node_lI++ ) {
		Partition_Index		rn_I;
		Bool			notForceBC;
		Snac_Node*		node = Snac_Node_At( self, node_lI );

		notForceBC = True;
		if( self->forceCalcType != Snac_Force_Complete ) {
			for( rn_I = 0; rn_I < parallel->rnCount; rn_I++ ) {
				if( parallel->boundarySet[rn_I] && IndexSet_IsMember( parallel->boundarySet[rn_I], node_lI ) ) {
					notForceBC = False;
				}
			}
		}

		if( notForceBC ) {
			Force*			forcePtr;

			forcePtr = &meshExt->force[node_lI];

			KeyCall( self, self->updateNodeK, Snac_UpdateNodeMomentum_CallCast* )(
				KeyHandle(self,self->updateNodeK),
				self,
				node_lI,
				node->inertialMass,
				*forcePtr );
		}
	}

	if( self->forceCalcType != Snac_Force_Complete ) {
		_Snac_Context_LoopBoundaryNodesPost( self );
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
	if(self->timeStep==self->restartStep && self->restartStep==0) {
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

	if( self->doForceCheckSum ) {
		/* Recieve all the forces from all the processors */
		if( self->rank == 0 ) {
			PartitionIndex		rank_I;
			MPI_Status		s;

			/* Split loops to have all counts first */
			for( rank_I = 1; rank_I < self->nproc; rank_I++ ) {
				MPI_Recv(
					self->forceTmp[rank_I],
					self->nCount[rank_I] * 3,
					MPI_DOUBLE,
					rank_I,
					1028,
					self->communicator,
					&s );
			}
		}
		else {
			Snac_Mesh*		meshExt = ExtensionManager_Get(
							self->meshExtensionMgr,
							self->mesh,
							self->storeForcesMeshHandle );

			MPI_Send( meshExt->force, meshExt->nodeLocalCount * 3, MPI_DOUBLE, 0, 1028, self->communicator );
		}

		/* Build the global force array from the buffer */
		if( self->rank == 0 ) {
			PartitionIndex		rank_I;
			#if 0
				IndexSet* is = IndexSet_New( self->mesh->nodeGlobalCount );
			#endif

			memset( self->force, 0, sizeof(Force) * self->mesh->nodeGlobalCount );
			for( rank_I = 0; rank_I < self->nproc; rank_I++ ) {
				Index			index;

				for( index = 0; index < self->nCount[rank_I]; index++ ) {
					Node_GlobalIndex	node_gI = self->forceTmpArray[rank_I][index];

					if( node_gI < self->mesh->nodeGlobalCount ) {
						self->force[node_gI][0] = self->forceTmp[rank_I][index][0];
						self->force[node_gI][1] = self->forceTmp[rank_I][index][1];
						self->force[node_gI][2] = self->forceTmp[rank_I][index][2];
						#if 0
							IndexSet_Add( is, node_gI );
						#endif
					}
				}
			}
			#if 0
			{
				int i;
				for( i = 0; i < self->mesh->nodeGlobalCount; i++ )
					Journal_Firewall(
						IndexSet_IsMember( is, i ),
						self->snacError,
						"Not all the nodes are accounted for" );
			}
			#endif
		}

		/* Calculate force check sum... it is important to always add the elements of this number in order... hence the
		    elaborate messaging to bring all the values to the one rank */
		if( self->rank == 0 ) {
			Node_GlobalIndex	node_gI;

			self->forceCheckSum[0] = 0.0f;
			self->forceCheckSum[1] = 0.0f;
			self->forceCheckSum[2] = 0.0f;

			for( node_gI = 0; node_gI < self->mesh->nodeGlobalCount; node_gI++ ) {
				self->forceCheckSum[0] += self->force[node_gI][0];
				self->forceCheckSum[1] += self->force[node_gI][1];
				self->forceCheckSum[2] += self->force[node_gI][2];
			}
		}

		MPI_Bcast( self->forceCheckSum, 3, MPI_DOUBLE, 0, self->communicator );
		DumpForceCheckSum( self );
	}

	fflush( self->timeStepInfo );
	Stream_Flush( self->strainRateOut );
	Stream_Flush( self->stressOut );
	Stream_Flush( self->hydroPressureOut );
	Stream_Flush( self->coordOut );
	Stream_Flush( self->velOut );
	Stream_Flush( self->forceOut );
	fflush( self->forceCheckSumOut );
}


void DumpLoopInfo( void* context ) {
	Snac_Context* self = (Snac_Context*)context;

	fprintf( self->timeStepInfo, "%16u %16g %16g\n", self->timeStep, self->currentTime, self->dt );
}

void DumpForceCheckSum( Snac_Context* self ) {
	if( sizeof(long unsigned int) == sizeof(double) && sizeof(double) == 8 ) {
		fprintf(
			self->forceCheckSumOut,
			"%05u: %016lx %016lx %016lx %16g %16g %16g\n",
			self->timeStep,
			(long unsigned int)self->forceCheckSum[0],
			(long unsigned int)self->forceCheckSum[1],
			(long unsigned int)self->forceCheckSum[2],
			self->forceCheckSum[0],
			self->forceCheckSum[1],
			self->forceCheckSum[2] );
	}
	else if( 2 * sizeof(long unsigned int) == sizeof(double) && sizeof(double) == 8 ) {
		long unsigned int* fcs0 = (long unsigned int*)((char*)&self->forceCheckSum[0]);
		long unsigned int* fcs1 = (long unsigned int*)((char*)&self->forceCheckSum[1]);
		long unsigned int* fcs2 = (long unsigned int*)((char*)&self->forceCheckSum[2]);

		fprintf(
			self->forceCheckSumOut,
			"%05u: %08lx%08lx %08lx%08lx %08lx%08lx %16g %16g %16g\n",
			self->timeStep,
			fcs0[0],
			fcs0[1],
			fcs1[0],
			fcs1[1],
			fcs2[0],
			fcs2[1],
			self->forceCheckSum[0],
			self->forceCheckSum[1],
			self->forceCheckSum[2] );
	}
	Journal_DFirewall(
		(sizeof(long unsigned int) == sizeof(double) && sizeof(double) == 8) ||
			(2 * sizeof(long unsigned int) == sizeof(double) && sizeof(double) == 8),
		self->snacError,
		"Size of double is not what we expected" );
}

void _Snac_Context_InitDump( Snac_Context* self ) {
	Snac_Mesh* tmpMesh;
	char tmpBuf[200];

	/* Create the strain rate dumping stream */
	self->strainRateOut = Journal_Register( VariableDumpStream_Type, "StrainRate" );
	sprintf( tmpBuf, "%s/strainRate.%u", self->outputPath, self->rank );
	VariableDumpStream_SetVariable(
		self->strainRateOut,
		Variable_Register_GetByName( self->variable_Register, "strainRate" ),
		self->mesh->elementLocalCount,
		self->dumpEvery,
		tmpBuf );

	/* Create the stress dumping stream */
	self->stressOut = Journal_Register( VariableDumpStream_Type, "Stress" );
	sprintf( tmpBuf, "%s/stress.%u", self->outputPath, self->rank );
	VariableDumpStream_SetVariable(
		self->stressOut,
		Variable_Register_GetByName( self->variable_Register, "stress" ),
		self->mesh->elementLocalCount,
		self->dumpEvery,
		tmpBuf );

        /* Create the pressure  dumping stream */
        self->hydroPressureOut = Journal_Register( VariableDumpStream_Type, "HydroPressure" );
        sprintf( tmpBuf, "%s/hydroPressure.%u", self->outputPath, self->rank );
        VariableDumpStream_SetVariable(
                self->hydroPressureOut,
                Variable_Register_GetByName( self->variable_Register, "hydroPressure" ),
                self->mesh->elementLocalCount,
                self->dumpEvery,
                tmpBuf );

	/* Create the coords dumping stream */
	self->coordOut = Journal_Register( VariableDumpStream_Type, "Coord" );
	sprintf( tmpBuf, "%s/coord.%u", self->outputPath, self->rank );
	VariableDumpStream_SetVariable(
		self->coordOut,
		Variable_Register_GetByName( self->variable_Register, "coord" ),
		self->mesh->nodeLocalCount,
		self->dumpEvery,
		tmpBuf );

	/* Create the velocity dumping stream */
	self->velOut = Journal_Register( VariableDumpStream_Type, "Velocity" );
	sprintf( tmpBuf, "%s/vel.%u", self->outputPath, self->rank );
	VariableDumpStream_SetVariable(
		self->velOut,
		Variable_Register_GetByName( self->variable_Register, "velocity" ),
		self->mesh->nodeLocalCount,
		self->dumpEvery,
		tmpBuf );


	/* Create the force StG variable... delayed until here because the extension doesn't exist at the point that the other
	 * variables are created. TODO: move force BACK into node structure. Force build. */
	tmpMesh = ExtensionManager_Get( self->meshExtensionMgr, self->mesh, self->storeForcesMeshHandle );
	Variable_NewVector(
		"force",
		Variable_DataType_Double,
		3,
		&self->mesh->layout->decomp->nodeLocalCount,
		(void**)&(tmpMesh->force),
		self->variable_Register,
		"fx",
		"fy",
		"fz" );
	Build( Variable_Register_GetByName( self->variable_Register, "force" ), NULL, False );
	Build( Variable_Register_GetByName( self->variable_Register, "fx" ), NULL, False );
	Build( Variable_Register_GetByName( self->variable_Register, "fy" ), NULL, False );
	Build( Variable_Register_GetByName( self->variable_Register, "fz" ), NULL, False );

	/* Create the force dumping stream */
	self->forceOut = Journal_Register( VariableDumpStream_Type, "Forces" );
	sprintf( tmpBuf, "%s/force.%u", self->outputPath, self->rank );
	VariableDumpStream_SetVariable(
		self->forceOut,
		Variable_Register_GetByName( self->variable_Register, "force" ),
		self->mesh->nodeLocalCount,
		self->dumpEvery,
		tmpBuf );

	/* Create the stressTensor dump file */
	sprintf( tmpBuf, "%s/stressTensor.%u", self->outputPath, self->rank );
	if( (self->stressTensorOut = fopen( tmpBuf, "w+" )) == NULL ) {
		assert( self->stressTensorOut /* failed to open file for writing */ );
	}
	/* Create the phaseIndex dump file */
	sprintf( tmpBuf, "%s/phaseIndex.%u", self->outputPath, self->rank );
	if( (self->phaseIndexOut = fopen( tmpBuf, "w+" )) == NULL ) {
		assert( self->phaseIndexOut /* failed to open file for writing */ );
	}
}

void _Snac_Context_DumpStressTensor( Snac_Context* self ) {

	if( self->timeStep ==0 || (self->timeStep-1) % self->dumpEvery == 0 ) {
		Element_LocalIndex			element_lI;

		for( element_lI = 0; element_lI < self->mesh->elementLocalCount; element_lI++ ) {
			Snac_Element* 				element = Snac_Element_At( self, element_lI );
			/* Take average of tetra viscosity for the element */
			Tetrahedra_Index		tetra_I;
			for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
				float tensor[3][3] = { {element->tetraStress[tetra_I][0][0], element->tetraStress[tetra_I][0][1], element->tetraStress[tetra_I][0][2]},
									   {element->tetraStress[tetra_I][0][1], element->tetraStress[tetra_I][1][1], element->tetraStress[tetra_I][1][2]},
									   {element->tetraStress[tetra_I][0][2], element->tetraStress[tetra_I][1][2], element->tetraStress[tetra_I][2][2]} };
				fwrite( &tensor, sizeof(float), 9, self->stressTensorOut );
			}
		}
	}
}

void _Snac_Context_DumpPhaseIndex( Snac_Context* self ) {

	if( self->timeStep ==0 || (self->timeStep-1) % self->dumpEvery == 0 ) {
		Element_LocalIndex			element_lI;

		for( element_lI = 0; element_lI < self->mesh->elementLocalCount; element_lI++ ) {
			Snac_Element* 				element = Snac_Element_At( self, element_lI );
			fwrite( &element->material_I, sizeof(unsigned int), 1, self->phaseIndexOut );
		}
	}
}
