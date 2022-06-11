#include "config.h"
#include "libgplot.h"

static double
findMin(double  r, double  g, double  b);

static double 
findMax(double  r, double  g, double  b);

#ifdef MAIN
#include <stdio.h>

main()
{
	char	type[8];
	double	a, b, c;
	double	x, y, z;

	while (fscanf(stdin, "%s %lf %lf %lf", type, &a, &b, &c) == 4)
	{

		if (!strcmp(type, "hsv"))
		{
			hsvToRgb(a, b, c, &x, &y, &z);
		}
		else
		{
			rgbToHsv(a, b, c, &x, &y, &z);
		}

		fprintf(stdout, "%d %d %d\n", (int) x, (int) y, (int) z);

	}
}
#endif /* MAIN */

void
rgbToHsv(double r, double g, double b, double *h, double *s, double *v)
{
	double min, max;

	min = findMin(r, g, b);
	max = findMax(r, g, b);

	/* set val to the maximum */
	*v = max;

	/* if max is 0, we have black */
	if (max == 0)
	{
		*s = 0;
		*h = 0;
	}
	else
	{
		*s = ((max-min)/max)*255.;
		if (r == max)
		{
			*h = 60 * ((g - b)/(max - min));
		}
		else if (g == max)
		{
			*h = 60 * (2 + (b - r)/(max - min));
		}
		else if (b == max)
		{
			*h = 60 * (4 + (r - g)/(max - min));
		}

		if (*h < 0)
		{
			*h = *h + 360;
		}
	}
}
	
static double
findMin(double  r, double  g, double  b)
{
	double	min;

	if (r < g)
	{
		if (r < b)
		{
			min = r;
		}
		else
		{
			min = b;
		}
	}
	else 
	{
		if (g < b)
		{
			min = g;
		}
		else
		{
			min = b;
		}
	}
	
	return min;
}

static double
findMax(double r, double g, double b)
{
	double	max;

	if (r > g)
	{
		if (r > b)
		{
			max = r;
		}
		else
		{
			max = b;
		}
	}
	else 
	{
		if (g > b)
		{
			max = g;
		}
		else
		{
			max = b;
		}
	}
	
	return max;
}

void
hsvToRgb(double h, double s, double v, double *r, double *g, double *b)
{
	int	maxc;
	double	vals[3];
	double	min, thu;

	if (h > 300)
	{
		h = h - 360;
	}

	if (h < 60)
	{
		maxc = 0;
	}
	else if (h > 180)
	{
		maxc = 2;
	}
	else
	{
		maxc = 1;
	}

	vals[maxc] = v;

	min = (v*(255. - s)) / 255.;

	thu = ((h/60) - (2*maxc)) * (v - min);

	if (thu > 0.0)
	{
		vals[(maxc+2)%3] = min;
		vals[(maxc+1)%3] = min + thu;
	}
	else
	{
		vals[(maxc+1)%3] = min;
		vals[(maxc+2)%3] = min - thu;
	}

	*r = vals[0];
	*g = vals[1];
	*b = vals[2];
}

void
rgbBrighten(double  *r, double  *g, double  *b, double  percent)
{
	double h, s, v;
	double red, green, blue;

	red = *r;
	green = *g;
	blue = *b;

	rgbToHsv(red, green, blue, &h, &s, &v);
	v = v * percent;
	hsvToRgb(h, s, v, &red, &green, &blue);

	*r = red;
	*g = green;
	*b = blue;
}
