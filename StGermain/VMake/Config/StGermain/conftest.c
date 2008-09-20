#include <mpi.h>
#include <StGermain/StGermain.h>

int main( int argc, char** argv ) {
	MPI_Init( &argc, &argv );
	StGermain_Init( &argc, &argv );

	StGermain_Finalise();
	MPI_Finalize();
	return 0;
}
