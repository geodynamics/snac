#include <stdio.h>
int main() {
	#if defined (__GNUC__) && !defined (__INTEL_COMPILER)
		printf( "gnu\n" );
		return 0;
	#endif
	#if defined (__INTEL_COMPILER)
		printf( "intel\n" );
		return 0;
	#endif
	#ifdef __osf__
		printf( "osf\n" );
		return 0;
	#endif
	#ifdef __xlc__
		printf( "ibmxl\n" );
		return 0;
	#endif
	#if ( defined( sparc ) || defined( __sparc ) || defined( __sparc__ ) ) && !defined(__GNUC__)
		printf( "sparc\n" );
		return 0;
	#else
		return 0;
	#endif
}
