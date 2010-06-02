/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003,
**	Steve Quenette, 110 Victoria Street, Melbourne, Victoria, 3053, Australia.
**	Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
**	University of Texas, 1 University Station, Austin, Texas, 78712, USA.
**
** Authors:
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**	Stevan M. Quenette, Visitor in Geophysics, Caltech.
**	Luc Lavier, Research Scientist, The University of Texas. (luc@utig.ug.utexas.edu)
**	Luc Lavier, Research Scientist, Caltech.
**
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2, or (at your option) any
** later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**
** $Id: TestCondFunc.c  2006-06-19 18:21:50Z LaetitiaLePourhiet $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "Register.h"
#include "SnacHetero.h"
#include <stdio.h>
#include <math.h>
#include <assert.h>

void _Snac_Dyke(double a,double b,double c,double d,double e,Coord x,int*  pans) {
      double   dummy = (a*x[0]+b*x[1]+c*x[2]+d);
      double   Distance = dummy*dummy/(a*a+b*b+c*c);
      *pans  = Distance <= e*e/4 ;
	  if( *pans )
		  fprintf(stderr,"%e %e %e %e %e (%e %e %e) dist=%e, %e\n",a,b,c,d,e,x[0],x[1],x[2],Distance,e*e/4);
}

void _Snac_Sphere(double a,double b,double c,double d,double e,Coord x,int*  ans) {
      
      double  dummy0   = x[0]-a;
      double  dummy1   = x[1]-b;
      double  dummy2   = x[2]-c;
      double  Distance = dummy0*dummy0+dummy1*dummy1+dummy2*dummy2;
      *ans    = Distance <= e*e ;
}

void _Snac_Cylinder(double a,double b,double c,double d,double e,Coord x, int*  ans   ) {

      double  dummy0 = (x[0]-a*x[2]-c);
      double  dummy1 = (x[1]-b*x[2]-d);
      double  dummy2 =  b*(x[0]-c)-a*(x[1]-d);
      double  Distance = dummy0*dummy0+dummy1*dummy1+dummy2*dummy2/(a*a+b*b+1.0);
      *ans    = Distance <= e*e ;
}

void _Snac_CylinderH(double a,double b,double c,double d,double e,Coord x, int*  ans   ) {

      double  dummy0 = (x[0]-a*x[2]-c);
      double  Distance = dummy0*dummy0;
      *ans    = Distance <= e*e ;
}

void _Snac_CylinderV(double a,double b,double c,double d,double e,Coord x, int*    ans ) {
      double  dummy0 = x[0]-a;
      double  dummy2 = x[2]-c;
      double Distance = dummy0*dummy0+dummy2*dummy2;
      *ans    = Distance <= e*e ;
}

void _Snac_UpperLimit(double a,double b,double c,double d,double e,Coord x, int*    ans ) {
      double   Distance = (-a*x[0]-c*x[2]-d)/b;
      *ans    =  x[1] <= Distance;
}

void _Snac_LowerLimit(double a,double b,double c,double d,double e,Coord x, int*    ans ) {
      double   Distance = (a*x[0]+c*x[2]+d)/b;
      *ans    =  x[1] >= Distance;
}

void _Snac_RightLimit(double a,double b,double c,double d,double e,Coord x, int*  ans   ) {
      double   Distance = (-b*x[1]-c*x[2]-d)/a;
      *ans    = x[0] <= Distance  ;
}

void _Snac_LeftLimit(double a,double b,double c,double d,double e,Coord x, int*  ans   ) {
      double   Distance = (b*x[1]+c*x[2]+d)/a;
      *ans    = x[0] >= Distance  ;
}

void _Snac_FrontLimit(double a,double b,double c,double d,double e,Coord x, int*  ans   ) {
      double   Distance = (-a*x[0]-b*x[1]-d)/c;
      *ans    = x[2] <= Distance  ;
}

void _Snac_BackLimit(double a,double b,double c,double d,double e,Coord x, int*  ans   ) {
      double   Distance = (a*x[0]+b*x[1]+d)/c;
      *ans    = x[2] >= Distance  ;
}

void _SnacHetero_node ( Index numHetero, Index node_dI,  void* _context, void* _heteroProperty ) {
        Snac_Context*                   context         = (Snac_Context*)_context;
        Snac_Hetero*                    heteroProperty  = (Snac_Hetero*)_heteroProperty;
        Coord*                          coord;
        Index                           WhichOne;
        Index                           hetero_I;
            coord       = Snac_NodeCoord_P( context, node_dI );
            WhichOne = 0;
        for (hetero_I = 0; hetero_I<numHetero; hetero_I++){
             if (heteroProperty[hetero_I].IsNodeVC) 
              Is_coord_Inside(&WhichOne,hetero_I,heteroProperty,coord);
             }
        if (WhichOne >0) VariableCondition_ApplyToIndex(heteroProperty[WhichOne-1].nodeVC,node_dI,context);
}     




void _SnacHetero_element ( Index numHetero, Index element_dI,  void* _context, void* _heteroProperty ) {
        Snac_Context*                   context         = (Snac_Context*)_context;
        Snac_Hetero*                    heteroProperty  = (Snac_Hetero*)_heteroProperty;
        Coord*                          pX;
        double                          X[3];
        Index                           inode;
        Index                           WhichOne;
        Index                           hetero_I;
        WhichOne = 0;
        X[0]     = 0.0;
        X[1]     = 0.0;
        X[2]     = 0.0;
        for (inode=0;inode<8;inode++)   {
                       X[0]  +=  Snac_Element_NodeCoord( context, element_dI, inode )[0];
                       X[1]  +=  Snac_Element_NodeCoord( context, element_dI, inode )[1];
                       X[2]  +=  Snac_Element_NodeCoord( context, element_dI, inode )[2];
                     }
        X[0]        = X[0]/8;
        X[1]        = X[1]/8;
        X[2]        = X[2]/8;
        pX  = &X;
        for (hetero_I = 0; hetero_I<numHetero; hetero_I++){
			if (heteroProperty[hetero_I].IsElementVC)  Is_coord_Inside(&WhichOne,hetero_I,heteroProperty,pX);
		}
        if (WhichOne >0) {
			VariableCondition_ApplyToIndex(heteroProperty[WhichOne-1].elementVC,element_dI,context);
		}

}     

void Is_coord_Inside(Index* pWhichOne,Index hetero_I, void* _heteroProperty,Coord* coord) {
        Snac_Hetero*                    hetero  =  (Snac_Hetero*)_heteroProperty;
        int                             In_or_Out;        
  
        /* retreive the geometric parameters*/
              switch (hetero[hetero_I].geom) {
                        case 0:
                                 _Snac_Dyke(hetero[hetero_I].a_shape,hetero[hetero_I].b_shape,hetero[hetero_I].c_shape,hetero[hetero_I].d_shape,hetero[hetero_I].e_shape,*coord,&In_or_Out); 
                                if(In_or_Out) *pWhichOne = (hetero_I+1); 
                                break;

                        case 1:
                                _Snac_Sphere(hetero[hetero_I].a_shape,hetero[hetero_I].b_shape,hetero[hetero_I].c_shape,hetero[hetero_I].d_shape,hetero[hetero_I].e_shape,*coord,&In_or_Out);
                                if(In_or_Out) *pWhichOne = (hetero_I+1);
                                break;

                        case 2:
                                _Snac_Cylinder(hetero[hetero_I].a_shape,hetero[hetero_I].b_shape,hetero[hetero_I].c_shape,hetero[hetero_I].d_shape,hetero[hetero_I].e_shape,*coord,&In_or_Out);
                                if(In_or_Out) *pWhichOne = (hetero_I+1);
                                break;

                        case 3:
                                _Snac_CylinderH(hetero[hetero_I].a_shape,hetero[hetero_I].b_shape,hetero[hetero_I].c_shape,hetero[hetero_I].d_shape,hetero[hetero_I].e_shape,*coord,&In_or_Out);
                                if(In_or_Out) *pWhichOne = (hetero_I+1);
                                break;

                        case 4:
                                _Snac_CylinderV(hetero[hetero_I].a_shape,hetero[hetero_I].b_shape,hetero[hetero_I].c_shape,hetero[hetero_I].d_shape,hetero[hetero_I].e_shape,*coord,&In_or_Out);
                                if(In_or_Out) *pWhichOne = (hetero_I+1);
                                break;

                        case 5:
                                _Snac_UpperLimit(hetero[hetero_I].a_shape,hetero[hetero_I].b_shape,hetero[hetero_I].c_shape,hetero[hetero_I].d_shape,hetero[hetero_I].e_shape,*coord,&In_or_Out);
                                if(In_or_Out) *pWhichOne = (hetero_I+1);
                                break;

                        case 6:
                                _Snac_LowerLimit(hetero[hetero_I].a_shape,hetero[hetero_I].b_shape,hetero[hetero_I].c_shape,hetero[hetero_I].d_shape,hetero[hetero_I].e_shape,*coord,&In_or_Out);
                                if(In_or_Out) *pWhichOne = (hetero_I+1);
                                break;

                        case 7:
                                _Snac_RightLimit(hetero[hetero_I].a_shape,hetero[hetero_I].b_shape,hetero[hetero_I].c_shape,hetero[hetero_I].d_shape,hetero[hetero_I].e_shape,*coord,&In_or_Out);
                                if(In_or_Out) *pWhichOne = (hetero_I+1);
                                break;

                        case 8:
                                _Snac_LeftLimit(hetero[hetero_I].a_shape,hetero[hetero_I].b_shape,hetero[hetero_I].c_shape,hetero[hetero_I].d_shape,hetero[hetero_I].e_shape,*coord,&In_or_Out);
                                if(In_or_Out) *pWhichOne = (hetero_I+1);
                                break;

                        case 9:
                                _Snac_FrontLimit(hetero[hetero_I].a_shape,hetero[hetero_I].b_shape,hetero[hetero_I].c_shape,hetero[hetero_I].d_shape,hetero[hetero_I].e_shape,*coord,&In_or_Out);
                                if(In_or_Out) *pWhichOne = (hetero_I+1);
                                break;

                        case 10:
                                _Snac_BackLimit(hetero[hetero_I].a_shape,hetero[hetero_I].b_shape,hetero[hetero_I].c_shape,hetero[hetero_I].d_shape,hetero[hetero_I].e_shape,*coord,&In_or_Out);
                                if(In_or_Out) *pWhichOne = (hetero_I+1);
                                break;

                        default:
                                assert(0);
                                break;
                       } //endswitch
  } 


