#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgDomain/StgDomain.h>

int main( int argc, char** argv ) {
	MPI_Init( &argc, &argv );
	StGermain_Init( &argc, &argv );
	StgDomain_Init( &argc, &argv );

	StgDomain_Finalise();
	StGermain_Finalise();
	MPI_Finalize();
	return 0;
}
