#ifndef WAVEFORM_CONSTRAINT_H
#define WAVEFORM_CONSTRAINT_H

#include <vector>
#include "gobject++/gvector.h"
#include "gobject++/SegmentInfo.h"
using namespace std;

class Waveform;
class DataSource;

 
/**
 *  @ingroup libgx
 */
class WaveInput : public Gobject
{
    public :
	WaveInput(double start, double end, double t0);
	~WaveInput(void);

	double  start;
	double  end;
	double  t0;
	gvector<SegmentInfo *> segments;
};

/**
 *  @ingroup libgx
 */
class WaveformConstraint
{
    public:
	WaveformConstraint();
	~WaveformConstraint(void);

	double	overlap_limit;
	double	join_time_limit;
	double	start_time;
	double	end_time;
	double	window_length;
	double	max_length;
	bool	azCon;
	double	minAz;
	double	maxAz;
	char	startPhase[20];
	char	endPhase[20];
	double	start_phase_edge;
	double	end_phase_edge;
	int	waveform_limit;

	char	chan_sort_order0[50];
	char	chan_sort_order1[50];
	char	chan_sort_order2[50];

	vector<WaveInput *> *getTimeLimits(int num_selected, SegmentInfo **wav,
				vector<ArrivalInfo> &arr_info);
	int readWaveforms(DataSource *ds, gvector<SegmentInfo *> *segs,
				gvector<Waveform *> &wvec);

    protected:
	static bool checkAz(SegmentInfo *wav, double minAz, double maxAz);
	static double phaseTime(char *phase_code, SegmentInfo *wav,
			vector<ArrivalInfo> &arr_info, int *status);
	static int firstObs(char *net, char *phase, double *time,
			vector<ArrivalInfo> &arr_info);
};

#endif
