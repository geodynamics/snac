#include <gsl/gsl_math.h>
#include <gsl/gsl_eigen.h>

#define DIM 3

int
main (void)
{
	double data[] = { 1.0  , 1/2.0, 1/3.0,
			  1/2.0, 1/9.0, 1/4.0,
			  1/3.0, 1/4.0, 1/5.0 };

	gsl_matrix_view m 
		= gsl_matrix_view_array (data, DIM, DIM);

	gsl_vector *eval = gsl_vector_alloc (DIM);
	gsl_matrix *evec = gsl_matrix_alloc (DIM, DIM);

	gsl_eigen_symmv_workspace * w = 
		gsl_eigen_symmv_alloc (DIM);

  
	gsl_eigen_symmv (&m.matrix, eval, evec, w); 

	gsl_eigen_symmv_free (w);

	gsl_vector_free (eval);
	gsl_matrix_free (evec);
  
	return 0;
}
