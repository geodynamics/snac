#include <stdio.h>

int main() {
	/* Are we little or big endian?  From Harbison&Steele. */
	/* From PETSc configure source */
	union
	{
		long l;
		char c[sizeof(long)];
	} u;
	u.l = 1;
	printf( "%d", u.c[sizeof(long) - 1] == 1 );
	return 0;
}
