/** \file MapOverlay.cpp
 *  \brief Defines class MapOverlay.
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdio.h>
#include <errno.h>
#include <sstream>

extern "C" {
#include "libstring.h"
}

#include "MapOverlay.h"
#include "MapWindow.h"
#include "motif++/Toggle.h"
#include "widget/Map.h"

using namespace libgmap;

/** Constructor. Create a MapOverlay object from the input file.
 *  @throws GERROR_INVALID_ARGS if there is an error reading the file, or the
 * 	first line in the file does not contain the string "overlay".
 */
MapOverlay::MapOverlay(const string &pathname, MapWindow *mapWindow) : Gobject()
{
    path = stringToQuark(pathname);
    mw = mapWindow;
    map = mw->map;
    name = -1;
    num_ids = 0;
    id = NULL;
    selected = False;
    has_been_read = False;
    readOverlayName();
}

MapOverlay::~MapOverlay(void)
{
    if(id) free(id);
}

void MapOverlay::readOverlayName(void)
{
    const char *pathname = quarkToString(path);
    char *str, buf[200];
    int line_no;
    FILE *fp;

    if((fp = fopen(pathname, "r")) == NULL)
    {
	if(errno > 0) {
	    GError::setMessage("Cannot open: %s\n%s", pathname,strerror(errno));
	}
	else {
	    GError::setMessage("Cannot open: %s", pathname);
	}
	throw(GERROR_INVALID_ARGS);
    }

    buf[0] = '\0';
    line_no = 0;
    if(getLine(fp, buf, sizeof(buf), &line_no) != EOF)
    {
	if((str = stringGetArg(buf, "overlay")) != NULL)
	{
	    name = stringToQuark(str);
	    free(str);
	    buf[0] = '\0';
	}
	else {
	    GError::setMessage("Error reading %s\n%s", pathname,
		"The first line of the file must contain the word 'overlay'");
	    fclose(fp);
	    throw(GERROR_INVALID_ARGS);
	}
    }
    fclose(fp);
}

void MapOverlay::readOverlay(int display)
{
    const char	*pathname;
    string	msg;
    char	*str, cmd[200], label[50], buf[200];
    int		i, j, n=0, m, siz, nsta, nsrc, line_no;
    double	lat, lon, latmin, latmax, lonmin, lonmax;
    double	x1, x2, y1, y2, x0, y0, xdif, ydif;
    Pixel	bg, fg;
    Arg		args[20];
    bool	on, found_source = false, warned_once = false;
    SymbolInfo	default_sym;
    MapPlotStation	sta, *stations = NULL, sta_init = MAP_PLOT_STATION_INIT;
    MapPlotSource	src, *sources = NULL, src_init = MAP_PLOT_SOURCE_INIT;
    MapPlotArc		arc, arc_init = MAP_PLOT_ARC_INIT;
    MapPlotDelta	del, del_init = MAP_PLOT_DELTA_INIT;
    MapPlotEllipse	ell, ell_init = MAP_PLOT_ELLIPSE_INIT;
    MapPlotLine		line, line_init = MAP_PLOT_LINE_INIT;
    MapPlotSymbolGroup	symbol, symbol_init = MAP_PLOT_SYMBOL_GROUP_INIT;
    MapPlotPolygon	poly, poly_init = MAP_PLOT_POLYGON_INIT;
    FILE		*fp;

    if(has_been_read)
    {
	/* The overlay has already been input. Delete it and read it again.
	 */
	for(j = 0; j < num_ids; j++)
	{
	    map->mapDelete(id[j], False);
	}
	has_been_read = False;
	if(id) free(id);
	num_ids = 0;
    }

    pathname = quarkToString(path);

    if((fp = fopen(pathname, "r")) == NULL)
    {
	if(errno > 0) {
	    mw->showWarning("Cannot open: %s\n%s", pathname, strerror(errno));
	}
	else {
	    mw->showWarning("Cannot open: %s", pathname);
	}
	return;
    }

    buf[0] = '\0';
    line_no = 0;
    if(getLine(fp, buf, 200, &line_no) != EOF)
    {
	if((str = stringGetArg(buf, "overlay")) != NULL)
	{
	    name = stringToQuark(str);
	    free(str);
	    buf[0] = '\0';
	}
	else {
	    name = stringToQuark(pathname);
	}
    }
    id = (int *)mallocWarn(sizeof(int));
    num_ids = 0;
    has_been_read = True;

    XtSetArg(args[0], XtNforeground, &fg);
    XtSetArg(args[1], XtNbackground, &bg);
    map->getValues(args, 2);

    if(buf[0] == '\0') {
	n = getLine(fp, buf, 200, &line_no);
    }
    while(n != EOF)
    {
	cmd[0] = '\0';
	sscanf(buf, "%s", cmd);

	if(!strcmp(cmd, "stations"))
	{
	    /* get overall defaults on the "stations" line */
	    getOverlayDefaults(buf, "stations", &default_sym, fg);

	    while((n = getLine(fp, buf, 200, &line_no)) != EOF)
	    {
		sta = sta_init;
		if(sscanf(buf, "%s %lf %lf", label, &sta.lat, &sta.lon)
			!= 3 || isCommand(buf)) break;
		sta.tag_loc = NOPREF;
		sta.label = label;

		sta.sym = default_sym;

		if((str = stringGetArg(buf, "symbol")) != NULL)
		{
		    sta.sym.type = getSymType(str);
		    free(str);
		}
		parseGetArg(buf, "stations", msg, "size", &sta.sym.size);
		if((str = stringGetArg(buf, "color")) != NULL)
		{
		    sta.sym.fg = mw->stringToPixel(str);
		    free(str);
		}
		sta.sym.display = display;
		id = (int *)reallocWarn(id, (num_ids+1)*sizeof(int));
		id[num_ids++] = map->addStation(&sta, False);
	    }
	}
	else if(!strcmp(cmd, "sources"))
	{
	    /* get overall defaults on the "sources" line */
	    getOverlayDefaults(buf, "sources", &default_sym, fg);

	    while((n = getLine(fp, buf, 200, &line_no)) != EOF)
	    {
		src = src_init;
		if(sscanf(buf, "%s %lf %lf", label, &src.lat, &src.lon)
			!= 3 || isCommand(buf)) break;
		found_source = true;
		src.tag_loc = NOPREF;
		src.label = label;
		src.depth = -999.;
		src.time = NULL_TIME;
		src.smajax = -1.;
		src.sminax = -1.;
		src.strike = -1.;
		parseGetArg(buf, "sources", msg, "smajax", &src.smajax);
		parseGetArg(buf, "sources", msg, "sminax", &src.sminax);
		parseGetArg(buf, "sources", msg, "strike", &src.strike);
		parseGetArg(buf, "sources", msg, "depth", &src.depth);

		src.sym = default_sym;
		if((str = stringGetArg(buf, "symbol")) != NULL)
		{
		    src.sym.type = getSymType(str);
		    free(str);
		}
		parseGetArg(buf, "sources", msg, "size", &src.sym.size);
		if((str = stringGetArg(buf, "color")) != NULL)
		{
		    src.sym.fg = mw->stringToPixel(str);
		    free(str);
		}
		src.sym.display = display;
		id = (int *)reallocWarn(id, (num_ids+1)*sizeof(int));
		id[num_ids] = map->addSource(&src, False);
		src.id = id[num_ids];
		num_ids++;
	    }
	}
	else if(!strcmp(cmd, "arcs"))
	{
	    while((n = getLine(fp, buf, 200, &line_no)) != EOF)
	    {
		arc = arc_init;
		if(sscanf(buf, "%lf %lf %lf %lf", &arc.lat,
		    &arc.lon, &arc.del, &arc.az) != 4 || isCommand(buf)) break;
		arc.label = NULL;
		if((str = stringGetArg(buf, "label")) != NULL)
		{
		    stringcpy(label, str, sizeof(label));
		    arc.label = label;
		    free(str);
		}
		getLineInfo(buf, bg, fg, &arc.line);
		arc.line.display = display;
		id = (int *)reallocWarn(id, (num_ids+1)*sizeof(int));
		id[num_ids++] = map->addArc(&arc, False);
	    }
	}
	else if(!strcmp(cmd, "circles"))
	{
	    while((n = getLine(fp, buf, 200, &line_no)) != EOF)
	    {
		del = del_init;
		if(sscanf(buf, "%lf %lf %lf", &del.lat, &del.lon, &del.del)
			!= 3 || isCommand(buf)) break;
		del.label = NULL;
		if((str = stringGetArg(buf, "label")) != NULL)
		{
		    stringcpy(label, str, sizeof(label));
		    del.label = label;
		    free(str);
		}
		getLineInfo(buf, bg, fg, &del.line);
		del.line.display = display;
		id = (int *)reallocWarn(id, (num_ids+1)*sizeof(int));
		id[num_ids++] = map->addDelta(&del, False);
	    }
	}
	else if(!strcmp(cmd, "ellipses"))
	{
	    while((n = getLine(fp, buf, 200, &line_no)) != EOF)
	    {
		ell = ell_init;
		if(sscanf(buf, "%lf %lf %lf %lf %lf", &ell.lat, &ell.lon,
			&ell.sminax, &ell.smajax, &ell.strike) != 5
			|| isCommand(buf)) break;
		ell.label = NULL;
		if((str = stringGetArg(buf, "label")) != NULL)
		{
		    stringcpy(label, str, sizeof(label));
		    ell.label = label;
		    free(str);
		}
		getLineInfo(buf, bg, fg, &ell.line);
		ell.line.display = display;
		id = (int *)reallocWarn(id, (num_ids+1)*sizeof(int));
		id[num_ids++] = map->addEllipse(&ell, False);
	    }
	}
	else if(!strcmp(cmd, "assoc"))
	{
	    if(found_source)
	    {
		char s[200];
		nsta = map->getStations(&stations);
		while((n = getLine(fp, buf, 200, &line_no)) != EOF)
		{
		    if(sscanf(buf, "%s", s) != 1 || isCommand(buf)) break;

		    for(i = 0; i < nsta; i++)
			if(!strcmp(stations[i].label, s))
		    {
			map->assocSta(src.id, 1, &stations[i].id, False,
					False);
			break;
		    }
		}
		Free(stations);
	    }
	}
	else if(!strcmp(cmd, "line"))
	{
	    line = line_init;
	    line.label = NULL;
	    if((str = stringGetArg(buf, "label")) != NULL)
	    {
		stringcpy(label, str, sizeof(label));
		line.label = label;
		free(str);
	    }
	    getLineInfo(buf, bg, fg, &line.line);
	    line.line.display = display;
	    line.npts = m = 0;
	    line.lat = (double *)mallocWarn(sizeof(double));
	    line.lon = (double *)mallocWarn(sizeof(double));

	    while((n = getLine(fp, buf, 200, &line_no)) != EOF)
	    {
		if(sscanf(buf, "%lf %lf", &lat, &lon) != 2 ||
			isCommand(buf)) break;
		line.lat = (double *)reallocWarn(line.lat,(m+1)*sizeof(double));
		line.lon = (double *)reallocWarn(line.lon,(m+1)*sizeof(double));
		line.lat[m] = lat;
		line.lon[m] = lon;
		m++;
	    }
	    if(m > 0)
	    {
		line.npts = m;
		id = (int *)reallocWarn(id, (num_ids+1)*sizeof(int));
		id[num_ids++] = map->addLine(&line, False);
	    }
	    free(line.lat);
	    free(line.lon);
	}
	else if(!strcmp(cmd, "boundary"))
	{
	    bool first = true;
	    line = line_init;
	    line.label = NULL;
	    if((str = stringGetArg(buf, "label")) != NULL)
	    {
		stringcpy(label, str, sizeof(label));
		line.label = label;
		free(str);
	    }
	    getLineInfo(buf, bg, fg, &line.line);
	    line.line.display = display;
	    line.npts = m = 0;
	    line.lat = (double *)mallocWarn(sizeof(double));
	    line.lon = (double *)mallocWarn(sizeof(double));

	    while((n = getLine(fp, buf, 200, &line_no)) != EOF)
	    {
		if(sscanf(buf, "%lf %lf", &lon, &lat) == 2 && !isCommand(buf))
		{
		    line.lat = (double *)reallocWarn(line.lat,
						(m+1)*sizeof(double));
		    line.lon = (double *)reallocWarn(line.lon,
						(m+1)*sizeof(double));
		    line.lat[m] = lat;
		    line.lon[m] = lon;
		    if(!first) m++;
		    first = false;
		}
		else if(!strncmp(buf, "END", 3))
		{
		    if(m > 0)
		    {
			line.npts = m;
			id = (int *)reallocWarn(id, (num_ids+1)*sizeof(int));
			id[num_ids++] = map->addLine(&line, False);
			line.npts = m = 0;
			first = true;
		    }
		}
	    }
	    if(m > 0)
	    {
		line.npts = m;
		id = (int *)reallocWarn(id, (num_ids+1)*sizeof(int));
		id[num_ids++] = map->addLine(&line, False);
	    }
	    free(line.lat);
	    free(line.lon);
	}
	else if(!strcmp(cmd, "symbols"))
	{
	    symbol = symbol_init;
	    symbol.sym.display = display;
	    symbol.label = NULL;
	    if((str = stringGetArg(buf, "label")) != NULL)
	    {
		stringcpy(label, str, sizeof(label));
		symbol.label = label;
		free(str);
	    }
	    if((str = stringGetArg(buf, "symbol")) != NULL)
	    {
		symbol.sym.type = getSymType(str);
		free(str);
	    }
	    symbol.sym.fg = fg;
	    if((str = stringGetArg(buf, "color")) != NULL)
	    {
		symbol.sym.fg = mw->stringToPixel(str);
		free(str);
	    }

	    symbol.npts = m = 0;
	    symbol.lat = (double *)mallocWarn(sizeof(double));
	    symbol.lon = (double *)mallocWarn(sizeof(double));
	    symbol.size = (int *)mallocWarn(sizeof(int));

	    while((n = getLine(fp, buf, 200, &line_no)) != EOF)
	    {
		if(sscanf(buf, "%lf %lf %d", &lat, &lon, &siz) != 3 ||
			isCommand(buf)) break;
		symbol.lat = (double *)reallocWarn(symbol.lat,
					(m+1)*sizeof(double));
		symbol.lon = (double *)reallocWarn(symbol.lon,
					(m+1)*sizeof(double));
		symbol.size = (int *)reallocWarn(symbol.size,(m+1)*sizeof(int));
		symbol.lat[m] = lat;
		symbol.lon[m] = lon;
		symbol.size[m] = siz;
		m++;
	    }
	    if(m > 0)
	    {
		symbol.npts = m;
		id = (int *)reallocWarn(id, (num_ids+1)*sizeof(int));
		id[num_ids++] = map->addSymbolGroup(&symbol, False);
	    }
	    free(symbol.lat);
	    free(symbol.lon);
	    free(symbol.size);
	}
	else if(!strcmp(cmd, "polygon"))
	{
	    poly = poly_init;
	    poly.sym.display = display;
	    poly.label = NULL;
	    if((str = stringGetArg(buf, "label")) != NULL)
	    {
		stringcpy(label, str, sizeof(label));
		poly.label = label;
		free(str);
	    }
	    poly.sym.fg = fg;
	    if((str = stringGetArg(buf, "color")) != NULL)
	    {
		poly.sym.fg = mw->stringToPixel(str);
		free(str);
	    }

	    poly.npts = m = 0;
	    poly.lat = (double *)mallocWarn(sizeof(double));
	    poly.lon = (double *)mallocWarn(sizeof(double));

	    while((n = getLine(fp, buf, 200, &line_no)) != EOF)
	    {
		if(sscanf(buf, "%lf %lf", &lat, &lon) != 2 ||
			isCommand(buf)) break;
		poly.lat = (double *)reallocWarn(poly.lat,(m+1)*sizeof(double));
		poly.lon = (double *)reallocWarn(poly.lon,(m+1)*sizeof(double));
		poly.lat[m] = lat;
		poly.lon[m] = lon;
		m++;
	    }
	    if(m > 0)
	    {
		poly.npts = m;
		id = (int *)reallocWarn(id, (num_ids+1)*sizeof(int));
		id[num_ids++] = map->addPolygon(&poly, False);
	    }
	    free(poly.lat);
	    free(poly.lon);
	}
	else if(!strcmp(cmd, "limits"))
	{
	    latmin = latmax = lonmin = lonmax = -999.;

	    m = 0;
	    if(parseGetArg(buf, "limits", msg, "latmin", &latmin)) m++;
	    if(parseGetArg(buf, "limits", msg, "latmax", &latmax)) m++;
	    if(parseGetArg(buf, "limits", msg, "lonmin", &lonmin)) m++;
	    if(parseGetArg(buf, "limits", msg, "lonmax", &lonmax)) m++;

	    if( latmin != -999. && latmax != -999. &&
		lonmin == -999. && lonmax == -999.)
	    {
		map->getLimits(&x1, &x2, &y1, &y2);
		x0 = .5*(x1 + x2);
		xdif = fabs((latmax-latmin)/(y2-y1))*fabs(x2-x1);
		lonmin = x0 - .5*xdif;
		lonmax = x0 + .5*xdif;
		m = 4;
	    }
	    else if(lonmin != -999. && lonmax != -999. &&
		    latmin == -999. && latmax == -999.)
	    {
		map->getLimits(&x1, &x2, &y1, &y2);
		y0 = .5*(y1 + y2);
		ydif = fabs((lonmax-lonmin)/(x2-x1))*fabs(y2-y1);
		latmin = y0 - .5*ydif;
		latmax = y0 + .5*ydif;
		m = 4;
	    }
	    if(m == 4) {
		map->setLimits(lonmin, lonmax, latmin, latmax);
	    }
	    else {
		map->unzoomAll();
	    }
	    n = getLine(fp, buf, 200, &line_no);
	}
	else if(!strcmp(cmd, "unzoom"))
	{
	    map->unzoomAll();
	    n = getLine(fp, buf, 200, &line_no);
	}
	else if(!strcmp(cmd, "rotate"))
	{
	    if( parseGetArg(buf, "rotate", msg, "lat", &lat) &&
		parseGetArg(buf, "rotate", msg, "lon", &lon))
	    {
		map->rotate(lon, lat, False);
	    }
	    else if((str = stringGetArg(buf, "station")) != NULL)
	    {
		nsta = map->getStations(&stations);
		for(i = 0; i < nsta; i++)
		{
		    if(!strcmp(stations[i].label, str)) break;
		}
		free(str);
		if(i < nsta)
		{
		    map->rotate(stations[i].lon, stations[i].lat, False);
		}
		Free(stations);
	    }
	    else if((str = stringGetArg(buf, "source")) != NULL)
	    {
		nsrc = map->getSources(&sources);
		for(i = 0; i < nsrc; i++)
		{
		    if(!strcmp(sources[i].label, str)) break;
		}
		free(str);
		if(i < nsrc)
		{
		    map->rotate(sources[i].lon,sources[i].lat,False);
		}
		Free(sources);
	    }
	    n = getLine(fp, buf, 200, &line_no);
	}
	else if(!strcmp(cmd, "delete"))
	{
	    if((str = stringGetArg(buf, "source")) != NULL)
	    {
		nsrc = map->getSources(&sources);
		for(i = 0; i < nsrc; i++)
		{
		    if(!strcmp(sources[i].label, str)) break;
		}
		free(str);
		if(i < nsrc)
		{
		    map->mapDelete(sources[i].id, False);
		}
		Free(sources);
	    }
	    else if((str = stringGetArg(buf, "station")) != NULL)
	    {
		nsta = map->getStations(&stations);
		for(i = 0; i < nsta; i++)
		{
		    if(!strcmp(stations[i].label, str)) break;
		}
		free(str);
		if(i < nsta)
		{
		    map->mapDelete(stations[i].id, False);
		}
		Free(stations);
	    }
	    n = getLine(fp, buf, 200, &line_no);
	}
	else if(!strcmp(cmd, "projection"))
	{
	    char proj_type[100];
	    if(sscanf(buf, "%s %s", cmd, proj_type) == 2)
	    {
		if(!strcasecmp(proj_type, "linear_cylindrical")) {
		    mw->setProjection("Llinear Cylindrical");
		}
		else if(!strcasecmp(proj_type, "cylindrical_equal-area")) {
		    mw->setProjection("Cylindrical Equal-Area");
		}
		else if(!strcasecmp(proj_type, "mercator")) {
		    mw->setProjection("Mercator");
		}
		else if(!strcasecmp(proj_type, "orthographic")) {
		    mw->setProjection("Orthographic");
		}
		else if(!strcasecmp(proj_type, "azimuthal_equidistant")) {
		    mw->setProjection("Azimuthal Equidistant");
		}
		else if(!strcasecmp(proj_type, "azimuthal_equal-area")) {
		    mw->setProjection("Azimuthal Equal-Area");
		}
	    }
	    n = getLine(fp, buf, 200, &line_no);
	}
	else if(!strcmp(cmd, "display"))
	{
	    if((str = stringGetArg(buf, "source_tags")) != NULL)
	    {
		on = (!strcasecmp(str, "on") || !strcasecmp(str,"true"))
			? True : False;
		mw->origin_tags->set(on, true);
		free(str);
	    }
	    if((str = stringGetArg(buf, "station_tags")) != NULL)
	    {
		on = (!strcasecmp(str, "on") || !strcasecmp(str,"true"))
			? True : False;
		mw->station_tags->set(on, true);
		free(str);
	    }
	    if((str = stringGetArg(buf, "grid")) != NULL)
	    {
		on = (!strcasecmp(str, "on") || !strcasecmp(str,"true"))
			? True : False;
		mw->grid_toggle->set(on, true);
		free(str);
	    }
	    if((str = stringGetArg(buf, "paths")) != NULL)
	    {
		if(!strcasecmp(str, "All")) mw->paths[0]->set(true, true);
		else if(!strcasecmp(str, "Selected"))
			mw->paths[1]->set(true, true);
		else if(!strcasecmp(str, "None")) mw->paths[2]->set(true, true);
		free(str);
	    }
	    if((str = stringGetArg(buf, "ellipses")) != NULL)
	    {
		if(!strcasecmp(str, "All")) mw->ellipses[0]->set(true, true);
		else if(!strcasecmp(str, "Selected"))
			mw->ellipses[1]->set(true, true);
		else if(!strcasecmp(str, "None"))
			mw->ellipses[2]->set(true, true);
		free(str);
	    }
	    if((str = stringGetArg(buf, "distances")) != NULL)
	    {
		if(!strcasecmp(str, "All")) mw->dist[0]->set(true, true);
		else if(!strcasecmp(str, "Selected"))
			mw->dist[1]->set(true, true);
		else if(!strcasecmp(str, "None")) mw->dist[2]->set(true, true);
		free(str);
	    }
	    if((str = stringGetArg(buf, "stations")) != NULL)
	    {
		if(!strcasecmp(str, "All")) mw->stations[0]->set(true, true);
		else if(!strcasecmp(str, "Selected"))
			mw->stations[1]->set(true, true);
		else if(!strcasecmp(str, "None"))
			mw->stations[2]->set(true, true);
		free(str);
	    }
	    if((str = stringGetArg(buf, "sources")) != NULL)
	    {
		if(!strcasecmp(str,"All")) mw->origin_toggle[0]->set(true,true);
		else if(!strcasecmp(str, "Selected"))
			mw->origin_toggle[1]->set(true, true);
		else if(!strcasecmp(str, "None"))
			mw->origin_toggle[2]->set(true, true);
		free(str);
	    }
	    n = getLine(fp, buf, 200, &line_no);
	}
	else
	{
	    if(!warned_once)
	    {
		warned_once = true;
		mw->showWarning("format error line %d", line_no);
	    }
	    n = getLine(fp, buf, 200, &line_no);
	}
    }
    fclose(fp);

    if(display >= MAP_ON)
    {
	map->update();
    }
}

int MapOverlay::getLine(FILE *fp, char *line, int len, int *line_no)
{
    int  n;
    char *c;

    /*	skip blank lines and lines whose first non-white char is '!' or '#'
	if any line contains "ignore this file", EOF is returned
	This allows Makefiles to silently exist in the mapoverlays dir
     */
    while((n = stringGetLine(fp, line, len)) != EOF)
    {
	(*line_no)++;
	/* skip spaces */
	for(c = line; *c != '\0' && isspace((int)*c); c++);

	if (strstr(line, "ignore") && strstr(line, "this") &&
	    strstr(line, "file"))
	{
	    return(EOF);
	}

	if(*c != '!' && *c != '#' && *c != '\0') return(n);
    }
    return(n);
}

/* try to read default values for symbol, color and size
 */
void MapOverlay::getOverlayDefaults(char * buf, const char *overlay_name,
		SymbolInfo *default_sym, Pixel fg)
{
    SymbolInfo symbol_info_init = SYMBOL_INFO_INIT;
    string msg;
    char *str;

    *default_sym = symbol_info_init;

    if((str = stringGetArg(buf, "symbol")) != NULL)
    {
	default_sym->type = getSymType(str);
	free(str);
    }
    default_sym->fg = fg;
    if((str = stringGetArg(buf, "color")) != NULL)
    {
	default_sym->fg = map->stringToPixel(str);
	free(str);
    }
    parseGetArg(buf, overlay_name, msg, "size", &default_sym->size);
}

void MapOverlay::getLineInfo(char *buf, Pixel bg, Pixel fg, LineInfo *line)
{
    char *str;
    string msg;
    LineInfo line_info_init = LINE_INFO_INIT;

    *line = line_info_init;
    line->fg = fg;
    line->bg = bg;
    if((str = stringGetArg(buf, "color")) != NULL)
    {
	line->fg = mw->getColor(str);
	free(str);
    }
    if((str = stringGetArg(buf, "dcolor")) != NULL)
    {
	line->bg = mw->getColor(str);
	free(str);
    }
    parseGetArg(buf, "line", msg, "width", &line->line_width);
    line->display = MAP_ON;
    if((str = stringGetArg(buf, "dashes")) != NULL)
    {
	line->n_dashes = getDashLen(str);
    }
    if(line->n_dashes > 0)
    {
	line->dashes = (char *)mallocWarn(line->n_dashes);
	getDashes(str, line->dashes);
	line->line_style = (line->bg == bg) ?  LineOnOffDash :LineDoubleDash;
    }
    Free(str);
    if((str = stringGetArg(buf, "join_style")) != NULL)
    {
	if(!strcmp(str, "JoinMiter")) line->join_style = JoinMiter;
	else if(!strcmp(str, "JoinRound")) line->join_style = JoinRound;
	else if(!strcmp(str, "JoinBevel")) line->join_style = JoinBevel;
	free(str);
    }
    if((str = stringGetArg(buf, "cap_style")) != NULL)
    {
	if(!strcmp(str, "CapButt")) line->cap_style = CapButt;
	else if(!strcmp(str, "CapRound")) line->cap_style = CapRound;
	else if(!strcmp(str, "CapNotLast")) line->cap_style= CapNotLast;
	else if(!strcmp(str, "CapProjecting")) line->cap_style = CapProjecting;
	free(str);
    }
}

int MapOverlay::getDashLen(char *dashes_list)
{
    int     i, ndash;

    if(dashes_list == (char *)NULL || (int)strlen(dashes_list) < 1) return(0);

    /* find max number of dashes by counting commans  */

    ndash = i = 0;
    while (dashes_list[i] != '\n' && dashes_list[i] != '\0')
    {
	if(dashes_list[i] == ',') ndash++;
	i++;
    }
    ndash++;
    return(ndash);
}

void MapOverlay::getDashes(char *in_dashes, char *out_dashes)
{
    char    val[5];
    int     i, j, n;

    j = i = n = 0;
    while (in_dashes[i] != '\n' && in_dashes[i] != '\0')
    {
	if(in_dashes[i] == ',')
	{
	    val[j] = '\0';
	    out_dashes[n++] = atoi(val);
	    j=0;
	}
	else
	{
	    val[j++] = in_dashes[i];
	}
	i++;
    }
    out_dashes[n] = atoi(val);
}

bool MapOverlay::isCommand(const char *buf)
{
    int i, j, n;

    for(i = 0; isspace((int)buf[i]); i++);

    j = i;
    for(n = 0; !isspace((int)buf[i]); i++, n++);

    if(	!strncasecmp(buf+j, "stations", n) ||
	!strncasecmp(buf+j, "sources", n) ||
	!strncasecmp(buf+j, "arcs", n) ||
	!strncasecmp(buf+j, "circles", n) ||
	!strncasecmp(buf+j, "ellipses", n) ||
	!strncasecmp(buf+j, "assoc", n) ||
	!strncasecmp(buf+j, "line", n) ||
	!strncasecmp(buf+j, "symbols", n) ||
	!strncasecmp(buf+j, "polygon", n) ||
	!strncasecmp(buf+j, "limits", n) ||
	!strncasecmp(buf+j, "rotate", n) ||
	!strncasecmp(buf+j, "delete", n) ||
	!strncasecmp(buf+j, "projection", n) ||
	!strncasecmp(buf+j, "display", n)) return true;

    return false;
}

int MapOverlay::getSymType(const char *type)
{
    if((int)strlen(type) < 1)			return(FILLED_TRIANGLE);
    else if(!strcasecmp(type, "SQUARE"))	return(SQUARE);
    else if(!strcasecmp(type, "TRIANGLE"))	return(TRIANGLE);
    else if(!strcasecmp(type, "PLUS"))		return(PLUS);
    else if(!strcasecmp(type, "EX"))		return(EX);
    else if(!strcasecmp(type, "INV_TRIANGLE"))  return(INV_TRIANGLE);
    else if(!strcasecmp(type, "DIAMOND"))	return(DIAMOND);
    else if(!strcasecmp(type, "CIRCLE"))	return(CIRCLE);
    else if(!strcasecmp(type, "FILLED_SQUARE"))	return(FILLED_SQUARE);
    else if(!strcasecmp(type, "FILLED_TRIANGLE"))return(FILLED_TRIANGLE);
    else if(!strcasecmp(type,"FILLED_INV_TRIANGLE"))return(FILLED_INV_TRIANGLE);
    else if(!strcasecmp(type, "FILLED_DIAMOND"))return(FILLED_DIAMOND);
    else if(!strcasecmp(type, "FILLED_CIRCLE")) return(FILLED_CIRCLE);
    return(FILLED_TRIANGLE);
}
