#ifdef HAVE_PYTHON
#include <Python.h>
#endif

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"

#include <assert.h>

int main( int argc, char* argv[] ) {
	MPI_Comm               CommWorld;
	int                    rank;
	int                    numProcessors;
	Dictionary*            dictionary;
	Dictionary*            componentDict;
	Stg_ComponentFactory*  cf;
	XML_IO_Handler*        ioHandler;
	int	                   procToWatch;

	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );

	#ifdef HAVE_PYTHON
	Py_Initialize();
	#endif
	
	Base_Init( &argc, &argv );
	
	DiscretisationGeometry_Init( &argc, &argv );
	DiscretisationShape_Init( &argc, &argv );

	MPI_Barrier( CommWorld ); /* Ensures copyright info always come first in output */

	dictionary = Dictionary_New();
	ioHandler = XML_IO_Handler_New();
	IO_Handler_ReadAllFromCommandLine( ioHandler, argc, argv, dictionary );

	componentDict = Dictionary_GetDictionary( dictionary, "components" );
	assert( componentDict );
	cf = Stg_ComponentFactory_New( dictionary, componentDict, Stg_ObjectList_New() );
	Stg_ComponentFactory_CreateComponents( cf );
	Stg_ComponentFactory_ConstructComponents( cf, 0 /* dummy */ );

	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	if( rank == procToWatch ) printf( "Watching rank: %i\n", rank );

	if( rank == procToWatch ) {
		Stg_Shape*       shape;
		Index            index;
		Name             outputPath;
		Dimension_Index  dim;
		Index            testCoordCount = Dictionary_GetUnsignedInt_WithDefault( dictionary, "testCoordCount", 10000 );
		Coord            coord;
		Stream*          stream         =  Journal_Register( Info_Type, Stg_Shape_Type );

		shape   = (Stg_Shape*) LiveComponentRegister_Get( cf->LCRegister, "shape" );
		assert( shape );

		dim = Dictionary_GetUnsignedInt( dictionary, "dim" );
		outputPath = Dictionary_GetString( dictionary, "outputPath" );
		Stream_RedirectFile_WithPrependedPath( stream, outputPath, "output.dat" );

		/* Test to see if random points are in shape */
		srand48(0);
		for (index = 0 ; index < testCoordCount ; index++ ) {
			coord[ I_AXIS ] = drand48() - 1.0;
			coord[ J_AXIS ] = drand48() - 1.0;
			if ( dim == 3 ) 
				coord[ K_AXIS ] = drand48() - 1.0;

			if ( Stg_Shape_IsCoordInside( shape, coord ) ) 
				Journal_Printf( stream, "%u\n", index );
		}
	}

	/* Destroy stuff */
	Stg_Class_Delete( cf->registerRegister );
	Stg_Class_Delete( cf );
	Stg_Class_Delete( dictionary );
	
	DiscretisationShape_Finalise();
	DiscretisationGeometry_Finalise();
	Base_Finalise();
	
	#ifdef HAVE_PYTHON
	Py_Finalize();
	#endif
	/* Close off MPI */
	MPI_Finalize();

	return EXIT_SUCCESS;
}
