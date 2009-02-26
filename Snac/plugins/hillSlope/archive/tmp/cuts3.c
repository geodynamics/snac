
		/* Calculate tetrahedra heat flux (1st pass)*/
		elementExt->heatFlux[tetra_I][0] = 1.0f * material->thermal_conduct * (
				contributingNodeExt[0]->temperature *
				element->tetra[tetra_I].surface[1].normal[0] *
				element->tetra[tetra_I].surface[1].area +
				contributingNodeExt[1]->temperature *
				element->tetra[tetra_I].surface[2].normal[0] *
				element->tetra[tetra_I].surface[2].area +
				contributingNodeExt[2]->temperature *
				element->tetra[tetra_I].surface[3].normal[0] *
				element->tetra[tetra_I].surface[3].area +
				contributingNodeExt[3]->temperature *
				element->tetra[tetra_I].surface[0].normal[0] *
				element->tetra[tetra_I].surface[0].area ) /
			3.0f / element->tetra[tetra_I].volume;
		elementExt->heatFlux[tetra_I][1] = 1.0f * material->thermal_conduct * (
				contributingNodeExt[0]->temperature *
				element->tetra[tetra_I].surface[1].normal[1] *
				element->tetra[tetra_I].surface[1].area +
				contributingNodeExt[1]->temperature *
				element->tetra[tetra_I].surface[2].normal[1] *
				element->tetra[tetra_I].surface[2].area +
				contributingNodeExt[2]->temperature *
				element->tetra[tetra_I].surface[3].normal[1] *
				element->tetra[tetra_I].surface[3].area +
				contributingNodeExt[3]->temperature *
				element->tetra[tetra_I].surface[0].normal[1] *
				element->tetra[tetra_I].surface[0].area ) /
			3.0f / element->tetra[tetra_I].volume;
		elementExt->heatFlux[tetra_I][2] = 1.0f * material->thermal_conduct * (
				contributingNodeExt[0]->temperature *
				element->tetra[tetra_I].surface[1].normal[2] *
				element->tetra[tetra_I].surface[1].area +
				contributingNodeExt[1]->temperature *
				element->tetra[tetra_I].surface[2].normal[2] *
				element->tetra[tetra_I].surface[2].area +
				contributingNodeExt[2]->temperature *
				element->tetra[tetra_I].surface[3].normal[2] *
				element->tetra[tetra_I].surface[3].area +
				contributingNodeExt[3]->temperature *
				element->tetra[tetra_I].surface[0].normal[2] *
				element->tetra[tetra_I].surface[0].area ) /
			3.0f / element->tetra[tetra_I].volume;

		/* thermal stress */
		if( context->computeThermalStress ) {
			temp = 0.25f * ( contributingNodeExt[0]->temperature + contributingNodeExt[1]->temperature + contributingNodeExt[2]->temperature + contributingNodeExt[3]->temperature );
			temp0 = 0.25f * ( contributingNodeExt[0]->temperature0 + contributingNodeExt[1]->temperature0 + contributingNodeExt[2]->temperature0 + contributingNodeExt[3]->temperature0 );
			/* Thermal stresses (alfa_v = 3.e-5 1/K) */
			/* elementExt->thermalStress[tetra_I] = -1.0f*alpha*bulkm*(context->timeStep==1?(1.0*(temp-1300.0)):(temp-temp0)); */
			elementExt->thermalStress[tetra_I] = -1.0f*alpha*bulkm*(temp-temp0);
			Journal_Firewall( (!isinf(elementExt->thermalStress[tetra_I]) && !isnan(elementExt->thermalStress[tetra_I])), " timeStep %d thermalStress for element %d tetra %d (T: %e %e) is inf or nan!!\n",context->timeStep,element_lI,tetra_I,temp,temp0);
		}
