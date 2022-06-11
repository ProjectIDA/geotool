#ifndef _IASPEI_H_
#define _IASPEI_H_

/**
 *  @ingroup libgx
 */
class Iaspei
{
    public:
	Iaspei();
	Iaspei(const string &iaspei_file_prefix);
	Iaspei(const string &iaspei_file_prefix, double depth);
	~Iaspei(void);

	void setDepth(float depth);
	int travelTime(const string &phase_name, float delta, float depth,
			float *tt, float *ray_p, float *dtdd, float *dtdh,
			float *dddp);
	float travelTime(const string &phase_name, float delta, float depth);
	float travelTime(const string &phase_name, float delta);
	int getCurve(const string &phase, double depth, int *npts, float *tt,
			float *dist, float *ray_p);


	double stop_Pdiff;
	double t_vel;
	double lr_vel;
	double rg_vel;
	double lg_vel;
	double lq_vel;

    protected:
	void init(const string &iaspei_file_prefix, double depth);

    private:

};

#endif
