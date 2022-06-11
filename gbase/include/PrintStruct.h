#ifndef _PRINTSTRUCT_h
#define	_PRINTSTRUCT_h

typedef struct
{
	char	label_font[15];
	char	axis_font[15];
	char	tag_font[15];
	char	arr_font[15];
	char	title_font[15];

	int	label_fontsize;
	int	axis_fontsize;
	int	tag_fontsize;
	int	arr_fontsize;
	int	title_fontsize;
} AxesFonts;

typedef struct
{
        double  left, right, top, bottom;
        bool    portrait, color, arrPlace, tagPlace, full_scale, center;
        bool    top_title_user;
        int     top_title_lines, top_title_x, top_title_y;
        int     x_axis_x, x_axis_y;
        AxesFonts fonts;
        int     data_linewidth;
        char    labelsFontStyle[15], axesFontStyle[15];
        char    tagFontStyle[15], arrFontStyle[15], mainFontStyle[15];
        char    *top_title, *x_axis_label, *y_axis_label;
} PrintStruct;

#endif
