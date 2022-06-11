#include "config.h"
#include "cssio.h"

/**
 * Decompress compressed data. Data is returned as an array of long ints.
 * This is an 8-bit version.
 * compression technique
 * @param numin  The number of characters in the input character string.
 * @param in The input string of characters containing compressed data.
 * @param out The ouput array of long ints containing decompressed data.
 * @return The number of floating point words in out.
 */
int
longdcpress(long numin, const char *in, long *out)
{
  int isign=64, ioflow=128, i, j;
  int jsign, joflow, mask1, mask2;
  long  itemp;
  mask1 = 63;
  mask2 = 127;
  i = 0;
  j = 0;
  
  /*start of decoding */
  /* and for sign */
  
  for (i = 0; i < numin; ) {
    jsign = in[i] & isign;
    joflow = in[i] & ioflow;
    itemp = in[i] & mask1;

    for ( ; ; )  {
      i = i + 1; 
      if (joflow == 0) break;
      
      /* there is another byte in this sample */
      itemp = itemp << 7;
      joflow = in[i] & ioflow;
      itemp = itemp + (in[i] & mask2);
    }

    if (jsign != 0) itemp = -itemp;
    out[j] = itemp;
    j = j + 1;
  }

  /* have finished get number of output samples and return */
  
  return (j);
}


/**
 * Remove the first-difference on long data. Second differences can be
 * removed by calling this routine twice.
 * @param num The number of data values input.
 * @param in The input data array.
 * @param out The  output data array.
 */
void
rmfdif(int num, long *in, long *out)
{
    int i;
    out[0] = in[0];

    for (i = 1; i < num; i++)
      out[i] = out[i-1] + in[i];
}
