
#Finds the Absolute path to the Project Root directory
SHELL := /bin/bash
PROJ_ROOT := $(shell until test -r ./Makefile.system ; do cd .. ; done ; echo `pwd`)
include ${PROJ_ROOT}/Makefile.system

# Subdirectories
tests = lib${PROJECT}_DiscretisationShape

SRCS = $(wildcard *.c)
checks = \
	testBox2D.0of1.sh \
	testBox3D.0of1.sh \
	testConvexHull2D.0of1.sh \
	testConvexHull3D.0of1.sh \
	testCylinder.0of1.sh \
	testIntersection2D.0of1.sh \
	testIntersection2DDoubleNot.0of1.sh \
	testIntersection2DSingleNot.0of1.sh \
	testIntersection3D.0of1.sh \
	testIntersection3DDoubleNot.0of1.sh \
	testIntersection3DSingleNot.0of1.sh \
	testPolygonShape2D.0of1.sh \
	testPolygonShape3D.0of1.sh \
	testSphere2D.0of1.sh \
	testSphere3D.0of1.sh \
	testSphere2D-invert.0of1.sh \
	testSuperellipsoid2D.0of1.sh \
	testSuperellipsoid3D.0of1.sh \
	testUnion2D.0of1.sh \
	testUnion2DSingleNot.0of1.sh \
	testUnion3D.0of1.sh \
	testUnion3DSingleNot.0of1.sh 

ifdef HAVE_PYTHON
	checks += \
		testPython2D.0of1.sh \
		testPython3D.0of1.sh 
endif

EXTERNAL_INCLUDES = -I${INC_DIR}/${PROJECT}
EXTERNAL_LIBS = -L${LIB_DIR} -lStGermainDiscretisationShape -lStGermainDiscretisationGeometry -lStGermainBase ${RPATH_LFLAGS}

packages = MPI XML MATH DL 

ifdef HAVE_PYTHON
	packages += PYTHON
endif

ifdef USE_OPTIONAL_MATH
	# For full tensor math it needs some blas lapack functions only if available
	packages := LAPACK_BLAS ${packages}
        ifneq (Darwin,${SYSTEM})
	       EXTERNAL_LIBS +=  -lg2c
	endif
endif

include ${PROJ_ROOT}/Makefile.vmake
