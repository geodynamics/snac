#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <mpi.h>

/* This file was renamed to a C++ file because of this header under PDT-TAU */
extern "C" {
#include <StGermain/StGermain.h>
}

#include <SDL/SDL.h>

#if defined(__APPLE__) && defined(__MACH__)
	#include <OpenGL/gl.h>	// Header File For The OpenGL32 Library
	#include <OpenGL/glu.h>	// Header File For The GLu32 Library
#else
	#include <GL/gl.h>	// Header File For The OpenGL32 Library
	#include <GL/glu.h>	// Header File For The GLu32 Library
#endif

#include "types.h"
#include "GLMesh.h"


/*
** Constants
*/

#define MAX_GL_STRING_LEN 255


/*
** Global variables
*/

GLMesh*			glMesh = NULL;
int			scrWidth;
int			scrHeight;
int			quit = 0;
int			mpiInitialised = 0;
int			stgInitialised = 0;
int			sdlInitialised = 0;
GLuint			fontBase = 0;
int			rank = 0;
Partition_Index		viewRank = 0;
Partition_Index		maxRank = 0;
int			oldMouseX;
int			oldMouseY;
GLdouble		xRot = 0.0;
GLdouble		yRot = 0.0;
int			motionOn = 0;


/*
** Function prototypes
*/

void		terminate( void );
void		buildStGermain( int argc, char* argv[] );
/*void		glPrintf( char *fmt, ... );*/
void		initGL( void );
void		display( void );
void		reshape( int width, int height );
void		mouseClick( int button, int state, int x, int y );
void		mouseMotion( int x, int y );
void		eventHandler( void );


int main( int argc, char* argv[] ) {
	/* Register termination function */
	atexit( terminate );
	
	/* Initialise MPI */
	MPI_Init( &argc, &argv );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	mpiInitialised = 1;

	/* Build StGermain constructs */
	Base_Init( &argc, &argv );
	DiscretisationGeometry_Init( &argc, &argv );
	DiscretisationMesh_Init( &argc, &argv );
	buildStGermain( argc, argv );
	stgInitialised = 1;
	
	if( rank == 0 ) {
		/* Root process, handles visualisation */

		/* Initialize SDL for video output */
		if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
			fprintf( stderr, "Unable to initialize SDL: %s\n", SDL_GetError() );
			exit( 1 );
		}
		else {
			sdlInitialised = 1;
		}

		/* Create a 640x480 OpenGL screen */
		if( SDL_SetVideoMode( 640, 480, 0, SDL_OPENGL ) == NULL ) {
			fprintf( stderr, "Unable to create OpenGL screen: %s\n", SDL_GetError() );
			exit( 2 );
		}
		
		/* Initialise OpenGL */
		initGL();
		reshape( 640, 480 );

		/* Enter main loop */
		while( !quit ) {
			eventHandler();
			display();		
		}
	}
	
	return EXIT_SUCCESS;
}


void terminate( void ) {
	if( glMesh ) {
		Stg_Class_Delete( glMesh );
	}

	if( fontBase ) {
		glDeleteLists( fontBase, 127 );
	}
	
	if( sdlInitialised ) {
		SDL_Quit();
	}
	
	if( stgInitialised ) {
		DiscretisationMesh_Finalise();
		DiscretisationGeometry_Finalise();
		Base_Finalise();
	}
	
	if( mpiInitialised ) {
		MPI_Finalize();
	}
}


void buildStGermain( int argc, char* argv[] ) {
	Dictionary*		dictionary;
	Topology*		nTopology;
	ElementLayout*		eLayout;
	NodeLayout*		nLayout;
	MeshDecomp*		decomp;
	MeshLayout*		meshLayout;
	XML_IO_Handler*		ioHandler;
	char*			filename;
	Dimension_Index		numPartitionedDims;
	
	dictionary = Dictionary_New();
	
	ioHandler = XML_IO_Handler_New();
	if( argc >= 2 ) {
		filename = strdup( argv[1] );
	}
	else {
		filename = strdup( "input.xml" );
	}
	
	if( IO_Handler_ReadAllFromFile( ioHandler, filename, dictionary ) == False ) {
		printf( "Failed to open input XML file.\n" );
		exit( EXIT_FAILURE );
	}
	
	nTopology = (Topology*)IJK6Topology_New( "Topology", dictionary );
	eLayout = (ElementLayout*)ParallelPipedHexaEL_New( "elementLayout", 3, dictionary );
	nLayout = (NodeLayout*)CornerNL_New( "nodeLayout", dictionary, eLayout, nTopology );
	numPartitionedDims = Dictionary_GetUnsignedInt_WithDefault( dictionary, "decompDims", 2 );
	/* Make sure "StoreAll" is set so proc 0 can get all info */
	decomp = (MeshDecomp*)HexaMD_New_All( "decomp", dictionary, MPI_COMM_WORLD, eLayout, nLayout, numPartitionedDims );
	meshLayout = MeshLayout_New( "meshLayout", eLayout, nLayout, decomp );
	
	maxRank = decomp->procsInUse;
	
	if( rank == 0 ) {
		glMesh = GLMesh_New();
		GLMesh_BuildFromMesh( glMesh, meshLayout );
	}
	
	Stg_Class_Delete( meshLayout );
	Stg_Class_Delete( decomp );
	Stg_Class_Delete( nLayout );
	Stg_Class_Delete( eLayout );
	Stg_Class_Delete( nTopology );
	Stg_Class_Delete( dictionary );
}


/*void glPrintf( char *fmt, ... ) {
	va_list		args;
	char		str[MAX_GL_STRING_LEN];
	
	va_start( args, fmt );
	vsprintf( str, fmt, args );
	va_end( args );

	glListBase( fontBase );
	glCallLists( strlen( str ), GL_UNSIGNED_BYTE, str );
}*/


void initGL( void ) {
	/*GLfloat		pda[3] = { 0.0, 0.3, 0.0 };*/

	/* Set the background color */
	glClearColor( 0.3, 0.3, 0.3, 1.0 );
	
	/* Enable back-face culling */
	glEnable( GL_CULL_FACE );
	
	/* Set the shading model to smooth */
	glShadeModel( GL_SMOOTH );
	
	/* Enable depth testing */
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LEQUAL );
	
	glFrontFace( GL_CCW );
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	
	/* Set the initial point thickness */
	glPointSize( 5.0 );
	/*glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, pda);*/
	
	/* Build the font lists for text rendering */
	/*fontBase = glGenLists( 127 );
	for( glyph_I = 0; glyph_I < 127; glyph_I++ ) {
		glNewList( fontBase + i, GL_COMPILE );
		glutBitmapCharacter( GLUT_BITMAP_HELVETICA_12, i );
		glEndList();
	}*/
	
	glFlush();
}


void display( void ) {
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glTranslated( 0.0, 0.0, -3.0 );
	glRotated( xRot, 0.0, 1.0, 0.0 );
	glRotated( yRot, 1.0, 0.0, 0.0 );
	
	if( glMesh ) {
		GLMesh_RenderRank( glMesh, viewRank );
	}
	
	SDL_GL_SwapBuffers();
}


void reshape( int width, int height ) {
	scrWidth = width;
	scrHeight = height ? height : 1;

	glViewport( 0, 0, scrWidth, scrHeight );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0, (GLdouble)scrWidth / (GLdouble)scrHeight, 0.5, 30.0 );
}


void mouseClick( int button, int state, int x, int y ) {
	if( button == SDL_BUTTON_LEFT ) {
		if( state == SDL_PRESSED ) {
			oldMouseX = x;
			oldMouseY = y;
			motionOn = 1;
		}
		else if ( state == SDL_RELEASED ) {
			motionOn = 0;
		}
	}
}


void mouseMotion( int x, int y ) {
	if( motionOn ) {
		int	dx = x - oldMouseX;
		int	dy = y - oldMouseY;
		
		xRot += (GLdouble)dx * 0.3;
		yRot += (GLdouble)dy * 0.3;
		
		oldMouseX = x;
		oldMouseY = y;
	}
}


void eventHandler( void ) {
	SDL_Event	event;
	
	if( SDL_PollEvent( &event ) ) {
		switch( event.type ) {
			case SDL_QUIT:
				quit = 1;
				break;
			
			case SDL_VIDEORESIZE:
				reshape( event.resize.w, event.resize.h );
				break;
			
			case SDL_KEYDOWN:
				switch( event.key.keysym.sym ) {
					case SDLK_ESCAPE:
						quit = 1;
						break;
					
					case SDLK_c:
						xRot = 0.0;
						yRot = 0.0;
						break;
					
					case SDLK_r:
						viewRank = (viewRank + 1) % maxRank;
						break;

					default:
						break;
				}
				break;

			case SDL_MOUSEMOTION:
				mouseMotion( event.motion.x, event.motion.y );
				break;
			
			case SDL_MOUSEBUTTONDOWN:
				mouseClick( event.button.button, event.button.state, event.button.x, event.button.y );
				break;
			
			case SDL_MOUSEBUTTONUP:
				mouseClick( event.button.button, event.button.state, event.button.x, event.button.y );
				break;
			
			default:
				break;
		}
	}
}
