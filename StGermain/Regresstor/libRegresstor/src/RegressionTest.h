

#ifndef __Regresstor_RegressionTest_h__
#define __Regresstor_RegressionTest_h__

void RegressionTest_Init_Func( char* url, char* runID, char* unitName, char* testName );
void RegressionTest_Finalise_Func();
void RegressionTest_Check_Func( UnitTest* self, int expression, void* stream, char* subTestName, char* description );

extern UnitTest* stgRegressionTest;

#ifdef REGRESSION_RUN
	#define RegressionTest_Init( unitName ) \
		RegressionTest_Init_Func( REGRESSTOR_URL, RUNID, unitName, __FILE__ )
	#define RegressionTest_Finalise RegressionTest_Finalise_Func
	#define RegressionTest_Check( expression, stream, subTestName, description ) \
		RegressionTest_Check_Func( stgRegressionTest, expression, stream, subTestName, description )
	
#else
	/* If not a regression run, these functions mean nothing */
	#define RegressionTest_Init( unitName )
	#define RegressionTest_Finalise()

	/* If not a regression run, firewall instead of check */
	#define RegressionTest_Check( expression, stream, subTestName, description) \
		{ \
			Journal_Firewall( expression, stream, "Failed test %s: %s\n", subTestName, description ); \
			Journal_Printf( stream, "Passed test %s: %s\n", subTestName, description ); \
		}
#endif

#endif


