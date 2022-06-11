#include "config.h"
#include <stdio.h>
#include <math.h>

#include "libgplot.h"

void
mapalf(DrawStruct *d,
	int x, int y,
	float size, float angle,
	int coord_sys,
	char *s)
{
/*
 		draws alphanumeric characters.     17 July 83    wjm
         note --  pen may still be down when letter is finished.
 	 note --  x and y must be equally scaled in 'vplot' for characters
		  to have true shape.
 
 	x,y	    position (in 'inches') of lower lefthand corner of
 		    the symbol.   (#'s 1-16 are centered on this point.)
 	size	    width of character (as scaled in vplot).  The height 
 		    of a char is 7/4 * size.  The blank space between
 		    chars is 1/2 * size, thus the spacing between the
 		    start of one char and the next is 3/2 * size.
        angle       orientation of char, in degrees ccw from x-axis.
	coord_sys   0: origin at bottom left, x increases to right,
			y increases up.
		    1: origin at top left, x increases to right,
			y increases down.
 	s	    character string to draw (x&y are automatically advanced.)
 
 	 the 'mascii' array tells where the 1st point in 'move' is and
 	 how many steps to take for each entry 'j'.
 		number of steps = mascii(j) / 1000
 		location of first point = last three digits.
 		e.g. mascii(65)=9021  Start this symbol (the letter 'A')
 		at location '21', and continue for 9 moves of the pen.
 
 	 the 'move' array tells how to move the pen.    the moves are
 	 given in the expanded version below, where '48' means x=4,y=8,
 	 '18' means x=1,y=8, etc. '00' means lift pen (as in making'=').
 	 The point '3,3' is the origin -- values less than 3 are used to
 	 draw the 'centered' characters [#1-16] and to draw below the
       line for some lower case letter.
 
       the details of the expanded 'pen-move' array.
       note how one symbol's points often overlap the next.
 
     0                32 sp   1  61     64 at  14  75     96 dg   9 170
     1 cl   9 441     33  !   9 158     65  A   9  21     97  a  12 309
     2 sq   5 431     34  "   9  43     66  B  12 145     98  b  10 321
     3 hx   7 456     35  #  11  33     67  C   8 135     99  c   8 335
     4 dm   5 466     36  $  16  86     68  D   7   1    100  d  10 335
     5 tr   6 470     37  %  14 121     69  E   7 112    101  e  10 333
     6 dl   6 475     38  &  10 251     70  F   6 112    102  f   8 345
     7 ++   5 436     39  '   4  43     71  G  10 135    103  g  13 353
     8 xx   5 427     40  (   4 230     72  H   6 242    104  h   8 197
     9 **   8 449     41  )   4  18     73  I   7  51    105  i   5 369
    10 c+  10 436     42  *   8  59     74  J   5 226    106  j   8 393
    11 c*  17 441     43  +   5  65     75  K   6 246    107  k   6 204
    12 s+  10 431     44  ,   6 263     76  L   3 233    108  l   3 424
    13 sx   9 427     45  -   2  68     77  M   5 234    109  m  10 383
    14 h+  11 456     46  .   5 263     78  N   4 237    110  n   7 198
    15 h*  14 449     47  /   2 121     79  O   9 134    111  o   9 358
    16 d+   9 462     48  0   9   4     80  P   7 149    112  p  10 323
    17                49  1   5  54     81  Q  12   4    113  q  11 358
    18                50  2   8 302     82  R   9 149    114  r   6 198
    19                51  3  13 282     83  S  12 172    115  s  10 414
    20                52  4   6  29     84  T   4 110    116  t   7 371
    21                53  5  10 272     85  U   6 225    117  u   7 399
    22                54  6  12 274     86  V   3 219    118  v   3 405
    23                55  7   5 269     87  W   5 239    119  w   5 407
    24                56  8  16 170     88  X   5 118    120  x   5 379
    25                57  9  13 294     89  Y   5 221    121  y  11 395
    26                58  :  11 185     90  Z   4  15    122  z   4 376
    27                59  ;  12 185     91  [   4 109    123  {   8 102
    28                60  <   3  73     92  \   2 118    124  |   2 110
    29                61  =   5  33     93  ]   4 209    125  }   8 212
    30                62  >   3 412     94  ^   3 260    126  ~   4  69
    31 ct  11  82     63  ?  13 163     95  _   2 481    127           
 
	  1  D  7          6a 3a 33  +
	  4  Q 12   0  9   63 75 78 6a 4a 38 35 43 63 00 55  +
	 15  Z  4          73 33 7a  +
	 18  )  4          3a 48 45  +
	 21  A  9          33 36 76 36 38 4a 6a 78  +
	 29  4  6          73 53 63 6a  +
	 33  # 11   =  5   35 75 00 77 37 47 48 44 00 64  +
	 43  "  9   '  4   68 5a 6a 68 00 4a 48 3a  +
	 51  I  7          4a 6a 00  +
	 54  1  5          43 63 53 5a 49
	 59  *  8          77 35  +
	 61 sp  1          00 75 37 00
	 65  +  5          58 54 00  +
	 68  -  2          36  +
	 69  ~  4          76 65 47 36
	 73  <  3          78 36  +
	 75 at 14          74 68 58 46 55 65 76  +
	 82 ct 11          78 69 49 38  +
	 86  $ 16          35 44 64 75 00 53 5a 00 78 69 49 38 47 67 76 75
	102  {  8          7a 69 67 57 56 66 64  +
	109  [  4          73  +
	110  T  4   |  2   53 5a  +
	112  E  7   F  6   7a 3a 37 67 37 33  +
	118  X  5   \  2   73 3a 00  +
	121  % 14   /  2   33 7a 00 4a 39 48 59 4a 00 74 65 54 63  +
	134  O  9          74  +
	135  G 10   C  8   79 6a 4a 39 34 43 63 74 76 56
	145  B 12          67 76 74 63  +
	149  R  9   P  7   33 3a 6a 79 78 67 37 57 73
	158  !  9          55 6a 5a 55 00  +
	163  ? 13          63 54 53 63 00 55 57  +
	170  8 16  dg  9   67 78  +
	172  S 12          79 6a 4a 39 38 47 67 76 74 63 43 34 36
	185  ; 12   : 11   47 48 58 57 47 00 54 44 45 55 54 43
	197  h  8          3a  +
	198  n  7   r  6   37 33 36 47 67 76  +
	204  k  6          73 56 77 35 33  +
	209  ]  4          3a 5a 53  +
	212  }  8          33 44 46 56 57 47 49  +
	219  V  3          3a 53  +
	221  Y  5          7a 57 53 57  +
	225  U  6          3a  +
	226  J  5          34 43 63 74  +
	230  (  4          7a 68 65  +
	233  L  3          73  +
	234  M  5          33 3a 56  +
	237  N  4          7a 73  +
	239  W  5          3a 33 57  +
	242  H  6          73 7a 77 37  +
	246  K  6          3a 33 36 7a 58  +
	251  & 10          73 48 49 5a 59 35 34 43 53  +
	260  ^  3          75 58 35
	263  ,  6   .  5   53 54 44 43 53 42
	269  7  5          53 54 79  +
	272  5 10          7a 3a  +
	274  6 12          36 47 67 76 74 63 43 34  +
	282  3 13          39 4a 6a 79 78 67 47 67 76 74 63 43  +
	294  9 13          34 43 63 74 77 66 46 37  +
	302  2  8          39 4a 6a 79 77 35 33  +
	309  a 12          73 75 66 46 35 34 43 63 74 76 67 47
	321  b 10          3a 33  +
	323  p 10          36 47 67 76 74 63 43 34 37 30
	333  e 10          35 75  +
	335  c  8   d 10   76 67 47 36 34 43 63 74 73 7a
	345  f  8          79 6a 5a 49 43 00 37 57
	353  g 13          31 40 60 71 77  +
	358  q 11   o  9   76 67 47 36 34 43 63 74 76 77 70
	369  i  5          59 00  +
	371  t  7          63 54 57 59 00  +
	376  z  4          37 77 33  +
	379  x  5          73 37 00 77  +
	383  m 10          33 37 36 47 56 53 56 67 76 73
	393  j  8          79 00  +
	395  y 11          31 40 60 71  +
	399  u  7          73 77 74 63 43 34  +
	405  v  3          37 53  +
	407  w  5          77 63 57 43 37
	412  >  3          38 76  +
	414  s 10          34 43 63 74 65 45 36 47 67 76
	424  l  3          5a 54 63
	427 sx  9  xx  5   11 55 00 15  +
	431 s+ 10  sq  5   51 55 15 11 51  +
	436 c+ 10  ++  5   53 13 00 35 31  +
	441 c* 17  cl  9   41 52 54 45 25 14 12 21  +
	449 h* 14  **  8   41 25 00 45 21 00 13  +
	456 h+ 11  hx  7   53 45 25 13 21 41  +
	462 d+  9          53 13 00 31  +
	466 dm  5          35 13 31 53  +
	470 tr  6          35 12 52 35 00  +
	475 dl  6          33 00 31 54 14 31
	481  _  2          32 72
*/

	static int mask4 = 0x000000ff;

	static short mascii[127] = 
	{
	     9441, 5431, 7456, 5466, 6470, 6475, 5436, 5427, 8449,
	    10436,17441,10431, 9427,11456,14449, 9462, 1061, 1061, 1061, 1061,
 	     1061, 1061, 1061, 1061, 1061, 1061, 1061, 1061, 1061, 1061,11082,
 	     1061, 9158, 9043,11033,16086,14121,10251, 4043, 4230, 4018, 8059,
 	     5065, 6263, 2068, 5263, 2121, 9004, 5054, 8302,13282, 6029,10272,
 	    12274, 5269,16170,13294,11185,12185, 3073, 5033, 3412,13163,14075,
 	     9021,12145, 8135, 7001, 7112, 6112,10135, 6242, 7051, 5226, 6246,
 	     3233, 5234, 4237, 9134, 7149,12004, 9149,12172, 4110, 6225, 3219,
 	     5239, 5118, 5221, 4015, 4109, 2118, 4209, 3260, 2481, 9170,12309,
 	    10321, 8335,10335,10333, 8345,13353, 8197, 5369, 8393, 6204, 3424,
 	    10383, 7198, 9358,10323,11358, 6198,10414, 7371, 7399, 3405, 5407,
 	     5379,11395, 4376, 8102, 2110, 8212, 4069, 1061
	};
	static short move[241] = 
	{
    			    0x6a3a, 0x3363, 0x7578, 0x6a4a, 0x3835, 0x4363, 
     	    0x0055, 0x7333, 0x7a3a, 0x4845, 0x3336, 0x7636, 0x384a, 0x6a78, 
     	    0x7353, 0x636a, 0x3575, 0x0077, 0x3747, 0x4844, 0x0064, 0x685a, 
     	    0x6a68, 0x004a, 0x483a, 0x4a6a, 0x0043, 0x6353, 0x5a49, 0x7735, 
     	    0x0075, 0x3700, 0x5854, 0x0036, 0x7665, 0x4736, 0x7836, 0x7468, 
     	    0x5846, 0x5565, 0x7678, 0x6949, 0x3835, 0x4464, 0x7500, 0x535a, 
     	    0x0078, 0x6949, 0x3847, 0x6776, 0x757a, 0x6967, 0x5756, 0x6664, 
     	    0x7353, 0x5a7a, 0x3a37, 0x6737, 0x3373, 0x3a00, 0x337a, 0x004a, 
     	    0x3948, 0x594a, 0x0074, 0x6554, 0x6374, 0x796a, 0x4a39, 0x3443, 
     	    0x6374, 0x7656, 0x6776, 0x7463, 0x333a, 0x6a79, 0x7867, 0x3757, 
     	    0x7355, 0x6a5a, 0x5500, 0x6354, 0x5363, 0x0055, 0x5767, 0x7879, 
     	    0x6a4a, 0x3938, 0x4767, 0x7674, 0x6343, 0x3436, 0x4748, 0x5857, 
     	    0x4700, 0x5444, 0x4555, 0x5443, 0x3a37, 0x3336, 0x4767, 0x7673, 
     	    0x5677, 0x3533, 0x3a5a, 0x5333, 0x4446, 0x5657, 0x4749, 0x3a53, 
     	    0x7a57, 0x5357, 0x3a34, 0x4363, 0x747a, 0x6865, 0x7333, 0x3a56, 
     	    0x7a73, 0x3a33, 0x5773, 0x7a77, 0x373a, 0x3336, 0x7a58, 0x7348, 
     	    0x495a, 0x5935, 0x3443, 0x5375, 0x5835, 0x5354, 0x4443, 0x5342, 
     	    0x5354, 0x797a, 0x3a36, 0x4767, 0x7674, 0x6343, 0x3439, 0x4a6a, 
            0x7978, 0x6747, 0x6776, 0x7463, 0x4334, 0x4363, 0x7477, 0x6646, 
     	    0x3739, 0x4a6a, 0x7977, 0x3533, 0x7375, 0x6646, 0x3534, 0x4363, 
     	    0x7476, 0x6747, 0x3a33, 0x3647, 0x6776, 0x7463, 0x4334, 0x3730, 
     	    0x3575, 0x7667, 0x4736, 0x3443, 0x6374, 0x737a, 0x796a, 0x5a49, 
     	    0x4300, 0x3757, 0x3140, 0x6071, 0x7776, 0x6747, 0x3634, 0x4363, 
     	    0x7476, 0x7770, 0x5900, 0x6354, 0x5759, 0x0037, 0x7733, 0x7337, 
     	    0x0077, 0x3337, 0x3647, 0x5653, 0x5667, 0x7673, 0x7900, 0x3140, 
     	    0x6071, 0x7377, 0x7463, 0x4334, 0x3753, 0x7763, 0x5743, 0x3738, 
     	    0x7634, 0x4363, 0x7465, 0x4536, 0x4767, 0x765a, 0x5463, 0x1155, 
     	    0x0015, 0x5155, 0x1511, 0x5153, 0x1300, 0x3531, 0x4152, 0x5445, 
     	    0x2514, 0x1221, 0x4125, 0x0045, 0x2100, 0x1353, 0x4525, 0x1321, 
     	    0x4153, 0x1300, 0x3135, 0x1331, 0x5335, 0x1252, 0x3500, 0x3300, 
     	    0x3154, 0x1431, 0x3272
	};
	int i, n, lfrt, nstart, nend, nsteps, nn, mpen, mv, xxx, yyy,
		last_x=0, last_y=0;
	float xx, yy, sina, cosa, xi, yi;

	if(!coord_sys)
	{
		angle += 90.;
	}
	xx = .0174533 * angle;
	yy = .25 * size;
	sina = yy * sin(xx);
	cosa = yy * cos(xx);
	if(!coord_sys)
	{
		xx = y - 3.*(cosa-sina);
		yy = x - 3.*(sina+cosa);
	}
	else
	{
		xx = x - 3.*(cosa-sina);
		yy = y - 3.*(sina+cosa);
	}

	/* draw all characters given in s.
	 */
	for(; *s != '\0'; s++)
	{
		i = (int)(*s) - 1;
		nsteps = mascii[i] / 1000;
		nstart = mascii[i] - 1000*nsteps;
		nend   = nstart + nsteps - 1;
		mpen   = 0;

		for(n = nstart; n <= nend; n++)
		{
			nn = (n+1)/2;
			lfrt = 2*nn - n;
			mv = move[nn-1];
			if(lfrt == 1) mv = mv/256;
			mv &= mask4;
			if(mv <= 0)
			{
				mpen = 0;
			}
			else
			{
				xi = mv / 16;
				yi = mv & 15;
				xxx = (int)((xx + xi*cosa - yi*sina + .5));
				yyy = (int)((yy + xi*sina + yi*cosa + .5));
				if(mpen)
				{
					if(!coord_sys)
					{
						imove(d, scale_x(d, last_y),
							 scale_y(d, last_x));
						idraw(d, scale_x(d, yyy),
							 scale_y(d, xxx));
					}
					else
					{
						imove(d, scale_x(d, last_x),
							 scale_y(d, last_y));
						idraw(d, scale_x(d, xxx),
							 scale_y(d, yyy));
					}
				}
				last_x = xxx;
				last_y = yyy;
				mpen = 1;
			}
		}
		/* advance origin to next starting location
		 */
		xx = xx + 6. * cosa;
		yy = yy + 6. * sina;
	}
	iflush(d);

	return;
}
