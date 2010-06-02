/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
** Role:
**	Handles the temperature initial and boundary conditions
**
** Assumptions:
**	None as yet.
**
** Comments:
**	None as yet.
**
** $Id: TestCondFunc.h 1506 2004-06-02 18:55:34Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __SnacCartesian_hetero_h__
#define __SnacCartesian_hetero_h__
  

        //typedef enum    {nodeIC,elementIC,bothIC} heterogeneityType;
typedef enum    {SnacDyke,SnacSphere,SnacCylinder,SnacCylinder_H,SnacCylinder_V,SnacUpperLimit,SnacLowerLimit,SnacRightLimit,SnacLeftLimit,SnacFrontLimit,SnacBackLimit} heterogeneitygeometry;
        typedef struct _Snac_Hetero                     Snac_Hetero;
        /* Heterogeneity*/
        struct _Snac_Hetero {
                /* for adjusting geometry  */
                heterogeneitygeometry   geom;
                int                     IsElementVC;
                int                     IsNodeVC;
                double                  a_shape;
                double                  b_shape;
                double                  c_shape;
                double                  d_shape;
                double                  e_shape;
                /* for adjusting the variable */
                CompositeVC*            elementVC;
                CompositeVC*            nodeVC;
        };

        void _SnacHetero_InitialCondition( void* _context );
        void SnacHetero_Apply(Index phaseI, Index nodeI,Index elementI,  void* _context, void* _heteroProperty);
        void  printHetero_Properties( void* _heteroproperty, int hetero_I, Stream* stream );
        void  getHetero_Dictionary_Entry_Values( Dictionary* heteroDict, Index hetero_I, Index* nodeI, Index* elementI, void* _context, void* _heteroProperty);
        void _Snac_Dyke(double a,double b,double c,double d,double e,Coord x,int*  ans);
        void _Snac_Sphere(double a,double b,double c,double d,double e,Coord x,int*  ans);
        void _Snac_Cylinder(double a,double b,double c,double d,double e,Coord x,int*  ans);
        void _Snac_CylinderH(double a,double b,double c,double d,double e,Coord x,int*  ans);
        void _Snac_CylinderV(double a,double b,double c,double d,double e,Coord x,int*  ans);
        void _Snac_UpperLimit(double a,double b,double c,double d,double e,Coord x,int*  ans);
        void _Snac_RightLimit(double a,double b,double c,double d,double e,Coord x,int*  ans);
        void _Snac_FrontLimit(double a,double b,double c,double d,double e,Coord x,int*  ans);
        void _SnacHetero_node ( Index numHetero, Index node_dI,  void* _context, void* _heteroProperty );
        void _SnacHetero_element ( Index numHetero, Index element_dI,  void* _context, void* _heteroProperty );
        void Is_coord_Inside(Index* WhichOne,Index hetero_I, void* _heteroProperty,Coord* coord);
#endif /* __SnacCartesian_hetero_h__ */
