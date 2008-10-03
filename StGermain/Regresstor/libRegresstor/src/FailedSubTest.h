

#ifndef __FailedSubTest_h__
#define __FailedSubTest_h__


#define __FailedSubTest \
	char*		subTestName;

struct FailedSubTest {
	__FailedSubTest
};

FailedSubTest* FailedSubTest_New( char* subTestName );

void FailedSubTest_Delete( FailedSubTest* self );


#endif
