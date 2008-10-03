

#include "forwardDecl.h"

#include "types.h"
#include "FailedSubTest.h"
#include "UnitTest.h"
#include "RegressionTest.h"

#include <stdlib.h>
#include <signal.h>
#include <libcsoap/soap-client.h>
#include <libxml/xmlIO.h>

const char* Regresstor_URN = "urn:Regresstor";			/* The name of the web service class */
const char* Regresstor_SubmitCheckMethod = "submitCheck";       /* The name of the web service function */

UnitTest* stgRegressionTest = 0;

typedef void (*Regresstor_HandlePtr)(int);
void RegressionTest_CrashHandler( int sig );
Regresstor_HandlePtr prevSegvHandler;
Regresstor_HandlePtr prevBusHandler;

/* Interface functions */
void RegressionTest_Init_Func( char* url, char* runID, char* unitName, char* testName ) {

	stgRegressionTest = UnitTest_New( url, runID, unitName, testName );

	prevSegvHandler = signal( SIGSEGV, RegressionTest_CrashHandler );
	prevBusHandler = signal( SIGBUS, RegressionTest_CrashHandler );
}
void RegressionTest_Finalise_Func() {
	SoapCtx* request;
	SoapCtx* response;
	herror_t error;
	
	int dummyArgc = 0;
	char** dummyArgv = NULL;

	if ( !stgRegressionTest ) {
		return;
	}
	if ( !stgRegressionTest->url || !stgRegressionTest->runID ) {
		return;
	}

	error = soap_client_init_args( dummyArgc, dummyArgv );
	if ( error != H_OK ) {
		/* Print Error */
		herror_release(error);
		return;
	}

	error = soap_ctx_new_with_method( Regresstor_URN, Regresstor_SubmitCheckMethod, &request );
	if ( error != H_OK ) {
		/* Print Error */
		herror_release(error);
		return;
	}

	soap_env_push_item( request->env, NULL, "parameters" );

	soap_env_add_item( request->env, "xsd:string", "RunID", stgRegressionTest->runID );
	soap_env_add_item( request->env, "xsd:string", "UnitName", stgRegressionTest->unitName );
	soap_env_add_item( request->env, "xsd:string", "TestName", stgRegressionTest->testName );

	if ( stgRegressionTest->subTestCount > 0 ) {
		int ii;
/*		xmlNodePtr failedSubTests;*/

/*		failedSubTests = soap_env_push_item( request->env, NULL, "FailedSubTests" ); */

		/* Make this an 'array' data type for SOAP */
/*		xmlNewProp( failedSubTests, (xmlChar*)"xsi:type", (xmlChar*)"SOAP-ENC:Array" );
		xmlNewProp( failedSubTests, (xmlChar*)"SOAP-ENC:arrayType", (xmlChar*)"xsd:string[2]" );
*/
		/* Added the array elements */
		for ( ii = 0; ii < stgRegressionTest->subTestCount; ++ii ) {
/*			soap_env_add_item( request->env, "xsd:string", "subTest", stgRegressionTest->subTests[ii]->subTestName );*/
			soap_env_add_item( request->env, "xsd:string", "FailedSubTests", stgRegressionTest->subTests[ii]->subTestName );
		}

		/* End array */
/*		soap_env_pop_item( request->env );*/
	}

	soap_env_pop_item( request->env );

/*	soap_xml_doc_print( request->env->root->doc );*/
	
	error = soap_client_invoke( request, &response, stgRegressionTest->url, "");
                                                                                                                                    
	if ( error != H_OK ) {
		/* Print Error */
		herror_release( error );
		soap_ctx_free( request );
		return;
	}

/*	soap_xml_doc_print( response->env->root->doc );*/

	soap_ctx_free( request );
	soap_ctx_free( response );

	soap_client_destroy();

	UnitTest_Delete( stgRegressionTest );
}

/* Private member functions */
void RegressionTest_Check_Func( UnitTest* self, int expression, void* stream, char* subTestName, char* description ) {
	if ( !stgRegressionTest ) {
		return;
	}

	if ( expression ) {
		/* Pass */
		Journal_Printf( stream, "Passed test %s: %s\n", subTestName, description );
	}
	else {
		/* Fail */
		Journal_Printf( stream, "Failed test %s: %s\n", subTestName, description );

		UnitTest_Add_FailedSubTest( stgRegressionTest, FailedSubTest_New( subTestName ) );
	}
}


void RegressionTest_CrashHandler( int sig ) {

	switch ( sig ) {
		case SIGSEGV:
			UnitTest_Add_FailedSubTest( stgRegressionTest, FailedSubTest_New( "SIGSEGV: Invalid memory reference" ) );
			break;
		case SIGBUS:
			UnitTest_Add_FailedSubTest( stgRegressionTest, FailedSubTest_New( "SIGBUS: Bad memory access" ) );
			break;
	}

	RegressionTest_Finalise();

	switch ( sig ) {
		case SIGSEGV:
			prevSegvHandler( sig );
			break;
		case SIGBUS:
			prevBusHandler( sig );
			break;
	}
	
	exit( 1 );
}

