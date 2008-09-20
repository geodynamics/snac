/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003,
**      Steve Quenette, 110 Victoria Street, Melbourne, Victoria, 3053, Australia.
**      Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
**      University of Texas, 1 University Station, Austin, Texas, 78712, USA.
**
** Authors:
**      Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**      Stevan M. Quenette, Visitor in Geophysics, Caltech.
**      Luc Lavier, Research Scientist, The University of Texas. (luc@utig.ug.utexas.edu)
**      Luc Lavier, Research Scientist, Caltech.
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
** $Id: SnacHetero_InitialCondition.c 3192 2006-06-14 03:07:59Z LaetitiaLePourhiet $
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "Register.h"
#include "SnacHetero.h"
#include <stdio.h>
#include <math.h>
#include <string.h>


void _SnacHetero_InitialCondition( void* _context ) {
        Snac_Context*   context  = (Snac_Context*)_context;
        Dictionary_Entry_Value*         heteroList = NULL;
        heteroList = Dictionary_Get( context->dictionary, "heterogeneities" );
        if( heteroList )
                {
                Index  phaseI   = 0;
                Index  nodeI;
                Index  elementI;
                nodeI    = 0; 
                elementI = 0;
                Dictionary_Entry_Value* heteroEntry;
                /* first get the number of phses */
                heteroEntry = Dictionary_Entry_Value_GetFirstElement( heteroList );
                while( heteroEntry ) 
                        {
                        phaseI++;
                        heteroEntry = heteroEntry->next;
                        }
                Snac_Hetero*   heteroProperty = (Snac_Hetero*)malloc( sizeof(Snac_Hetero) * phaseI );
                heteroEntry    = Dictionary_Entry_Value_GetFirstElement( heteroList );
                phaseI         = 0; 
                while( heteroEntry ) 
                        {
                        Dictionary* heteroDict = Dictionary_Entry_Value_AsDictionary( heteroEntry );
                        getHetero_Dictionary_Entry_Values( heteroDict, phaseI,&nodeI,&elementI,context, heteroProperty );
                        if( context->rank == 0 ) printHetero_Properties( heteroProperty, phaseI, context->snacInfo );
                        phaseI++;
                        heteroEntry = heteroEntry->next;
                        }
               SnacHetero_Apply (phaseI,nodeI,elementI,context, heteroProperty);
               }

}

void SnacHetero_Apply(Index phaseI, Index  nodeI,Index elementI,  void* _context, void* _heteroProperty) {
         Snac_Context*             context      = (Snac_Context*)_context;
         Snac_Hetero*              heteroProperty  = (Snac_Hetero*)_heteroProperty;
         Index                     node_dI;
         Index                     element_dI;
         
         if (nodeI == 1) {
         if( context->rank == 0 ) Journal_Printf( context->snacInfo,"%s: node 1 = %d\n",__func__,nodeI);
         for( node_dI = 0; node_dI < context->mesh->nodeLocalCount; node_dI++ ){
          _SnacHetero_node (phaseI, node_dI,context, heteroProperty );}
         }
         if (elementI == 1) 
         if( context->rank == 0 ) Journal_Printf( context->snacInfo,"%s: element 1 = %d\n",__func__,elementI);
         for( element_dI = 0; element_dI < context->mesh->elementLocalCount; element_dI++ )
          _SnacHetero_element (phaseI, element_dI,context, heteroProperty );

}


void getHetero_Dictionary_Entry_Values( Dictionary*     heteroDict , int hetero_I,int* pnodeI, int* pelementI, void* _context, void* _heteroProperty)

{
        Snac_Hetero*    heteroProperty  = (Snac_Hetero*)_heteroProperty;
        Snac_Context*   context  = (Snac_Context*)_context;
  //      Dictionary*     heteroDict = (Dictionary*)_heteroDict;
        char*           heterogeneity_geom;
        Dictionary*     nodeVCDict ;
        Dictionary*     elementVCDict;
        Dictionary_Entry_Value*         element_VC = NULL;
        Dictionary_Entry_Value*         node_VC    = NULL;
                heteroProperty[hetero_I].IsNodeVC = 0;
                heteroProperty[hetero_I].IsElementVC = 0;
//   geometrical information
        heterogeneity_geom = Dictionary_Entry_Value_AsString(
                Dictionary_GetDefault( heteroDict, "geometry", Dictionary_Entry_Value_FromString( "FrontLimit" ) ) );
        if( !strcmp( heterogeneity_geom, "Dyke" ) )
                heteroProperty[hetero_I].geom = SnacDyke;
        else if( !strcmp( heterogeneity_geom, "Sphere" ) )
                heteroProperty[hetero_I].geom = SnacSphere;
        else if( !strcmp( heterogeneity_geom, "Cylinder" ) )
                heteroProperty[hetero_I].geom = SnacCylinder;
        else if( !strcmp( heterogeneity_geom, "Cylinder_V" ) )
                heteroProperty[hetero_I].geom = SnacCylinder_V;
        else if( !strcmp( heterogeneity_geom, "Cylinder_H" ) )
                heteroProperty[hetero_I].geom = SnacCylinder_H;
        else if( !strcmp( heterogeneity_geom, "UpperLimit" ))
                heteroProperty[hetero_I].geom = SnacUpperLimit;
        else if( !strcmp( heterogeneity_geom, "RightLimit" ))
                heteroProperty[hetero_I].geom = SnacRightLimit;
        else if( !strcmp( heterogeneity_geom, "FrontLimit" ))
                heteroProperty[hetero_I].geom = SnacFrontLimit;

        heteroProperty[hetero_I].a_shape   = Dictionary_Entry_Value_AsDouble(
                Dictionary_GetDefault( heteroDict, "a_shape", Dictionary_Entry_Value_FromDouble( 1.0f ) ) );
        heteroProperty[hetero_I].b_shape    = Dictionary_Entry_Value_AsDouble(
                Dictionary_GetDefault( heteroDict, "b_shape", Dictionary_Entry_Value_FromDouble( 0.0f ) ) );
        heteroProperty[hetero_I].c_shape   = Dictionary_Entry_Value_AsDouble(
                Dictionary_GetDefault( heteroDict, "c_shape", Dictionary_Entry_Value_FromDouble( 0.0f ) ) );
        heteroProperty[hetero_I].d_shape   = Dictionary_Entry_Value_AsDouble(
                Dictionary_GetDefault( heteroDict, "d_shape", Dictionary_Entry_Value_FromDouble( 0.0f ) ) );
        heteroProperty[hetero_I].e_shape = Dictionary_Entry_Value_AsDouble(
                Dictionary_GetDefault( heteroDict, "e_shape", Dictionary_Entry_Value_FromDouble( 0.0f ) ) );

        node_VC = Dictionary_Get( heteroDict, "nodeVCs" );
        if (node_VC ) {
         	nodeVCDict  = Dictionary_Entry_Value_AsDictionary(node_VC);
         	heteroProperty[hetero_I].nodeVC = CompositeVC_New( "NodeIC", context->variable_Register, context->condFunc_Register, nodeVCDict, context->mesh );
         	Build( heteroProperty[hetero_I].nodeVC, 0, False ); 
        	*pnodeI=1;
                heteroProperty[hetero_I].IsNodeVC = 1;
                 if( context->rank == 0 ) Journal_Printf( context->snacInfo,"found nodeVCs int eh dictionary");
         }
         element_VC = Dictionary_Get( heteroDict, "elementVCs" );
         if (element_VC) {
         	elementVCDict  = Dictionary_Entry_Value_AsDictionary(element_VC);
         	heteroProperty[hetero_I].elementVC = CompositeVC_New( "ElementIC", context->variable_Register, context->condFunc_Register, elementVCDict, context->mesh );//}
         	Build( heteroProperty[hetero_I].elementVC, 0, False );
         	*pelementI=1;
                heteroProperty[hetero_I].IsElementVC = 1;
         }
}


        void  printHetero_Properties( void* _heteroproperty, int hetero_I, Stream* stream ) {
        Snac_Hetero* heteroProperty = (Snac_Hetero*)_heteroproperty;
        Journal_Printf( stream, "You inserted a heterogeneity of geometry  %d\n",heteroProperty[hetero_I].geom);
         if(heteroProperty[hetero_I].IsElementVC) VariableCondition_PrintConcise( heteroProperty[hetero_I].elementVC, stream );
         if(heteroProperty[hetero_I].IsNodeVC) {
        Journal_Printf( stream, "You shouldn't be in there");
         VariableCondition_PrintConcise( heteroProperty[hetero_I].nodeVC, stream );}
        Journal_Printf( stream, "with parameters:\n");
        Journal_Printf( stream, "\t a_shape   = %g\n",heteroProperty[hetero_I].a_shape );
        Journal_Printf( stream, "\t b_shape   = %g\n",heteroProperty[hetero_I].b_shape );
        Journal_Printf( stream, "\t c_shape   = %g\n",heteroProperty[hetero_I].c_shape );
        Journal_Printf( stream, "\t d_shape   = %g\n",heteroProperty[hetero_I].d_shape );
        Journal_Printf( stream, "\t e_shape   = %g\n",heteroProperty[hetero_I].e_shape );

}

