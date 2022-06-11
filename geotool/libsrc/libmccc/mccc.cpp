/** \file mccc.cpp
 *  \brief Multi Channel Cross Correlation library
 *  \author Vera Miljanovic
 */

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_multifit.h>
#include <gsl/gsl_vector.h>

using namespace std;

#include "gobject++/GTimeSeries.h"

extern "C" {
#include "libstring.h"  /* for mallocWarn */
}

#include "mccc.h"


/* Macro whose purpose is the conversion of time t to specified members. In
 * the equations from VanDecar Crosson (1990) time units can be found,
 * however those units are actually discrete values, that is each of those
 * units represents the product of delta_t time, which represents the period
 * of selection: for every t t=i*delta_t holds, where i is an integer. This
 * macro calculates the i, for specified t and delta_t
 */
#define INDEX(t, delta_t) ((int) floor( ((t)/(delta_t)) + 0.5 ) )

/* Number of iterations which are used for applying the least squares method
 */
#define STEPS_COUNT 3


/* Static functions
 */
static double calculatePhi(const int m,
		    const double delta_t,
		    const double *const x_i,
		    const double *const x_j,
		    const double T,
		    const double t_0,
		    const double tp_i,
		    const double tp_j,
		    const double tau);
static void calculateT(const int n,
		const int m,
		const double delta_t,
		const double *const *const x,
		const double T,
		const double t_0,
		const double *const tp,
		const double tau_range,
		const int ncf,
		const int *const cf,
		const Method method,
		double *const t,
		double *const max_coefs,
		double ** ccmatrix,
		bool ** ccmatrix_mask);
static float *getData(GTimeSeries *ts, int *npts);


/**
 * Compute multi channel cross correlation for waveforms.
 * @param[in] ts_list List of time series
 * @param[in] n_ts_list Number of items in list
 * @param[in] method Least Square method to use
 * @param[in] lag The infamous lag
 * @param[out] ccmatrix (Empty) Preallocated cross correlation matrix (n_ts_list x n_ts_list) which will be determined, if NULL return nothing
 * @param(out) ccmatrix_mask (Empty) Preallocated cross correlation matrix mask  (n x n); non-zero if element is determined, if NULL return nothing
 * @return Returns the a list of waveforms, returns NULL on error
 */
GTimeSeries ** multiChannelCCTS(GTimeSeries ** ts_list,
				     int n_ts_list,
				     Method method,
				     double lag,
				     double ** ccmatrix,
				     bool ** ccmatrix_mask)
{

  int nsig;
  float *sig;
  GTimeSeries **mccc_list = NULL;
  int n = n_ts_list;
  double delta_t;
  double *x_buffer;
  double **x;
  double T;
  double t_0 = 1.001;
  double *tp = NULL;
  double tau_range_max = 1.0;
  int ncf = 3;
  int *cf = NULL;
  double *t;
  double *max_coefs;
  double tbeg, tdel, calib, calper;

  /* Assume equal number of signals in all waveforms */
  int m;
  if(ts_list[0]->size() > 1) {
    sig = getData(ts_list[0], &m);
    Free(sig);
  }
  else {
    m = ts_list[0]->length();
  }
  
  /* Fill input matrix with signals */
  x_buffer = new double[n * m];
  x = new double *[n];

  for (int i = 0; i < n; i++) {

    x[i] = x_buffer + i * m;

    /* Get signals */
    if(ts_list[i]->size() > 1) {
      sig = getData(ts_list[i], &nsig);
    }
    else {
      nsig = ts_list[i]->length();
      sig = (float *)mallocWarn(nsig * sizeof(float));
      memcpy(sig, ts_list[i]->segment(0)->data, nsig * sizeof(float));
    }

    /* Check number of signals */
    if (nsig != m) {
      logErrorMsg(LOG_WARNING,
                  "multiChannelCCTS: waveforms not of equal length");
      if(ts_list[i]->size() > 1) free(sig);
      return NULL;
    }

    for (int j = 0; j < m; j++) {
      x[i][j] = (double)sig[j];
    }
  
    if(sig) free(sig);

  }

  /* Assume all waveforms have the same sample rate and
   * length as the first one
   */
  delta_t = ts_list[0]->segment(0)->tdel();
  T = ts_list[0]->duration();

  /* Preliminary arrival times */
  tp = new double[n];
  for (int i = 0; i < n; i++) {

    tp[i] = 0.0 + lag;
    //tp[i] = ts_list[i]->segment(0)->tbeg() + lag;

  }
  
  /* Roughness factors */
  cf = new int[ncf];
  cf[0] = 10;
  cf[1] = 5;
  cf[2] = 1;

  /* Estimated arrival times returned */
  t = new double[n];

  /* Max coefs returned */
  max_coefs = new double[n];
  
  /* Sanity check */
  if(n <= 2) {
    cerr << "multiChannelCCTS failed: n = " << n << endl;
  }
  else if(m <= 0) {
    cerr << "multiChannelCCTS failed: m = " << m << endl;
  }
  else if(delta_t <= 0) {
    cerr << "multiChannelCCTS failed: delta_t = " << delta_t << endl;
  }
  else if(T <= 0) {
    cerr << "multiChannelCCTS failed: T = " << T << endl;
  }
  else if(t_0 <= 0) {
    cerr << "multiChannelCCTS failed: t_0 = " << t_0 << endl;
  }
  else if(tau_range_max <= 0) {
    cerr << "multiChannelCCTS failed: tau_range_max = " << tau_range_max << endl;
  }
  else if(ncf <= 0) {
    cerr << "multiChannelCCTS failed: ncf = " << ncf << endl;
  }
  else
  {
    /* Work, work */
    calculateT(n, m, delta_t, x, T, t_0, tp, tau_range_max, ncf, cf, method,
	     t, max_coefs, ccmatrix, ccmatrix_mask);

    /* Create new multi channel cross correlation time series list with
       estimated arrival times */
    mccc_list = new GTimeSeries *[n];
    calib = 1.;
    for (int i = 0; i < n; i++) {
 
      calper = ts_list[i]->segment(0)->calper();
      tdel = ts_list[i]->segment(0)->tdel();
      tbeg = ts_list[i]->segment(0)->tbeg() + t[i];

      mccc_list[i] = new GTimeSeries(new GSegment(x[i], m,
					      tbeg, tdel, calib, calper));
      mccc_list[i]->putValue("max_coef", max_coefs[i]);

    }   
  }
    
  /* Clean up */
  delete [] x_buffer;
  delete [] x;
  delete [] tp;
  delete [] cf;
  delete [] t;
  delete [] max_coefs;

  /* Return new list */
  return mccc_list;

}

/* Function implements equation 
 *                    delta t   T/delta t
 * (1) phi_ij(tau) =  --------  SUM      x_i(tp_i + t_0 + k delta t + tau)
 *                       t       k=1 
 *                                          *x_j(tp_j + t_0 + k delta t)
 * from VD-C. The arguments of the
 * function are, apart from the first, values which can be found on the
 * right hand side of the equation:
 * @param(in) m length (number of elements) of signals in x_i and x_j
 * @param(in) delta_t period of selection (1/samplerate)
 * @param(in) x_i array (dimension m) with values of signal x_i
 * @param(in) x_j array (dimension m) with values of signal x_j
 * @param(in) T length of correlation "window"
 * @param(in) t_0 time between preliminary arrival time estimate and when
 *            correlation window begins
 * @param(in) tp_i i-th trace's preliminary arrival time estimate
 * @param(in) tp_j j-th trace's preliminary arrival time estimate
 * @param(in) tau lag time relative to preliminary arrival time estimates
 * @return phi
*/
static double calculatePhi(const int m,
			   const double delta_t,
			   const double *const x_i,
			   const double *const x_j,
			   const double T,
			   const double t_0,
			   const double tp_i,
			   const double tp_j,
			   const double tau)
{
  /* Equation (1). */

  /* Evaluates the number of elements of arrays x_i and x_j which are
   * needed to calculate the sum of their products (e.g. this is practically
   * T/delta_t, which is specified as the upper limit of sum in equation (1)
   */
  int n = INDEX(T, delta_t);
  //printf("delta_t = %f, T = %f n=%d m=%d\n", delta_t, T, n, m);
  if(n >= m) {
    cerr << "calculatePhi failed: n >= m" << endl;
    return 0.;
  }
  
  /* The evaluation of index in field x_i where the elements are summed,
   * and checks whether the indexes of all elements which will be taken into
   * account are within the valid range of index for field x_i 
   */
  int i_lo = INDEX(tp_i + t_0 + tau, delta_t);
  //printf("i_lo = %d, tpi = %f, t_0 = %f , tau = %f \n", i_lo, tp_i, t_0, tau);

  /* The evaluation of index in field x_j where the elements are summed,
   * and checks whether the indexes of all elements which will be taken into
   * account are within the valid range of index for field x_j
   */
  int j_lo = INDEX(tp_j + t_0, delta_t);

  /* Determine maximum number of elements in arrays x_i and X_j which
   * can be used to calculate the sum of their products
   */
  int k_max;
  if ( (n - i_lo) < (n-j_lo) ) {
    k_max = n - i_lo;
  }
  else {
    k_max = n - j_lo;
  }
  if(i_lo < 0 || i_lo + k_max >= m) {
    cerr << "calculatePhi failed: invalid i_lo" << endl;
    return 0.;
  }
  if(j_lo < 0 || j_lo + k_max >= m) {
    cerr << "calculatePhi failed: invalid j_lo" << endl;
    return 0.;
  }

  /* Evaluation of sum of products of the elements of arrays x_i and x_j,
   * followed by division of the number of elements which have entered the sum
   * (e.g. multiplied by delta_t/T), which then forms the required value phi.
   */
  double phi = 0;

  //  printf("before phi loop n=%d\n", n);

  for (int k = 0; k < k_max; ++k) {
    phi += x_i[i_lo + k] * x_j[j_lo + k];
    // printf("phi loop k=%d..%d phi=%f\n", k, n, phi);
  }
  // phi /= n;

  return phi;

}

/* This function implements the calculation of value t found in equation 
 *               1  i-1                   n 
 * (6)t^{est}_i = -(-SUM \Delta t_{ji} + SUM \Delta t_{ij}) from VD-C
 *               n  j=1                  j=i+1     
 * using values for dt calculated according to 
 * (2)\Delta t_{ij} = tp_i - tp_j - \tau_max
 * in which the values for tau_max are used again explained in the text between (1) and (2)
 * by calculating individual cross-correlation. The arguments are:
 * @param(in) n the number of signals
 * @param(in) m length (number of elements) of signals
 * @param(in) T length of correlation "window"
 * @param(in) t_0 time between preliminary arrival time estimate and when
 *            correlation window begins
 * @param(in) tp trace preliminary arrival time estimate
 * @param(in) delta_t period of selection (1/samplerate)
 * @param(in) x matrix (dimension n*m) with the elements from the signal
 * @param(in) tau_range the number which specifies the range of value tau
 *                  for calculating the maximum of cross-correlation (e.g.
 *                  value 1 would mean that tau is in the range of [-1,1],
 *                  depicted in figure 3 of VD-C paper
 * @param(in) ncf the number of "rough factors" used for calculation of
 *            cross-correlation maximum
 * @param(in) cf array (length ncf) with values of "the factor of roughness"
 *           used for calculation of maximum of cross-correlation, that is
 *           with values designated in the bottom of page 154 ("m" has nothing
 *           to do with the number "m" which is used in this code as the size
 *           of the array - when there are many values in a certain
 *           numerical/algorithmic procedure, it is hard to find good marks
 *           for each); these factors need to be such that every factor is the
 *           denominator of the previous factor
 * @param(in) t array (length n) with the values found on the left hand side of
 *          (6)
 * @param(out) max_coefs (Empty) Preallocated array (length n) of max coefs which will be determined
 * @param(out) ccmatrix (Empty) Preallocated cross correlation matrix (n x n) which will be determined, if NULL return nothing
 * @param(out) ccmatrix_mask (Empty) Preallocated cross correlation matrix mask  (n x n); true if element is determined, if NULL return nothing
 * 
 */

static void calculateT(const int n,
		       const int m,
		       const double delta_t,
		       const double *const *const x,
		       const double T,
		       const double t_0,
		       const double *const tp,
		       const double tau_range,
		       const int ncf,
		       const int *const cf,
		       const Method method,
		       double *const t,
		       double *const max_coefs,
		       double ** ccmatrix,
		       bool ** ccmatrix_mask)
{
  /* Allocation of memory for matrix of value dt */
  double *dt_buffer;
  double **dt;
  dt_buffer = new double[n * n];
  dt = new double *[n];
  for (int i = 0; i < n; ++i) dt[i] = dt_buffer + i * n;

  /* Allocation of memory for matrix of value coefs */
  double *coefs_buffer;
  double **coefs;
  coefs_buffer = new double[n * m];
  coefs = new double *[n];
  for (int i = 0; i < n; ++i) coefs[i] = coefs_buffer + i * m;

  /* Calculation of values dt according to 
   * (5) t_i-t_j=\Delta t_{ij}, i=1,...,n-1, j=i+1,...,n
   *   with \sum_{i=1}^n t_i = 0
   * for each pair of signals x[i] and x[j] where i<j
   */
  for (int i = 0; i < n; ++i) {

    for (int j = i + 1; j < n; ++j) {
      
      /* Value of tau (tau for which the cross-correlation function
       * has maximum) is initialized to 0, and then we calculate
       * the cross-correlation function for such tau */
      double tau_max = 0;
      double phi_max = calculatePhi(m, delta_t, x[i], x[j], T, t_0, tp[i],
				    tp[j], tau_max);

      /* Evaluation of the cross-correlation function for each
       * tau from the specified range. In case there is a value tau
       * where the value of the cross-correlation function is higher
       * than the maximum which has been calculated so far, the
       * value will be saved, together with the value of the max
       * cross-correlation function. At the end of each loop
       * in the next interval the value for tau which are of interest,
       * in each step some members are skipped according to the
       * "factor of coarseness"; also, after each loop, the values
       * of tau for the given interval which are of interest are
       * shrinking so that it encompasses those members which are specified
       * by the "factor of coarseness". The implementation
       * is explained in the text between formulas (1) and (2) 
       */ 
         
      int steps = INDEX(tau_range, delta_t);
      for (int k = 0; k < ncf; ++k) {

	for (int l = -steps; l <= steps; l += cf[k]) {

	  /* The value for tau_max is not necessary to be taken into account */
	  if (l == 0)
	    continue;

	  /* Evaluation of the next value for tau; if that value is
	   * not within the permitted range for tau, it is skipped */
	  double tau = tau_max + l * delta_t;
	  if (tau < -tau_range || tau > tau_range)
	    continue;
               
	  /* Evaluation according to (1) of the cross-correlation function
	   * for current tau, in order to halt the maximum of 
           * cross-correlation function, together with the desired tau value 
           */ 
	  double phi = calculatePhi(m, delta_t, x[i], x[j], T, t_0, tp[i],
				    tp[j], tau);
	  if (phi_max < phi) {
	    tau_max = tau;
	    phi_max = phi;
	  }

	}

	/* For next iteration the number of members from the first 
	 * and second side of current tau_max should be taken into
	 * account 
         */
	steps = cf[k];

      }

      /* Equation(2). */

      /* Evaluation of dt[i][j] value according to (2) */
      dt[i][j] = tp[i] - tp[j] - tau_max;
      
      /* Evaluation of coefs[i][j] value according to (3) */
      double sigma_i = 0.0;
      double sigma_j = 0.0;
  
      
      for (int k = 0; k < m; k++) {
	sigma_i += pow(x[i][k], 2);
	sigma_j += pow(x[j][k], 2);
	//printf("sigma_i[%d] = %f\n", k, sigma_i);
	//printf("sigma_j[%d] = %f\n", k, sigma_j);
      }
      sigma_i = sqrt(sigma_i);
      sigma_j = sqrt(sigma_j);
      
      coefs[i][j] = calculatePhi(m, delta_t, x[i], x[j], T, t_0, tp[i],
				 tp[j], tau_max) / (sigma_i*sigma_j);


      }  

  }
  /* Equations (6) & (7), where (7)res_ij = \Delta t_{ij} - (t_i - t_j_)
   */

  /* Allocation of space for matrix (we denote it matrix A) which can be found
   * on the left-hand side of the matrix equation on the p.157 of VanDecar
   * Crosson (and on the right-hand side of the matrix equation on the
   * p.8276 of Shearer)as well as the vector (which we denote vector y) 
   * which is on the right-hand side of the same equation for VD-C (i.e.
   * left-hand side of Shearer); size of the allocated space for this matrix
   * and vector is determined by the method of forming the matrix equation
   * which will be used
   */ 
  gsl_matrix *A=NULL;
  gsl_vector *y=NULL;
  switch (method) {

  case VANDECAR_CROSSON:
    A = gsl_matrix_alloc(n * (n - 1) / 2 + 1, n);
    y = gsl_vector_alloc(n * (n - 1) / 2 + 1);
    break;

  case SHEARER:
    A = gsl_matrix_alloc(n * (n - 1) / 2 + n, n);
    y = gsl_vector_alloc(n * (n - 1) / 2 + n);
    break;

  }

  /* Initialization of the elements of the matrix A, e.g. vector y, again for
   * both methods */

  gsl_matrix_set_all(A, 0);
  int index = 0;
  for (int i = 0; i < n; ++i) {
    
    for (int j = i + 1; j < n; ++j, ++index) {
      gsl_matrix_set(A, index, i, 1);
      gsl_matrix_set(A, index, j, -1);
      gsl_vector_set(y, index, dt[i][j]);
    }

  }
  
// is index used correctly below?
  switch (method) {

  case VANDECAR_CROSSON:
    for (int i = 0; i < n; ++i) gsl_matrix_set(A, index, i, 1);
    gsl_vector_set(y, index, 0);
    break;

  case SHEARER:
    for (int i = 0; i < n; ++i, ++index) {
      gsl_matrix_set(A, index, i, 1);
      gsl_vector_set(y, index, tp[i]);
    }
    break;

  }

  /* Allocation of space for vector (we denote it by t_est) which can be
   * found in the same matrix equations mentioned above (for VD-C it is on
   * the left-hand-side, and for Shearer on the right-hand side).
   * We also define values which will be necessary for GSL library in order
   * to do a procedure of minimization by least squares method.
   */

  gsl_vector *t_est = gsl_vector_alloc(n); 
  gsl_matrix *covariance = gsl_matrix_alloc(n, n);
  double chi_square;

  /* Allocation of space for vector of weighted cofficients (we denote it by
   * w) which will be used for minimization by least squares method; the
   * values of these coefficients are initialized; and again for both methods
   */ 
  gsl_vector *w=NULL;

  switch (method) {

  case VANDECAR_CROSSON:
    w = gsl_vector_alloc(n * (n - 1) / 2 + 1);
    gsl_vector_set_all(w, 1);
    break;

  case SHEARER:
    w = gsl_vector_alloc(n * (n - 1) / 2 + n);
    for (int i = 0; i < n * (n - 1) / 2; ++i) gsl_vector_set(w, i, 1);
    for (int i = n * (n - 1) / 2; i < n * (n - 1) / 2 + n; ++i) {
      gsl_vector_set(w, i, 0.5);
    }
    break;

  }

  //  /*  */
  //gsl_matrix *W;

  //  switch (method) {

  //case VANDECAR_CROSSON:
  // W = gsl_matrix_alloc(n * (n - 1) / 2 + 1, n * (n - 1) / 2 + 1);
  //   for (int j = i + 1; j < n; ++j, ++index) {
  //  gsl_matrix_set(W, index, j, 1);  
  //                  t_est.append( matrixmultiply( matrixmultiply( matrixmultiply( inv( matrixmultiply( matrixmultiply(A.transpose(), W), A) ) , A.transpose() ) , W ) , lags ) )

  //    gsl_vector_set_all(w, 1);
  // break;

  //case SHEARER:

  // break;

  // }

      

  /* Allocation of working space for GSL library; again the size of this
   * space is determined by one of the methods of forming the matrix
   * equation
   */

  gsl_multifit_linear_workspace *workspace=NULL;

  switch (method) {

  case VANDECAR_CROSSON:
    workspace = gsl_multifit_linear_alloc(n * (n - 1) / 2 + 1, n);
    break;
    
  case SHEARER:
    workspace = gsl_multifit_linear_alloc(n * (n - 1) / 2 + n, n);
    break;

  }

  /* Initial minimization is done by the least squares method */
  gsl_multifit_wlinear(A, w, y, t_est, covariance, &chi_square,
		       workspace);

  /* Iterative minimization is done by the least squares method given number
   * of times */
  for (int step = 0; step < STEPS_COUNT; ++step) {

    /* */
    index = 0;
    for (int i = 0; i < n; ++i) {
      for (int j = i + 1; j < n; ++j, ++index) {
	double res = dt[i][j] - (gsl_vector_get(t_est, i) -
				 gsl_vector_get(t_est, j));
	if (res > 0.5) gsl_vector_set(w, index, 0);
	else if (res > 0.2) gsl_vector_set(w, index, 0.5);
	else if (res > 0.1) gsl_vector_set(w, index, 0.75);
	else gsl_vector_set(w, index, 1);
      }

      /* Again we start the minimization by the least squares method */
      gsl_multifit_wlinear(A, w, y, t_est, covariance, &chi_square,
			   workspace);
    }

  }

  /* Working space for GSL library is freed in order to do the minimization
   * by least squares method */
  gsl_multifit_linear_free(workspace);
    
  for (int i = 0; i < n; ++i) t[i] = gsl_vector_get(t_est, i);

  /* Find max coef in matrix of calculated coefs
   * NB: Not all cells in the matrix are calculated because that would
   *     be duplicate work (see earlier loop i=0..n, j=i+1..n),
   *     which is why they are skipped here as well
   */
  for (int i = 0; i < n; ++i) {
    /* Initially assume small wrong value */
    max_coefs[i] = -999.9;
    /* Check coefs in matrix calculated earlier */
    for (int j = i + 1; j < n; ++j) {
      //printf("FIRST LOOP coefs[%d][%d]: %f\n", i, j, coefs[i][j]);
      if (coefs[i][j] > max_coefs[i]) max_coefs[i] = coefs[i][j];
    }
    for (int j = i - 1; j >= 0; --j) {
      //printf("SECOND LOOP coefs[%d][%d]: %f\n", j, i, coefs[j][i]);
      if (coefs[j][i] > max_coefs[i]) max_coefs[i] = coefs[j][i];
    }
    //printf("max_coefs[%d]: %f\n", i, max_coefs[i]);
  }
 
  /* Fill in cross correlation matrix to be returned */
  if (ccmatrix_mask != NULL) {
    for (int i = 0; i < n; ++i) {
      for (int j = 0; j < n; ++j) {
	ccmatrix_mask[i][j] = false;
      }
    }
  }
  if (ccmatrix != NULL) {
    for (int i = 0; i < n; ++i) {
      for (int j = i + 1; j < n; ++j) {
	ccmatrix[i][j] = coefs[i][j];
	if (ccmatrix_mask != NULL) ccmatrix_mask[i][j] = true;   
      }
    }    
  }

  /* The whole space used by GSL libraries is freed */ 
  gsl_matrix_free(A);
  gsl_vector_free(y);
  gsl_vector_free(t_est);
  gsl_matrix_free(covariance);
  gsl_vector_free(w);

   /* Allocated memory is freed */
  delete [] dt_buffer;
  delete [] dt;
  delete [] coefs_buffer;
  delete [] coefs;

}

static float * getData(GTimeSeries *ts, int *npts)
{
	int i, j, n;
	double tdel = ts->segment(0)->tdel();
	float *r;

	n = 0;
	for(j = 0; j < ts->size(); j++) {
	    if(j  > 0) {
		double gap = ts->segment(j)->tbeg() - ts->segment(j-1)->tend();
		int ngap = (int)(gap/tdel + .5) - 1;
		if(ngap > 0) {
		    n += ngap;
		}
	    }
	    n += ts->segment(j)->length();
	}
	r = (float *)mallocWarn(n*sizeof(float));
	n = 0;
	for(j = 0; j < ts->size(); j++) {
	    if(j  > 0) {
		double gap = ts->segment(j)->tbeg() - ts->segment(j-1)->tend();
		int ngap = (int)(gap/tdel + .5) - 1;
		for(i = 0; i < ngap; i++) {
		    r[n++] = 0.;
		}
	    }
	    for(i = 0; i < ts->segment(j)->length(); i++) {
		r[n++] = ts->segment(j)->data[i];
	    }
	}
	*npts = n;
	return r;
}
