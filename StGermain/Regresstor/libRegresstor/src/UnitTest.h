

#ifndef __UnitTest_h__
#define __UnitTest_h__


#define __UnitTest \
	char*		url; \
	char*		runID; \
	char*		unitName; \
	char*		testName; \
	FailedSubTest**	subTests; \
	unsigned int	subTestCount; \
	unsigned int	subTestSize;
	
struct UnitTest {
	__UnitTest
};

UnitTest* UnitTest_New( char* url, char* runID, char* unitName, char* testName );

void UnitTest_Delete( UnitTest* self );

void UnitTest_Add_FailedSubTest( UnitTest* self, FailedSubTest* test );

#endif
