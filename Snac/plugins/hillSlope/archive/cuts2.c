    if(fractionWeakPoints>=0.0 && fractionWeakPoints<=1.0) {
	unsigned int		rngSeed = contextExt->rngSeed;
	double			x_subdomainFraction = contextExt->xSubDomainFraction;
	double			y_subdomainFraction = contextExt->ySubDomainFraction;
	double			z_subdomainFraction = contextExt->zSubDomainFraction;
	unsigned int		numberSubDomainPoints,numberWeakPoints;
	unsigned int		*shuffleIndexListPtr, *randomNumberListPtr;
	int			subdomain_I_element_range,subdomain_J_element_range,subdomain_K_element_range;	    


	/*  Report HillSlope plugin variables picked up (?) from xml parameter file */
	Journal_Printf( context->snacInfo, "\n\tSlope angle = %g degrees\n", slopeAngle/(M_PI/180.0) );
	Journal_Printf( context->snacInfo, "\tRNG seed = %u\n", rngSeed );
	Journal_Printf( context->snacInfo, "\tFraction of weak points = %g\n", fractionWeakPoints );
	Journal_Printf( context->snacInfo, "\tx subdomain fraction = %g\n", x_subdomainFraction );
	Journal_Printf( context->snacInfo, "\ty subdomain fraction = %g\n", y_subdomainFraction );
	Journal_Printf( context->snacInfo, "\tz subdomain fraction = %g\n", z_subdomainFraction );

	/*
	 *  Define portion of mesh (cells) that need weak point "seeding"
	 *    - lots of sloppy float/int casting back and forth here
	 */
	subdomain_I_element_range = (int)(full_I_element_range*x_subdomainFraction);
	subdomain_J_element_range = (int)(full_J_element_range*y_subdomainFraction);
	subdomain_K_element_range = (int)(full_K_element_range*z_subdomainFraction); 

	Journal_Printf( context->snacInfo, "\tx subdomain element range = %d/%d\n", subdomain_I_element_range,full_I_element_range );
	Journal_Printf( context->snacInfo, "\ty subdomain element range = %d/%d\n", subdomain_J_element_range,full_J_element_range );
	Journal_Printf( context->snacInfo, "\tz subdomain element range = %d/%d\n\n", subdomain_K_element_range,full_K_element_range );

	numberSubDomainPoints = subdomain_I_element_range*subdomain_J_element_range*subdomain_K_element_range;
	numberWeakPoints = (unsigned int)((float)numberSubDomainPoints*fractionWeakPoints);
	Journal_Printf(context->snacInfo,  "Number of weak points = %u/%u\n", numberWeakPoints,numberSubDomainPoints);
	
	/*
	 *  Seed RNG (random number generator) using seed value from input xml file
	 */
	srand( rngSeed );
	shuffleIndexListPtr=(unsigned int *)malloc((size_t)(numberSubDomainPoints*sizeof(unsigned int)));
	randomNumberListPtr=(unsigned int *)malloc((size_t)(numberSubDomainPoints*sizeof(unsigned int)));
	/*
	 *  Create (1) ordered sequence of element indices, (2) parallel list of random variates
	 */
	index=0;
	for(index_I = 0; index_I < subdomain_I_element_range; index_I++) {
	    for(index_J = 0; index_J < subdomain_J_element_range; index_J++) {
		for(index_K = 0; index_K < subdomain_K_element_range; index_K++) {
		    /*
		     *  Work out the element index from the i,j,k and the required location of the subdomain
		     */
		    element_gI = index_I+(full_I_element_range-subdomain_I_element_range)/2
			+ full_I_element_range*(full_J_element_range-1-index_J)
			+ full_I_element_range*full_J_element_range*(index_K+(full_K_element_range-subdomain_K_element_range)/2);
		    element_lI = Mesh_ElementMapGlobalToLocal( mesh, element_gI );
		    
		    shuffleIndexListPtr[index]=element_lI;
		    randomNumberListPtr[index]=rand();
		    index++;
		    if(index-1>numberSubDomainPoints) break;
		}
		if(index>0 && index-1>numberSubDomainPoints) break;
	    }
	    if(index>0 && index-1>numberSubDomainPoints) {fprintf(stderr, "We miscalculated the subdomain index\n"); break;}
	}
	/*
	 *  Sort the list of sub-domain local (ie global set pared down for this CPU) indices according to the random numbers
	 *    - i.e., randomize the index list
	 */
	ShellSort( randomNumberListPtr, shuffleIndexListPtr, numberSubDomainPoints );
#ifdef DEBUG
	for(index = 0; index < numberSubDomainPoints; index++) {
	    Journal_Printf( context->snacInfo, "\t Subdomain point #%d = %d, RNG var=%d\n", 
			    index, shuffleIndexListPtr[index],randomNumberListPtr[index] );
	}
#endif
	/*
	 *  Now the subdomain index list is shuffled and the random number list is ordered
	 */


	/*
	 *  Loop over the shuffled subdomain index list BUT only up to the number of weak points within it
	 *   - i.e., subset the subdomain and allocate weak points to only a random fraction of them
	 */
	for(index = 0; index < numberWeakPoints; index++) {
	    element_lI = shuffleIndexListPtr[index];

#ifdef DEBUG
	    Journal_Printf(context->snacInfo,  "Shuffled weak point # %d  -> %d \n", index,  element_lI);
#endif
	    
	    /*
	     *  At each point, force low cohesion by imposing a degree of plastic strain
	     */
	    if(element_lI < mesh->elementLocalCount) {
		Snac_Element		*element;
		Snac_Material	    	*material;
		SnacPlastic_Element	*plasticElement;
		Tetrahedra_Index	tetra_I;
		
		element = Snac_Element_At( context, element_lI );
		material = &context->materialProperty[element->material_I];
		plasticElement = ExtensionManager_Get(  mesh->elementExtensionMgr, element, SnacPlastic_ElementHandle );
		for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
		    plasticElement->plasticStrain[tetra_I] = PlasticStrainFromCohesion(material,(double)contextExt->weakPointCohesion);
#ifdef DEBUG		    
		    if(tetra_I==0) Journal_Printf( context->snacInfo,"timeStep=%d et=%d %d  plasticE:  %e -> %e\n",context->timeStep, element_lI, tetra_I, material->plstrain[1], plasticElement->plasticStrain[tetra_I] );
#endif

		}
	    } // End if
	    
	} // End for
	/*
	 *  Free up allocated memory for shuffling arrays
	 */
	free(shuffleIndexListPtr);
	free(randomNumberListPtr);
    } // End if (check fraction of weak points is in valid range [0,1]

    /*
     *  Trigger point assignment
     */
    if(contextExt->triggerPointCohesion>=0.0) {
	ijk[0]=(int)(full_I_element_range*contextExt->xTriggerPointFraction);
	ijk[1]=(int)(full_J_element_range*contextExt->yTriggerPointFraction);
	ijk[1]=	full_J_element_range-1-ijk[1];
	ijk[2]=(int)(full_K_element_range*contextExt->zTriggerPointFraction);
	/*
	 *  Calculate "global" (across all processors) element index and then find local equivalent (this CPU)
	 */
	element_gI = ijk[0] + full_I_element_range*ijk[1] + full_I_element_range*full_J_element_range*ijk[2];
	element_lI = Mesh_ElementMapGlobalToLocal( mesh, element_gI );
	
#ifdef DEBUG
	Journal_Printf(context->snacInfo,  "Trigger point:  ijk = %d,%d,%d  ->  %d  -> %d  <?  %d\n", ijk[0], ijk[1], ijk[2],  element_gI, element_lI, mesh->elementLocalCount);
#endif
	/*
	 *  At trigger point, force low cohesion by imposing a degree of plastic strain
	 */
	if(element_lI < mesh->elementLocalCount) {
	    Snac_Element		*element;
	    Snac_Material	    	*material;
	    SnacPlastic_Element		*plasticElement;
	    Tetrahedra_Index		tetra_I;
	
	    element = Snac_Element_At( context, element_lI );
	    material = &context->materialProperty[element->material_I];
	    plasticElement = ExtensionManager_Get( mesh->elementExtensionMgr,element,SnacPlastic_ElementHandle );
	    /*  Force each of 5*2 tetrahedra to have extra plastic strain to impose lower cohesion indirectly */
	    for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
		plasticElement->plasticStrain[tetra_I] =  PlasticStrainFromCohesion(material,(double)contextExt->triggerPointCohesion);
#ifdef DEBUG
		if(tetra_I==0) Journal_Printf(context->snacInfo, "timeStep=%d ijkt=%d %d %d %d  plasticE=%e\n", context->timeStep,ijk[0],ijk[1],ijk[2], tetra_I,  plasticElement->plasticStrain[tetra_I] );
#endif
	    }
	}
    } // End if






