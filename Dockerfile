FROM ubuntu:jammy

RUN apt-get update && \
  DEBIAN_FRONTEND='noninteractive' \
  DEBCONF_NONINTERACTIVE_SEEN='true' \
  apt-get install --yes \
    gfortran \
    build-essential \
    git \
    python2 \
    libxml2-dev \
    libgsl-dev \
    curl \
    openmpi-bin \
    openmpi-common \
    libopenmpi-dev \
    doxygen \
    vim

RUN useradd \
--create-home \
snac_user

WORKDIR /home/snac_user

USER snac_user

ENV HOME=/home/snac_user
ENV MPI_DIR=/usr/lib/x86_64-linux-gnu/openmpi
ENV MPI_BINDIR=/usr/bin
ENV MPI_INCDIR=${MPI_DIR}/include
ENV MPI_LIBDIR=${MPI_DIR}/lib
ENV MPI_RUN=${MPI_BINDIR}/mpirun
ENV PATH=${MPI_BINDIR}:${PATH}
ENV LD_LIBRARY_PATH=${MPI_LIBDIR}"${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
# ENV LD_LIBRARY_PATH=${MPI_LIBDIR}:${LD_LIBRARY_PATH}
# ENV LD_LIBRARY_PATH=${MPI_LIBDIR}
ENV CC=mpicc
ENV CXX=mpicxx
ENV GSL_DIR=/usr
ENV SNAC_DIR=${HOME}/snac
ENV SNAC_BLDDIR=${SNAC_DIR}/build
ENV SNAC_BINDIR=${SNAC_BLDDIR}/bin
ENV SNAC_INCDIR=${SNAC_BLDDIR}/include
ENV SNAC_LIBDIR=${SNAC_BLDDIR}/lib
ENV PATH=${SNAC_BINDIR}:${PATH}
ENV LD_LIBRARY_PATH=${SNAC_LIBDIR}:${LD_LIBRARY_PATH}

RUN git clone 'https://github.com/geodynamics/snac.git';

RUN cd snac; ./configure.sh; make
