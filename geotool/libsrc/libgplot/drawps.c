#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "libgplot.h"
#include "logErrorMsg.h"

void
gdrawInitPS(FILE *fp, char do_color, char do_portrait)
{
	/* Output ps prologue.
	 */
	fprintf(fp, "%%!PS-Adobe-2.0\n");
	fprintf(fp, "/S { currentpoint stroke moveto } def\n");
	fprintf(fp, "/M { moveto } def\n");
	fprintf(fp, "/d { lineto } def\n");
	fprintf(fp, "/c { copy } def\n");
	fprintf(fp, "/I { index } def\n");
	fprintf(fp, "/L { lineto } def\n");
	fprintf(fp, "/D { lineto currentpoint stroke moveto } def\n");
	fprintf(fp, "/m { rmoveto } def\n");
	fprintf(fp, "/l { rlineto } def\n");
	fprintf(fp, "/s { scale } def\n");
	fprintf(fp,"/PR { dup stringwidth pop neg 0 rmoveto");
	fprintf(fp, " dup false charpath pathbbox exch pop 1 index sub 2 div");
	fprintf(fp, " sub newpath moveto show } def\n");
	fprintf(fp,
	    "/PL { dup stringwidth pop dup div 0 rmoveto");
	fprintf(fp, " dup false charpath pathbbox exch pop 1 index sub 2 div");
	fprintf(fp, " sub newpath moveto show } def\n");

	fprintf(fp, "/PC {dup stringwidth pop 2 div neg 0 rmoveto show} def\n");
	fprintf(fp,"/P { show } def\n");
	fprintf(fp, "/C { closepath } def\n");
	fprintf(fp, "/N { newpath } def\n");
	fprintf(fp, "/p { 1 1 rectfill } def\n");
	fprintf(fp, "/r { rotate } def\n");
	fprintf(fp, "/slw { setlinewidth } def\n");
	fprintf(fp, "/inch {%d mul} def\n", DOTS_PER_INCH);
	fprintf(fp, "/fontscale {%.5f} def\n", 1./POINTS_PER_DOT);
	fprintf(fp, "%%%%EndProlog\n");

	fprintf(fp, "1 setlinecap\n");
	fprintf(fp, "%.5f %.5f scale N 0 slw [ ] 0 setdash\n",
			POINTS_PER_DOT, POINTS_PER_DOT);

	if(do_color) fprintf(fp, "gsave\n");

	if(!do_portrait)
	{
	    int i = (int)(-8.5*DOTS_PER_INCH);
	    fprintf(fp, "90 rotate 0 %d translate\n", i);
	}
}
