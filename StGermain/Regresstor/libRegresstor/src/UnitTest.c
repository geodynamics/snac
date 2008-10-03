
#include <stdlib.h>

#include "types.h"
#include "FailedSubTest.h"
#include "UnitTest.h"



UnitTest* UnitTest_New( char* url, char* runID, char* unitName, char* testName ) {
	UnitTest* result;

	result = (UnitTest*)malloc( sizeof(UnitTest) );

	result->url = url;
	result->runID = runID;
	result->unitName = unitName;
	result->testName = testName;
	result->subTests = NULL;
	result->subTestCount = 0;
	result->subTestSize = 0;

	return result;
}

void UnitTest_Delete( UnitTest* self ) {

	if ( self ) {
		int ii;
		
		for ( ii = 0; ii < self->subTestCount; ++ii ) {
			if ( self->subTests[ii] ) {
				FailedSubTest_Delete( self->subTests[ii] );
				self->subTests[ii] = NULL;
			}
		}
		if ( self->subTests ) {
			free( self->subTests );
		}
		
		free( self );
	}
}


void UnitTest_Add_FailedSubTest( UnitTest* self, FailedSubTest* test ) {
	if ( !self || !test ) {
		return;
	}

	if ( self->subTestCount >= self->subTestSize ) {
		self->subTestSize++;
		self->subTests = (FailedSubTest**)realloc( self->subTests, sizeof(FailedSubTest*) * self->subTestSize );
	}
	self->subTests[self->subTestCount] = test;
	self->subTestCount++;
}

