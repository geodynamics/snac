/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003, Victorian Partnership for Advanced Computing (VPAC) Ltd, 110 Victoria Street, Melbourne, 3053, Australia.
**
** Authors:
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**	Patrick D. Sunter, Software Engineer, VPAC. (pds@vpac.org)
**	Luke J. Hodkinson, Computational Engineer, VPAC. (lhodkins@vpac.org)
**	Siew-Ching Tan, Software Engineer, VPAC. (siew@vpac.org)
**	Alan H. Lo, Computational Engineer, VPAC. (alan@vpac.org)
**	Raquibul Hassan, Computational Engineer, VPAC. (raq@vpac.org)
**
**  This library is free software; you can redistribute it and/or
**  modify it under the terms of the GNU Lesser General Public
**  License as published by the Free Software Foundation; either
**  version 2.1 of the License, or (at your option) any later version.
**
**  This library is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**  Lesser General Public License for more details.
**
**  You should have received a copy of the GNU Lesser General Public
**  License along with this library; if not, write to the Free Software
**  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
**
** $Id: Class.c 2933 2005-05-12 06:13:10Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* Makes a web service call to submit the results of a make */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <libcsoap/soap-client.h>


const char* Regresstor_URN = "urn:Regresstor";			/* The name of the web service class */
const char* Regresstor_SubmitCheckMethod = "submitCheck";	/* The name of the web service function */

/* Index Positions to the paramter array */
#define RunID_Pos		0
#define UnitName_Pos		1
#define TestName_Pos		2
#define Passed_Pos		3
#define OutputLocation_Pos	4
#define URL_Pos			5		/* The URL for the submit build function */

/* The number of paramters in the submit build function */
#define Parameter_Num_Pos	6

/* The parameter names to the submit build function */
const char* parameterNames[Parameter_Num_Pos] = {
	"RunID",
	"UnitName",
	"TestName",
	"Passed",
	"OutputLocation",
	"URL" };

void printUsage();
void initParametersArray( char** parameters );
void fetchParametersFromArgv( char** parameters, int argc, char** argv );
int checkParameters( char** parameters );
char* getFileContents( char* filename );
int interpretResponse( SoapCtx* response, xmlChar** buildID );

int main( int argc, char** argv ) {

	SoapCtx* request;
	SoapCtx* response;
	herror_t error;
	int ii;

	/* The array of parameter values to be fetched from argv. */
	char* parameters[Parameter_Num_Pos];

	xmlChar* buildID;

	if ( argc != Parameter_Num_Pos + 1 ) {
		printf( "Regresstor: Invalid Number of arguments! Num given: %d\n", argc );
		printUsage();
		return EXIT_FAILURE;
	}

	initParametersArray( parameters );
	fetchParametersFromArgv( parameters, argc, argv );

	if ( ! checkParameters( parameters ) ) {
		printUsage();
		return EXIT_FAILURE;
	}

	printf( "Regresstor: calling web service %s(), %s at %s.\n",
		Regresstor_SubmitCheckMethod,
		Regresstor_URN,
		parameters[URL_Pos]);

	/* SOAP CALL */
	error = soap_client_init_args( argc, argv );
	if ( error != H_OK ) {
		log_error4( "%s():%s [%d]", herror_func(error), herror_message(error), herror_code(error) );
		herror_release(error);
		return EXIT_FAILURE;
	}
	
	error = soap_ctx_new_with_method( Regresstor_URN, Regresstor_SubmitCheckMethod, &request );
	if ( error != H_OK ) {
		log_error4( "%s():%s [%d]", herror_func(error), herror_message(error), herror_code(error) );
		herror_release(error);
		return EXIT_FAILURE;
	}

	/* Add parameters into envelope except for the URL */
	soap_env_push_item( request->env, NULL, "parameters" );

	for ( ii = 0; ii < URL_Pos; ++ii ) {
		if ( ii == OutputLocation_Pos ) {
			/* Attach file */
			char* contents;
			char* paramName = NULL;
			int sendDummyFile = 1;

			paramName = "Output";

			if ( strcmp( parameters[ii], "none" ) != 0 ) {
				contents = getFileContents( parameters[ii] );
				if ( contents != NULL ) {
					soap_env_add_item( request->env, "xsd:string", paramName, contents );
					free( contents );
					sendDummyFile = 0;
				}
			}
			if ( sendDummyFile ) {
				soap_env_add_item( request->env, "xsd:string", paramName, "No output file" );
			}
		}
		else
		{
			soap_env_add_item( request->env, "xsd:string", parameterNames[ii], parameters[ii] );
		}
	}

	/* Check commandline to see if diff passed. If fail, add a "Diff Failed" sub test */
	if ( strcmp( parameters[Passed_Pos], "0" ) == 0 ) {
		/* Add the sub test */
		soap_env_add_item( request->env, "xsd:string", "FailedSubTests", "Diff failed" );
	}
	
	soap_env_pop_item( request->env );
	
	error = soap_client_invoke( request, &response, parameters[URL_Pos], "");
                                                                                                                                    
	if ( error != H_OK ) {
		log_error4( "[%d] %s(): %s ", herror_code(error), herror_func(error), herror_message(error) );
		herror_release( error );
		soap_ctx_free( request );
		return EXIT_FAILURE;
	}

	interpretResponse( response, &buildID );

	soap_ctx_free( request );
	soap_ctx_free( response );

	soap_client_destroy();


	return EXIT_SUCCESS;
}

void printUsage() {
	int ii;
	
	printf( "Usage:\n" );
	printf( "\t./regresstorCheckDiff (param)=(value)\n" );
	printf( "All parameters must be given!\n" );
	printf( "Parameters:\n" );
	for ( ii = 0; ii < Parameter_Num_Pos; ++ii ) {
		printf( "\t%s\n", parameterNames[ii] );
	}
}

/* Initialises all values to NULL */
void initParametersArray( char** parameters ) {
	int ii;
	for ( ii = 0; ii < Parameter_Num_Pos; ++ii ) {
		parameters[ii] = NULL;
	}
}
void fetchParametersFromArgv( char** parameters, int argc, char** argv ) {
	int ii, jj;
	char* param;
	char* value;
	int pos;

	for ( ii = 1; ii < argc; ++ii ) {
		if ( !strchr( argv[ii], '=' ) ) {
			printf( "Regresstor: Invalid argument %s. Assign (param)=(value)\n", argv[ii] );
			return;
		}

		param = strtok( argv[ii], "=" );
		value = strtok( NULL, "=" );

		if ( !param ) {
			printf( "Regresstor: Invalid argument %s.\n", argv[ii] );
			return;
		}
		if ( !value ) {
			printf( "Regresstor: Parameter %s needs a value.\n", param );
			return;
		}

		pos = -1;
		for ( jj = 0; jj < Parameter_Num_Pos; ++jj ) {
			if ( strcmp( param, parameterNames[jj] ) == 0 ) {
				pos = jj;
				break;
			}
		}
		if ( pos == -1 ) {
			printf( "Regresstor: Invalid param %s\n", argv[ii] );
			return;
		}
		printf("\t%s=%s\n", parameterNames[pos], value );
		parameters[pos] = value;
	}
}
int checkParameters( char** parameters ) {
	int ii;
	for ( ii = 0; ii < Parameter_Num_Pos; ++ii ) {
		if ( parameters[ii] == NULL ) {
			printf( "Regresstor: Parameter %s was not given.\n", parameterNames[ii] );
			return 0;
		}
	}

	return 1;
}
char* getFileContents( char* filename ) {
	FILE* file;
	long size;
	char* contents;
                                                                                                                                    
	file = fopen( filename, "r" );
	if ( !file ) {
		return NULL;
	}
	if( fseek( file, 0L, SEEK_END ) != 0 ) {
		return NULL;
	}
	size = ftell( file ) + 1;
	if ( size < 1 ) {
		return NULL;
	}
	contents = (char*)malloc( size );
	if ( contents == NULL ) {
		return NULL;
	}
	rewind( file );
	fread( contents, 1, size - 1, file );
	contents[ size - 1 ] = '\0';

	fclose( file );

	return contents;
}
int interpretResponse( SoapCtx* response, xmlChar** buildID ) {
	xmlNodePtr method;
	xmlNodePtr node;
	xmlChar* value;
                                                                                                                                    
	method = soap_env_get_method( response->env );

	if ( method ) {
		if ( strcmp( (char*)method->name, "Fault" ) == 0 ) {
			printf( "Regresstor: A SOAP error has occured:\n" );
			node = soap_xml_get_children( method );
			while ( node ) {
				value = xmlNodeListGetString( node->doc, node->xmlChildrenNode, 1 );
				printf( "Regresstor: %s=%s\n", node->name, value );
				node = soap_xml_get_next(node);
			}
			return 0;
		}
		
		node = soap_xml_get_children( method );		/* The s-gensym3 struct */

		if ( node->children ) {
			/* This is where the response values are */
			node = node->children;
			while ( node ) {
				value = xmlNodeListGetString( node->doc, node->xmlChildrenNode, 1 );

				if ( strcmp( (char*)node->name, "errorMsg" ) == 0 ) {
					printf( "Regresstor: Error - %s\n", value );
					return 0;
				}
				
				node = soap_xml_get_next(node);
			}
		}
	}
	else {
		printf( "Regresstor: Non-SOAP response.\n" );
	}
	printf( "Regresstor: Successfully submited check.\n" );
	return 1;
}
