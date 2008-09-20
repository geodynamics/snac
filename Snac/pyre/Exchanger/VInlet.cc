// -*- C++ -*-
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  <LicenseText>
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

#include "journal/debug.h"
#include "Exchanger/BoundedMesh.h"
#include "Exchanger/Sink.h"
#include "VInlet.h"
extern "C" {
	#include <mpi.h>
	#include <StGermain/StGermain.h>
	#include <StGermain/FD/FD.h>
	#include <Snac/Snac.h>

	/* Required module */
	#include "Snac/Exchanger/Exchanger.h"
}

using Exchanger::Array2D;
using Exchanger::BoundedMesh;
using Exchanger::DIM;
using Exchanger::Sink;


VInlet::VInlet(const BoundedMesh& boundedMesh,
	       const Sink& sink,
	       void* __context) :
    Inlet(boundedMesh, sink),
    v(sink.size()),
    v_old(sink.size()),
    SnacExchangerForceBC_ContextHandle( (unsigned)-1 ),
    _context( __context )
{
    journal::debug_t debug("Snac-Exchanger");
    debug << journal::at(__HERE__) << journal::endl;

    Snac_Context*		context = (Snac_Context*)_context;
    fge_t = cge_t = 0;
    //Print( context->extension );
    SnacExchangerForceBC_ContextHandle = ExtensionManager_GetHandle( context->extensionMgr, "SnacExchangerForceBC" );
    SnacExchanger_ContextHandle = ExtensionManager_GetHandle( context->extensionMgr, "SnacExchanger" );

}


VInlet::~VInlet()
{}


void VInlet::recv()
{
    journal::debug_t debug("Snac-Exchanger");
    debug << journal::at(__HERE__) << journal::endl;

    // store bc from previous timestep
    v.swap(v_old);

    sink.recv(v);

    v.print("Snac-VInlet-V");
}


void VInlet::impose()
{
    imposeV();
}


void VInlet::storeVold()
{
    storeVold_();
}


void VInlet::readVold()
{
    readVold_();
}

// private functions

void VInlet::imposeV()
{
	journal::debug_t debug("Snac-VInlet-imposeV");
	debug << journal::at(__HERE__);

	Snac_Context*			context = (Snac_Context*)_context;
	SnacExchanger_Context*		contextExt = (SnacExchanger_Context*)ExtensionManager_Get( context->extensionMgr, context, SnacExchanger_ContextHandle );
	ExchangerVC*			eVC = contextExt->bcs;

	debug << "sink.size(): " << sink.size() << ", eVC->valuesSize: " << eVC->valuesSize << ", are " << ((unsigned)sink.size() == eVC->valuesSize ? "equal... which is good" : "not equal... need to look into.") << journal::endl;

    double N1, N2;
    getTimeFactors(N1, N2);

    for( int i = 0; i < sink.size(); i++ ) {
	    eVC->values[sink.meshNode(i)].as.typeArray.array[0] = N1 * v_old[0][i] + N2 * v[0][i];
	    eVC->values[sink.meshNode(i)].as.typeArray.array[1] = N1 * v_old[1][i] + N2 * v[1][i];
	    eVC->values[sink.meshNode(i)].as.typeArray.array[2] = N1 * v_old[2][i] + N2 * v[2][i];
// 	    if(i==sink.size()-1)
// 		    fprintf(stderr,"v_old=%e %e %e  V=%e %e %e and New Vel=%e %e %e\n",v_old[0][i],v_old[1][i],v_old[2][i],v[0][i],v[1][i],v[2][i],
// 			    eVC->values[sink.meshNode(i)].as.typeArray.array[0],
// 			    eVC->values[sink.meshNode(i)].as.typeArray.array[1],
// 			    eVC->values[sink.meshNode(i)].as.typeArray.array[2]);

	    debug << "Snac imposing VBC: i = "<<i<<" sink.meshNode = "<<sink.meshNode(i)<<" vbc = "
		  << eVC->values[sink.meshNode(i)].as.typeArray.array[0] << " "
		  << eVC->values[sink.meshNode(i)].as.typeArray.array[1] << " "
		  << eVC->values[sink.meshNode(i)].as.typeArray.array[2] << journal::newline;
    }
    debug << journal::endl;
}

void VInlet::storeVold_()
{
	journal::debug_t debug("Snac-VInlet-storeVold");
	debug << journal::at(__HERE__);

	Snac_Context*		    context = (Snac_Context*)_context;
	HexaMD*                     decomp = (HexaMD*)context->meshLayout->decomp;
	FILE                        *fp;
	char                        fname[255];

	sprintf(fname,"vold%d.temp",decomp->rank);
	fp=fopen(fname,"w");

	for( int i = 0; i < sink.size(); i++ )
		fprintf(fp,"%e %e %e\n",v[0][i],v[1][i],v[2][i]);

	fclose(fp);
}

void VInlet::readVold_()
{
	journal::debug_t debug("Snac-VInlet-readVold");
	debug << journal::at(__HERE__);

	Snac_Context*			context = (Snac_Context*)_context;
	HexaMD*                     decomp = (HexaMD*)context->meshLayout->decomp;
	FILE                            *fp;
	char                        fname[255];

	sprintf(fname,"vold%d.temp",decomp->rank);
	fp=fopen(fname,"r");

	for( int i = 0; i < sink.size(); i++ )
		fscanf(fp,"%lg %lg %lg\n",&(v[0][i]),&(v[1][i]),&(v[2][i]));

	fclose(fp);
}


// version
// $Id: VInlet.cc,v 1.5 2004/05/11 07:55:30 tan2 Exp $

// End of file
