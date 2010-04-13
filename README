-- For installation, see the user manual or INSTALL for quick start.

-- Some customization is required for installation of SNAC on some platforms.
The recommended set of environment variables is given below.
Note that some values need to be changed according to a user's setting.

In csh:
======================================================
setenv MPI_DIR /opt/local
setenv MPI_BINDIR ${MPI_DIR}/bin
setenv MPI_INCDIR ${MPI_DIR}/include/mpich2
setenv MPI_LIBDIR ${MPI_DIR}/lib
setenv MPI_RUN ${MPI_BINDIR}/mpirun
setenv PATH ${MPI_BINDIR}:{$PATH}
setenv LD_LIBRARY_PATH ${MPI_LIBDIR}:{LD_LIBRARY_PATH}
setenv CC mpicc
setenv CXX mpicxx

setenv GSL_DIR /opt/local

setenv DL_DIR /usr

setenv SNAC_DIR $HOME/Src/SNAC
setenv SNAC_BLDDIR ${SNAC_DIR}/build
setenv SNAC_BINDIR ${SNAC_BLDDIR}/bin
setenv SNAC_INCDIR ${SNAC_BLDDIR}/include
setenv SNAC_LIBDIR ${SNAC_BLDDIR}/lib
setenv PATH ${SNAC_BINDIR}:{$PATH}
setenv LD_LIBRARY_PATH ${SNAC_LIBDIR}:{LD_LIBRARY_PATH}
======================================================

In bash:
======================================================
export MPI_DIR=/opt/local
export MPI_BINDIR=${MPI_DIR}/bin
export MPI_INCDIR=${MPI_DIR}/include/mpich2
export MPI_LIBDIR=${MPI_DIR}/lib
export MPI_RUN=${MPI_BINDIR}/mpirun
export PATH=${MPI_BINDIR}:{$PATH}
export LD_LIBRARY_PATH=${MPI_LIBDIR}:{LD_LIBRARY_PATH}
export CC=mpicc
export CXX=mpicxx

export GSL_DIR=/opt/local

export SNAC_DIR=${HOME}/Src/SNAC
export SNAC_BLDDIR=${SNAC_DIR}/build
export SNAC_BINDIR=${SNAC_BLDDIR}/bin
export SNAC_INCDIR=${SNAC_BLDDIR}/include
export SNAC_LIBDIR=${SNAC_BLDDIR}/lib
export PATH=${SNAC_BINDIR}:{$PATH}
export LD_LIBRARY_PATH=${SNAC_LIBDIR}:{LD_LIBRARY_PATH}
======================================================


-- For Mac OS X Leopard (and probably Snow Leopard)

The following steps are recommended in order to ensure that all the relevant utilities, libraries and environment variables are set up as needed by SNAC.
	- install XCode
	- install MacPorts (and Porticus if you want the GUI)
	- using MacPorts/Porticus, install gsl and mpich2 (don't choose a variant)
	- set a bunch of environment variables to help the configure script find them
