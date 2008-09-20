#include <stdlib.h>
#include <math.h>

#include <mpi.h>

/* This file was renamed to a C++ file because of this header under PDT-TAU */
extern "C" {
#include <StGermain/StGermain.h>
}

#if defined(__APPLE__) && defined(__MACH__)
	#include <OpenGL/gl.h>	// Header File For The OpenGL32 Library
	#include <OpenGL/glu.h>	// Header File For The GLu32 Library
#else
	#include <GL/gl.h>	// Header File For The OpenGL32 Library
	#include <GL/glu.h>	// Header File For The GLu32 Library
#endif
#include <SDL/SDL.h>	

#define WIDTH 600
#define HEIGHT 400
unsigned int quit = 0;

typedef void    (BuildFunction)		( PartitionIndex proc );


void buildLocalEdges( Partition_Index proc );


/* StGermain globals */
Dictionary*     dictionary;
MeshLayout*     meshLayout;

IndexSet*       litEdges = NULL;
IndexSet*       litNodes = NULL;
BuildFunction*  build = buildLocalEdges;
Partition_Index viewProc = 0;

Coord		origin;
Coord		orient = { 0.0, 0.0, 0.0 };
Coord		bboxMin;
Coord		bboxMax;

Bool		rotate = False;
GLdouble	oldx, oldy;
GLdouble	xrot = 0.0, yrot = 0.0;


void display( void ) {
	Index			edge_I;
	Index			indexCnt;
	Index*			indices;
	Node_Index		node_I;
	
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT  );
	
	glPushMatrix();
	glRotated( orient[0], 0.0, 1.0, 0.0 );
	glRotated( orient[1], 1.0, 0.0, 0.0 );
	
	/* Draw elements */
	IndexSet_GetMembers( litEdges, &indexCnt, &indices );
	if( indexCnt ) {
		glBegin( GL_LINES );
		glColor3d( 0.9, 0.9, 0.9 );
		for( edge_I = 0; edge_I < indexCnt; edge_I++ ) {
			Coord   point;
			Edge    edge;
			
			meshLayout->elementLayout->edgeAt( meshLayout->elementLayout, indices[edge_I], edge );
			meshLayout->elementLayout->geometry->pointAt( meshLayout->elementLayout->geometry, edge[0], point );
			glVertex3d( point[0] + origin[0], point[1] + origin[1], point[2] + origin[2] );
			meshLayout->elementLayout->geometry->pointAt( meshLayout->elementLayout->geometry, edge[1], point );
			glVertex3d( point[0] + origin[0], point[1] + origin[1], point[2] + origin[2] );
		}
		glEnd();
	}
	if( indices )
		Memory_Free( indices );
	
	IndexSet_GetVacancies( litEdges, &indexCnt, &indices );
	if( indexCnt ) {
		glBegin( GL_LINES );
		glColor3d( 0.5, 0.5, 0.5 );
		for( edge_I = 0; edge_I < indexCnt; edge_I++ ) {
			Coord   point;
			Edge    edge;
			
			meshLayout->elementLayout->edgeAt( meshLayout->elementLayout, indices[edge_I], edge );
			meshLayout->elementLayout->geometry->pointAt( meshLayout->elementLayout->geometry, edge[0], point );
			glVertex3d( point[0] + origin[0], point[1] + origin[1], point[2] + origin[2] );
			meshLayout->elementLayout->geometry->pointAt( meshLayout->elementLayout->geometry, edge[1], point );
			glVertex3d( point[0] + origin[0], point[1] + origin[1], point[2] + origin[2] );
		}
		glEnd();
	}
	if( indices )
		Memory_Free( indices );
	
	/* Draw nodes */
	IndexSet_GetMembers( litNodes, &indexCnt, &indices );
	if( indexCnt ) {
		glBegin( GL_POINTS );
		glColor3d( 0.9, 0.9, 0.9 );
		for( node_I = 0; node_I < indexCnt; node_I++ ) {
			Coord   point;
			
			meshLayout->nodeLayout->nodeCoordAt( meshLayout->nodeLayout, indices[node_I], point );
			glVertex3d( point[0] + origin[0], point[1] + origin[1], point[2] + origin[2] );
		}
		glEnd();
	}
	if( indices )
		Memory_Free( indices );
	
	IndexSet_GetVacancies( litNodes, &indexCnt, &indices );
	if( indexCnt ) {
		glBegin( GL_POINTS );
		glColor3d( 0.5, 0.5, 0.5 );
		for( node_I = 0; node_I < indexCnt; node_I++ ) {
			Coord   point;
			
			meshLayout->nodeLayout->nodeCoordAt( meshLayout->nodeLayout, indices[node_I], point );
			glVertex3d( point[0] + origin[0], point[1] + origin[1], point[2] + origin[2] );
		}
		glEnd();
	}
	if( indices )
		Memory_Free( indices );
	
	glPopMatrix();

	SDL_GL_SwapBuffers();
}


void reshape( int width, int height ) {
	double  maxDepth;
	
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	maxDepth = sqrt( bboxMin[0] * bboxMin[0] + bboxMax[2] * bboxMax[2] ) + 1.0;
	gluPerspective( 45.0, (double)width / (double)height, 1.0, 3.0 * maxDepth );
	glMatrixMode( GL_MODELVIEW );
	glViewport( 0, 0, width, height );
	glLoadIdentity();
	glTranslated( 0.0, 0.0, -1.0 * ( maxDepth + 2.0 ) );
	
	display();
}


void buildStGermain( void ) {
	Topology*		nTopology;
	ElementLayout*		eLayout;
	NodeLayout*		nLayout;
	MeshDecomp*		decomp;
	
	dictionary = Dictionary_New();
	Dictionary_Add( dictionary, "meshSizeI", Dictionary_Entry_Value_FromUnsignedInt( 4 ) );
	Dictionary_Add( dictionary, "meshSizeJ", Dictionary_Entry_Value_FromUnsignedInt( 4 ) );
	Dictionary_Add( dictionary, "meshSizeK", Dictionary_Entry_Value_FromUnsignedInt( 4 ) );
	Dictionary_Add( dictionary, "shadowDepth", Dictionary_Entry_Value_FromUnsignedInt( 1 ) );
	Dictionary_Add( dictionary, "selfStorage", Dictionary_Entry_Value_FromString( "storeAll" ) );
	Dictionary_Add( dictionary, "allowUnusedCPUs", Dictionary_Entry_Value_FromBool( True ) );
	
	nTopology = (Topology*)IJK6Topology_New( "topology", dictionary );
	eLayout = (ElementLayout*)ParallelPipedHexaEL_New( "elementLayout", 3, dictionary );
	nLayout = (NodeLayout*)CornerNL_New( "nodeLayout", dictionary, eLayout, nTopology );
	decomp = (MeshDecomp*)HexaMD_New_All( "decomp", dictionary, MPI_COMM_WORLD, eLayout, nLayout, 2 );
	meshLayout = MeshLayout_New( "meshLayout", eLayout, nLayout, decomp );
}


void cleanUp( void ) {
	if( meshLayout ) {
		Stg_Class_Delete( meshLayout->decomp );
		Stg_Class_Delete( meshLayout->nodeLayout->topology );
		Stg_Class_Delete( meshLayout->nodeLayout );
		Stg_Class_Delete( meshLayout->elementLayout );
		Stg_Class_Delete( meshLayout );
	}
	
	if( dictionary )
		Stg_Class_Delete( dictionary );
	
	if( litEdges )
		Stg_Class_Delete( litEdges );
	
}


void calcOrigin( void ) {
	Node_GlobalIndex	gNode_I;
	
	origin[0] = 0.0;
	origin[1] = 0.0;
	origin[2] = 0.0;
	
	for( gNode_I = 0; gNode_I < meshLayout->nodeLayout->nodeCount; gNode_I++ ) {
		Coord   point;
		
		meshLayout->nodeLayout->nodeCoordAt( meshLayout->nodeLayout, gNode_I, point );
		origin[0] -= point[0];
		origin[1] -= point[1];
		origin[2] -= point[2];
	}
	
	origin[0] /= (double)meshLayout->nodeLayout->nodeCount;
	origin[1] /= (double)meshLayout->nodeLayout->nodeCount;
	origin[2] /= (double)meshLayout->nodeLayout->nodeCount;
}


void calcBBox( void ) {
	Node_GlobalIndex	gNode_I;
	
	bboxMin[0] = 0.0;
	bboxMin[1] = 0.0;
	bboxMin[2] = 0.0;
	bboxMax[0] = 0.0;
	bboxMax[1] = 0.0;
	bboxMax[2] = 0.0;
	
	for( gNode_I = 0; gNode_I < meshLayout->nodeLayout->nodeCount; gNode_I++ ) {
		Coord   point;
		
		meshLayout->nodeLayout->nodeCoordAt( meshLayout->nodeLayout, gNode_I, point );
		
		if( point[0] < bboxMin[0] )
			bboxMin[0] = point[0];
		else if( point[0] > bboxMax[0] )
			bboxMax[0] = point[0];
		
		if( point[1] < bboxMin[1] )
			bboxMin[1] = point[1];
		else if( point[1] > bboxMax[1] )
			bboxMax[1] = point[1];
		
		if( point[2] < bboxMin[2] )
			bboxMin[2] = point[2];
		else if( point[2] > bboxMax[2] )
			bboxMax[2] = point[2];
	}
	
	bboxMin[0] += origin[0];
	bboxMax[0] += origin[0];
	bboxMin[1] += origin[1];
	bboxMax[1] += origin[1];
	bboxMin[2] += origin[2];
	bboxMax[2] += origin[2];
}


void mouseClick( int button, int state, int x, int y ) {
	if( button == SDL_BUTTON_LEFT ) {
		
		if( state == SDL_PRESSED ) {
			oldx = (double) x;
			oldy = (double) y;
			rotate = True;
		}
		
		if ( state == SDL_RELEASED ){
			rotate = False;
		}
	}
}


void mouseMotion( int x, int y ) {

	if( rotate ) {
		orient[0] += (double)(x - oldx) / 2.0;
		orient[1] += (double)(y - oldy) / 2.0;
		
		oldx = (double) x;
		oldy = (double) y;

		display();
	}
}


void buildLocalEdges( Partition_Index proc ) {
	Index   indexCnt;
	Index*  indices;
	
	IndexSet_RemoveAll( litEdges );
	IndexSet_GetMembers( meshLayout->decomp->localElementSets[proc], &indexCnt, &indices );
	if( indexCnt ) {
		Index   edgeCnt;
		Index*  edges;
		Index   edge_I;
		
		edgeCnt = ElementLayout_BuildEdgeSubset( meshLayout->elementLayout, indexCnt, indices, &edges );
		for( edge_I = 0; edge_I < edgeCnt; edge_I++ )
			IndexSet_Add( litEdges, edges[edge_I] );
		
		if( edges )
			Memory_Free( edges );
	}
	Memory_Free( indices );
	
	litNodes = meshLayout->decomp->localNodeSets[proc];
}


void buildShadowEdges( Partition_Index proc ) {
	Index   indexCnt;
	Index*  indices;

	if( !meshLayout->decomp->shadowElementSets )
		return;
	
	IndexSet_RemoveAll( litEdges );
	IndexSet_GetMembers( meshLayout->decomp->shadowElementSets[proc], &indexCnt, &indices );
	if( indexCnt ) {
		Index   edgeCnt;
		Index*  edges;
		Index   edge_I;
		
		edgeCnt = ElementLayout_BuildEdgeSubset( meshLayout->elementLayout, indexCnt, indices, &edges );
		for( edge_I = 0; edge_I < edgeCnt; edge_I++ )
			IndexSet_Add( litEdges, edges[edge_I] );
		
		if( edges )
			Memory_Free( edges );
	}
	Memory_Free( indices );
	
	litNodes = meshLayout->decomp->shadowNodeSets[proc];
}


void keyboard( void ) {
	SDL_Event event;
	
	if ( SDL_PollEvent(&event) )
	{
		switch( event.type ){
			
			case SDL_QUIT:{
				quit = 1;  
			}
			break;

			case SDL_VIDEORESIZE:{
				reshape( event.resize.w, event.resize.h );					 
			}
			break;

			case SDL_KEYDOWN:{

				switch (event.key.keysym.sym){
					
					case SDLK_ESCAPE:
					{
						quit = 1;
					}
					break;
			
					case ']':
					{
						if( ++viewProc >= meshLayout->decomp->procsInUse )
							viewProc = meshLayout->decomp->procsInUse - 1;
						build( viewProc );

						display();
					}
					break;
				
					case '[':
					{
						if( --viewProc >= meshLayout->decomp->procsInUse )
							viewProc = 0;
						build( viewProc );

						display();
					}
					break;
				
					case SDLK_f:
					{
						build = (build == buildLocalEdges) ? buildShadowEdges : buildLocalEdges;
						build( viewProc );

						display();
					}
					break;

					default:
					{
					
					}
					break;
				}
						 
			}
			break;

			case SDL_MOUSEMOTION:{
				mouseMotion( event.motion.x, event.motion.y );
			}
			break;
			
			case SDL_MOUSEBUTTONDOWN:{
				mouseClick( event.button.button, event.button.state, event.button.x, event.button.y );
			}
			break;
			
			case SDL_MOUSEBUTTONUP:{
				mouseClick( event.button.button, event.button.state, event.button.x, event.button.y );
			}
			
			default:
			{
				
			}
			break;
		}
	}
}


int main( int argc, char* argv[] ) {
	int     rank;
	
	/* Initialise MPI */
	MPI_Init( &argc, &argv );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );

	// parse cmd-line params
	
	/* Build StGermain constructs */
	Base_Init( &argc, &argv );
	DiscretisationGeometry_Init( &argc, &argv );
	DiscretisationMesh_Init( &argc, &argv );
	
	buildStGermain();
	
	if( rank == 0 ) {
		/* Root process, handles visualisation */

		/* Initialize SDL for video output */
		if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
			fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
			exit(1);
		}

		/* Create a 640x480 OpenGL screen */
		if ( SDL_SetVideoMode(WIDTH, HEIGHT, 0, SDL_OPENGL) == NULL ) {
			fprintf(stderr, "Unable to create OpenGL screen: %s\n", SDL_GetError());
			SDL_Quit();
			exit(2);
		}
		
		/* Initialise GLUT */
		/*glutInit( &argc, argv );
		glutInitDisplayMode( GLUT_DEPTH | GLUT_RGB | GLUT_DOUBLE | GLUT_MULTISAMPLE );
		glutCreateWindow( "vis2" );
		glutDisplayFunc( display );
		glutInitWindowPosition( 200, 0 );
		glutInitWindowSize( 300, 300 );
		glutReshapeFunc( reshape );
		glutKeyboardFunc( keyboard );
		glutMouseFunc( mouseClick );
		glutMotionFunc( mouseMotion );*/
		
		/* Initialise OpenGL */
		glShadeModel( GL_SMOOTH );
		glFrontFace( GL_CCW );
		glEnable( GL_DEPTH_TEST );
		glClearColor( 0.3, 0.3, 0.3, 1.0 );
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		glPointSize( 5.0 );
		
		/* Convert StGermain to OpenGL */
		calcOrigin();
		calcBBox();
		litEdges = IndexSet_New( meshLayout->elementLayout->edgeCount );
		litNodes = IndexSet_New( meshLayout->nodeLayout->nodeCount );
		buildLocalEdges( 0 );
		
		//atexit( cleanUp );
		//glutMainLoop();
		while (!quit){
			keyboard();
			display();		
		}
		
		cleanUp();
	}
	
	// Destroy StGermain constructs, finalize
	DiscretisationMesh_Finalise();
	DiscretisationGeometry_Finalise();
	Base_Finalise();

	MPI_Finalize();

	return EXIT_SUCCESS;
}

