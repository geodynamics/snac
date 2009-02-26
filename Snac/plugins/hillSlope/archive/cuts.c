	/*
	 *  Loop over total number of weak points and locate each one
	 */
	for(index_I = 0; index_I < numberWeakPoints; index_I++) {
	    /*
	     *  Select random i,j,k indices  
	     *    - converts int random number into double, scales, and casts back to int.  Ugh.
	     */ 
	    randomNumber = (double)rand()/((double)RAND_MAX+1.0);
	    ijk[0] = (int)((subdomain_I_element_range) * randomNumber);
 	    ijk[0] = ijk[0]+(full_I_element_range-subdomain_I_element_range)/2;

	    randomNumber = (double)rand()/((double)RAND_MAX+1.0);
	    ijk[1] = (int)((subdomain_J_element_range) * randomNumber);
 	    ijk[1] =  full_J_element_range-1-ijk[1];

	    randomNumber = (double)rand()/((double)RAND_MAX+1.0);
	    ijk[2] = (int)((subdomain_K_element_range) * randomNumber);
 	    ijk[2] = ijk[2]+(full_K_element_range-subdomain_K_element_range)/2;

	#ifdef DEBUG
	    if (imax<ijk[1]) imax=ijk[1];
	#endif
	    /*
	     *  Calculate "global" (across all processors) element index and then find local equivalent (this CPU)
	     */
	    element_gI = ijk[0] + full_I_element_range*ijk[1] 
		                + full_I_element_range*full_J_element_range*ijk[2];
	    element_lI = Mesh_ElementMapGlobalToLocal( mesh, element_gI );

	#ifdef DEBUG
	    Journal_Printf(context->snacInfo,  "Weak point:  ijk = %d,%d,%d  ->  %d  -> %d  <?  %d\n", 
			   ijk[0], ijk[1], ijk[2],  element_gI, element_lI, mesh->elementLocalCount);
	#endif
	    
	    /*
	     *  At each point, force low cohesion by imposing a degree of plastic strain
	     */
	    if(element_lI < mesh->elementLocalCount) {
		
		element = Snac_Element_At( context, element_lI );
		material = &context->materialProperty[element->material_I];
		plasticElement = ExtensionManager_Get(
						      mesh->elementExtensionMgr,
						      element,
						      SnacPlastic_ElementHandle );
		for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
		    plasticElement->plasticStrain[tetra_I] = contextExt->weakPointCohesion * material->plstrain[1];
#ifdef DEBUG		    
		    if(tetra_I==0) Journal_Printf(
						  context->snacInfo,
						  "timeStep=%d ijkt=%d %d %d %d  plasticE=%e\n",
						  context->timeStep,
						  ijk[0],
						  ijk[1],
						  ijk[2],
						  tetra_I,
						  plasticElement->plasticStrain[tetra_I] );
#endif

		}
	    } // End if
	    
	} // End for


