      Program Conftest

      implicit none
      logical DONE

      write(*,10)

#ifdef __GNUC__
#ifndef __INTEL_COMPILER
   10 format('gnu')
#endif
#endif
#ifdef __osf__
   10 format('osf')
#endif
#ifdef __INTEL_COMPILER
   10 format('intel')
#endif 
#ifndef __GNUC__
#ifndef __INTEL_COMPILER
#ifndef __osf__
   10 format('unknown')
#endif
#endif
#endif
      END
