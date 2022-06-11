#include "qctool.h"
#define PTS_PER_INCH     72.0
#define INCHES_PER_CM 1.0/2.54

int  CheckLib_ps();
void Hplots(FILE *plot_file,int condition);
void HplotsBbox(FILE *plot_file,int condition,int llx,int lly,int urx,int ury);
void Newpage(FILE *plot_file,int *label, int *ordinal);
void Plot(FILE *plot_file,float x, float y,int num);
void PlotCircle(FILE *plot_file,float xor, float yor,
                           float radius, int pts,
                           float red,float green,float blue,
                           int fill);
void PlotVector(FILE *plot_file,double *x, double *y,int n,
                double xor,double yor,double pl,double ph,
                double red, double green, double blue,
                logical plot_grey,double grey,double linethickness,
                double x_min,double x_max,double y_min,double y_max);
void PlotBox(FILE *plot_file,double *pt0, double *pt1,
             double x_min,double x_max,double y_min,double y_max,
             logical fill,double red, double green, double blue,
             double linewidth,double xor,double yor,double pl,double ph);
void PlotNumber(FILE *logfile,double height,double angle,double xpt,
                double ypt,double number,char *font,double red, double blue, double green,int ndec);
void PlotString(FILE *plot_file,double height,double angle,double xpt,
                double ypt,char *string,char *font,double red,double green, double blue,int nchar);
void PlotCross(FILE *plot_file,double xpt, double ypt,double linewidth,double size,
               double red, double green, double blue);
void ConvertHSVtoRGB(double *h,double *s,double *v,double *r,double *g,double *b);
void Endpage(FILE *plot_file);
void PlotArrow(FILE *plot_file,float xpt,float ypt,double angle,double size,
               double red, double green, double blue);
void Plot3D(FILE *plot_file,
            float x3D_min,float x3D_max,
            float y3D_min,float y3D_max,
            float z3D_min,float z3D_max,
            float x3D,float y3D,float z3D,
            float phi,float theta,float psi,
            float linewidth,
            float r,float g,float b,
            int m);
