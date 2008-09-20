#include <stdio.h>

void method( int *i )
{
   printf( "-DFORTRAN_NORMAL" );
}

void method_( int *i )
{
   printf( "-DFORTRAN_SINGLE_TRAILINGBAR" );
}

void method__( int *i )
{
   printf( "-DFORTRAN_DOUBLE_TRAILINGBAR" );
}

void METHOD( int *i )
{
   printf( "-DFORTRAN_UPPERCASE" );
}
