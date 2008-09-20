
#include <stdlib.h>

#include "types.h"
#include "FailedSubTest.h"

FailedSubTest* FailedSubTest_New( char* subTestName ) {
	FailedSubTest* result;

	result = (FailedSubTest*)malloc( sizeof(FailedSubTest) );
	
	result->subTestName = subTestName;

	return result;
}

void FailedSubTest_Delete( FailedSubTest* self ) {

	if ( self ) {
		free( self );
	}
}
