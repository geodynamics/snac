#include <petsc.h>

int main( int argc, char** argv ) {

	PetscInitialize( &argc, &argv,(char *)0, NULL );
	PetscFinalize();
	
	return 0;
}
