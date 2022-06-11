/** \file mccc.h
 *  \brief Multi Channel Cross Correlation library header
 *  \author Vera Miljanovic
 */

#include <gobject++/GTimeSeries.h>

/** @defgroup libmccc library libmccc
 *  Multi-channel cross correlation technique from:
 * - VanDecar Crosson: "Determination of teleseismic relative phase arrival
 *  times using multi-channel cross-correlation and least squares", Bulletin
 *  of the Seismological Society of America, Vol.80, No.1, pp.150-169
 *  (February 1990).
 * - Shearer: "Improving local earthquake locations using the L_1 norm and
 *  waveform cross correlation: Application to the Whittier Narrows,
 *  California, afterschock sequence", Journal of Geophysical Research,
 *  vol.102, no.B4, pp. 8269-8283 (Apr. 1997).
 */ 

/* Data type Method */
enum Method {
  VANDECAR_CROSSON,
  SHEARER
};

/** @ingroup libmccc
 */
GTimeSeries ** multiChannelCCTS(GTimeSeries ** ts_list,
				int n_ts_list,
				Method method, 
				double lag, 
				double ** ccmatrix,
				bool ** ccmatrix_mask);
