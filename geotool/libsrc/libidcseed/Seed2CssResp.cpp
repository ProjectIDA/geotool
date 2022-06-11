/** \file Seed2CssRespResp.cpp
 *  \brief Defines class Seed2CssRespResp.
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>

#include "seed/Seed2CssResp.h"
#include "seed/SeedInput.h"

#ifndef _FCOMPLEX_
#define _FCOMPLEX_
typedef struct fcomplex {
        float re;
        float im;
} FComplex;
#endif


string Seed2CssResp::cssResponse(Station &sta, Channel &chan, Dictionary &d)
{
    double a0_norm_freq = 1.0;
    double calib = 1.0;
    double scaled_sens = 1.0;
    double sensitivity_freq = 0.0;
    string digitizer = "";
    string b53_type = "";
    Blockette33 *b33;
    Blockette53 *b53;
    Blockette54 *b54, *b54b;
    Blockette55 *b55;
    Blockette57 *b57;
    Blockette58 *b58;
    Blockette61 *b61;
    ostringstream os_header, os_start_header, os_data;
    char line[1000];
    int stage = 0;

    for(int i = 0; i < (int)chan.response.size(); i++) {
	if( (b53 = chan.response[i]->getBlockette53()) ) {
	    b53_type = b53->type;
	    process53(d, os_header, os_data, *b53, &calib);
	    stage = b53->stage;
	    a0_norm_freq = b53->norm_freq;
	}
	else if( (b54 = chan.response[i]->getBlockette54()) ) {
	    b54b = NULL;
	    if(i < (int)chan.response.size()-1) b54b = chan.response[i+1]->getBlockette54();
	    process54(d, os_header, os_data, *b54, b54b, chan);
	    stage = b54->stage;
	    if(b54b != NULL) i++;
	}
	else if( (b55 = chan.response[i]->getBlockette55()) ) {
	    process55(d, os_header, os_data, *b55, a0_norm_freq, &calib);
	    stage = b55->stage;
	}
	else if( (b57 = chan.response[i]->getBlockette57()) ) {
	    process57(d, os_header, *b57);
	    stage = b57->stage;
	}
	else if( (b58 = chan.response[i]->getBlockette58()) ) {
	    process58(os_header, *b58, stage, &scaled_sens, sensitivity_freq);
	    stage = b58->stage;
	    if(b58->stage == 0) {
		snprintf(line, sizeof(line), "# Displacement response for %s station %s\n#\n",
			sta.b50.network.c_str(), sta.b50.station.c_str());
		os_start_header << line;

    		string insname = (b33 = d.getB33(chan.b52.instrument)) ? b33->description : "";

		snprintf(line, sizeof(line), "# Seismometer type      = %s\n", insname.c_str());
		os_start_header << line;
		snprintf(line, sizeof(line), "# Digitizer type        = %s\n", digitizer.c_str());
		os_start_header << line;
		snprintf(line, sizeof(line), "# Data sample rate      = %g s/s\n#\n#\n", chan.b52.sample_rate);
		os_start_header << line;
		snprintf(line, sizeof(line), "# One zero has been added to convert velocity to displacement,\n");
		os_start_header << line;
		snprintf(line, sizeof(line), "# and two zeros have been added to convert acceleration to displacement.\n");
		os_start_header << line;
		snprintf(line, sizeof(line), "# Normalization A0 is calculated for displacement at %g Hz.\n#\n", a0_norm_freq);
		os_start_header << line;
		snprintf(line, sizeof(line), "# Following comments are extracted for reference purpose.\n");
		os_start_header << line;
		snprintf(line, sizeof(line), "#----------------------------------------------------\n");
		os_start_header << line;
		snprintf(line, sizeof(line), "# The sensitivity of channel is %6.4f counts/(nm/s)\n", scaled_sens*1.e-09);
		os_start_header << line;
		snprintf(line, sizeof(line), "# at frequency of %4.2f Hz\n#\n", sensitivity_freq);
		os_start_header << line;
	    }
	    else if(sensitivity_freq == 0.) {
		sensitivity_freq = b58->frequency;
	    }
	}
/*
	else if( (b59 = chan.response[i]->getBlockette59()) ) {
	    process59(d, os_header, os_data, *b59);
	}
*/
	else if( (b61 = chan.response[i]->getBlockette61()) ) {
	    process61(d, os_header, os_data, *b61, b53_type, chan);
	    stage = b61->stage;
	    digitizer = b61->name;
	}
    }
    return os_start_header.str() + os_header.str() + os_data.str();
}

void Seed2CssResp::process53(Dictionary &d, ostringstream &os_header, ostringstream &os_data, Blockette53 &b53, double *calib)
{
    char line[1000];
    int npoles = (int)b53.pr.size();
    FComplex *pole = (FComplex *)malloc(npoles*sizeof(FComplex));
    FComplex *pole_err = (FComplex *)malloc(npoles*sizeof(FComplex));
    double fac = (b53.type == "B") ? 2*M_PI : 1.0;

    for(int j = 0; j < npoles; j++) {
	pole[j].re = b53.pr[j] * fac;
	pole[j].im = b53.pi[j] * fac;
	pole_err[j].re = b53.pr_error[j];
	pole_err[j].im = b53.pi_error[j];
    }
    int nzeros = (int)b53.zr.size();
    int nadd = 0;
    if(nzeros > 0) {
	Blockette34 *b34;
	if( (b34 = d.getB34(b53.input_units)) && !strcasecmp(b34->name.c_str(), "M/S")) {
	    // add one zero
	    nadd = 1;
	    *calib = 2.*M_PI*b53.norm_freq;
	}
	else if( (b34 = d.getB34(b53.input_units)) && !strcasecmp(b34->name.c_str(), "M/S**2")) {
	    // add two zeros
	    nadd = 2;
	    *calib = 4.*M_PI*M_PI*b53.norm_freq*b53.norm_freq;
	}
	else {
	    *calib = 1.0;
	}
    }
    nzeros = (int)b53.zr.size();
    FComplex *zero = (FComplex *)malloc((nzeros+nadd)*sizeof(FComplex));
    FComplex *zero_err = (FComplex *)malloc((nzeros+nadd)*sizeof(FComplex));
    for(int j = 0; j < nzeros; j++) {
	zero[j].re = b53.zr[j] * fac;
	zero[j].im = b53.zi[j] * fac;
	zero_err[j].re = b53.zr_error[j];
	zero_err[j].im = b53.zi_error[j];
    }
    for(int j = 0; j < nadd; j++) {
	zero[nzeros+j].re = 0.0;
	zero[nzeros+j].im = 0.0;
	zero_err[nzeros+j].re = 0.0;
	zero_err[nzeros+j].im = 0.0;
    }
    nzeros += nadd;

    // compute A0
    double num_re=1.0, num_im=0.0, re, im, fre=0., fim;
    fim = 2.*M_PI*b53.norm_freq;

    for(int j = 0; j < nzeros; j++) {
	re = num_re*(fre + zero[j].re) - num_im*(fim + zero[j].im);
	im = num_im*(fre + zero[j].re) + num_re*(fim + zero[j].im);
	num_re = re;
	num_im = im;
    }
    double dnom_re=1.0, dnom_im=0.0;
    for(int j = 0; j < npoles; j++) {
	re = dnom_re*(fre + pole[j].re) - dnom_im*(fim + pole[j].im);
	im = dnom_im*(fre + pole[j].re) + dnom_re*(fim + pole[j].im);
	dnom_re = re;
	dnom_im = im;
    }
    double a0 = sqrt(dnom_re*dnom_re + dnom_im*dnom_im)/sqrt(num_re*num_re + num_im*num_im);

    snprintf(line, sizeof(line), "#  stage-%d\n", b53.stage);
    os_header << line;
    snprintf(line, sizeof(line), "#     Response type:                  A Laplace Transform (Rad/sec)\n");
    os_header << line;
    snprintf(line, sizeof(line), "#     Response in units:              %s\n", d.getB34(b53.input_units)->name.c_str());
    os_header << line;
    snprintf(line, sizeof(line), "#     Response out units:             %s\n", d.getB34(b53.output_units)->name.c_str());
    os_header << line;
    snprintf(line, sizeof(line), "#     A0 normalization factor:        %12E\n", a0);
    os_header << line;
    snprintf(line, sizeof(line), "#     N normalization frequency:      %12E\n", b53.norm_freq);
    os_header << line;
    if(nzeros > 0 || npoles > 0) {
	if(b53.type == "D") {
	    snprintf(line, sizeof(line), " theoretical  %d    digitizer paz\n", b53.stage);
	    os_data << line;
	}
	else {
	    snprintf(line, sizeof(line), " theoretical  %d   instrument paz\n", b53.stage);
	    os_data << line;
	}
	snprintf(line, sizeof(line), "%13E\n",a0);
	os_data << line;

	snprintf(line, sizeof(line), "%d\n", npoles);
	os_data << line;

	for(int j = 0; j < npoles; j++) {
	    snprintf(line, sizeof(line), "%13E   %13E    %8E   %8E\n",
		pole[j].re, pole[j].im, pole_err[j].re, pole_err[j].im);
	    os_data << line;
	}
	snprintf(line, sizeof(line), "%d\n", (int)nzeros);
	os_data << line;

	for(int j = 0; j < nzeros; j++) {
	    snprintf(line, sizeof(line), "%13E   %13E    %8E   %8E\n",
		zero[j].re, zero[j].im, zero_err[j].re, zero_err[j].im);
	    os_data << line;
	}
    }
 
    free(pole);
    free(pole_err);
    free(zero);
    free(zero_err);
}

void Seed2CssResp::process54(Dictionary &d, ostringstream &os_header, ostringstream &os_data, Blockette54 &b54,
				Blockette54 *b54b, Channel &chan)
{
    char line[1000];

    snprintf(line, sizeof(line), "#  stage-%d\n", b54.stage);
    os_header << line;
    if(b54.type == "A") {
	snprintf(line, sizeof(line), "#     Response type:                  A  Laplace Transform (Rad/sec)\n");
    }
    else {
	snprintf(line, sizeof(line), "#     Response type:                  %s\n", b54.type.c_str());
    }
    os_header << line;
    snprintf(line, sizeof(line), "#     Response in units:              %s\n", d.getB34(b54.input_units)->name.c_str());
    os_header << line;
    snprintf(line, sizeof(line), "#     Response out units:             %s\n", d.getB34(b54.output_units)->name.c_str());
    os_header << line;

    int num_n = (int)b54.numerator.size();
    int num_d = (int)b54.denominator.size();
    if(b54b != NULL) {
	// can have two 54 blockettes
	num_n += b54b->numerator.size();
	num_d += b54b->denominator.size();
    }

    if(num_n > 0 || num_d > 0) {
	if(b54.type == "D") {
	    snprintf(line, sizeof(line), " theoretical  %d    digitizer fir\n", b54.stage);
	}
	else {
	    snprintf(line, sizeof(line), " theoretical  %d   instrument fir\n", b54.stage);
	}
	os_data << line;

	int j;
	Blockette57 *b57;
	for(j = 0; j < (int)chan.response.size(); j++) {
	    if( (b57 = chan.response[j]->getBlockette57()) ) break;
	}
	if(j == (int)chan.response.size()) {
	    fprintf(stderr, "missing blockette 057\n");
	    return;
	}
	snprintf(line, sizeof(line), "%g\n", b57->input_sample_rate);
	os_data << line;

	snprintf(line, sizeof(line), "%d\n", num_n);
	os_data << line;
	for(j = 0; j < (int)b54.numerator.size(); j++) {
	    snprintf(line, sizeof(line), "%13E    %13E\n", b54.numerator[j], b54.nerror[j]);
	    os_data << line;
	}
	if(b54b != NULL) {
	    for(j = 0; j < (int)b54b->numerator.size(); j++) {
		snprintf(line, sizeof(line), "%13E    %13E\n", b54b->numerator[j], b54b->nerror[j]);
		os_data << line;
	    }
	}
	snprintf(line, sizeof(line), "%d\n", num_d);
	os_data << line;
	for(j = 0; j < (int)b54.denominator.size(); j++) {
	    snprintf(line, sizeof(line), "%13E    %13E\n", b54.denominator[j], b54.derror[j]);
	    os_data << line;
	}
	if(b54b != NULL) {
	    for(j = 0; j < (int)b54b->denominator.size(); j++) {
		snprintf(line, sizeof(line), "%13E    %13E\n", b54b->denominator[j], b54b->derror[j]);
		os_data << line;
	    }
	}
    }
}

void Seed2CssResp::process55(Dictionary &d, ostringstream &os_header, ostringstream &os_data, Blockette55 &b55,
			double norm_freq, double *calib)
{
    char line[1000];
    string input_units="", output_units="";
    Blockette34 *b34;

    if( (b34 = d.getB34(b55.input_units)) ) input_units = b34->name;
    if( (b34 = d.getB34(b55.output_units)) ) output_units = b34->name;

    if( !strcasecmp(input_units.c_str(), "M/S")) {
	*calib = 2.*M_PI*norm_freq;
    }
    else if( !strcasecmp(output_units.c_str(), "M/S**2")) {
	*calib = 4.*M_PI*M_PI*norm_freq*norm_freq;
    }
    else {
	*calib = 1.0;
    }
    snprintf(line, sizeof(line), "#  stage-%d\n", b55.stage);
    os_header << line;
    snprintf(line, sizeof(line), "#     Response in units:              %s\n", input_units.c_str());
    os_header << line;
    snprintf(line, sizeof(line), "#     Response out units:             %s\n", output_units.c_str());
    os_header << line;

    int nfap = (int)b55.frequency.size();
    if(nfap > 0) {
	snprintf(line, sizeof(line), " theoretical  %d   instrument fap\n", b55.stage);
	os_data << line;
	snprintf(line, sizeof(line), "%d\n", nfap);
	os_data << line;
	for(int i = 0; i < nfap; i++) {
	    snprintf(line, sizeof(line), "%13E  %13E  %13E   %8E  %8E\n", b55.frequency[i],  b55.amplitude[i],
			b55.phase[i], b55.amp_error[i], b55.phase_error[i]);
	    os_data << line;
	}
    }
}

void Seed2CssResp::process57(Dictionary &d, ostringstream &os_header, Blockette57 &b57)
{
    char line[1000];
    snprintf(line, sizeof(line), "#     Response input sampling rate:   %g\n", b57.input_sample_rate);
    os_header << line;
    snprintf(line, sizeof(line), "#     Response decimation factor:     %d\n", b57.decimation_factor);
    os_header << line;
    snprintf(line, sizeof(line), "#     Response decimation offset:     %d\n", b57.decimation_offset);
    os_header << line;
    snprintf(line, sizeof(line), "#     Response delay:                 %12E\n", b57.delay);
    os_header << line;
    snprintf(line, sizeof(line), "#     Response correction:            %12E\n", b57.correction);
    os_header << line;
}

void Seed2CssResp::process58(ostringstream &os_header, Blockette58 &b58, int stage, double *scaled_sens,
				double sensitivity_freq)
{
    char line[1000];

    if(b58.stage != 0) {
	*scaled_sens *= b58.sensitivity;

	if(b58.stage != stage) {
	    snprintf(line, sizeof(line), "#  stage-%d\n", b58.stage);
	    os_header << line;
	}
	snprintf(line, sizeof(line), "#     Sensitivity:                    %12E\n", b58.sensitivity);
	os_header << line;
	snprintf(line, sizeof(line), "#     Frequency of sensitivity:       %12E\n#\n", sensitivity_freq);
	os_header << line;
    }
}

void Seed2CssResp::process61(Dictionary &d, ostringstream &os_header, ostringstream &os_data, Blockette61 &b61,
				string b53_type, Channel &chan)
{
    char line[1000];
    int j, num_n = 0;
    float *fir_n=NULL;

    snprintf(line, sizeof(line), "#  stage-%d\n", b61.stage);
    os_header << line;
    snprintf(line, sizeof(line), "#     Response type:                  %s Laplace Transform (Rad/sec)\n", b53_type.c_str());
    os_header << line;
    snprintf(line, sizeof(line), "#     Response in units:              %s\n", d.getB34(b61.input_units)->name.c_str());
    os_header << line;
    snprintf(line, sizeof(line), "#     Response out units:             %s\n", d.getB34(b61.output_units)->name.c_str());
    os_header << line;

    if(b61.symmetry_code == "A") {
	num_n = (int)b61.coef.size();
	fir_n = (float *)malloc(num_n*sizeof(float));
	for(j = 0; j < num_n; j++) {
	    fir_n[j] = b61.coef[j];
	}
    }
    else if(b61.symmetry_code == "B") {
	int n = (int)b61.coef.size();
	num_n = 2*n - 1;
	fir_n = (float *)malloc(num_n*sizeof(float));
	for(j = 0; j < n-1; j++) {
	    fir_n[j] = b61.coef[j];
	    fir_n[num_n-1-j] = b61.coef[j];
	}
	fir_n[n-1] = b61.coef[n-1];
    }
    else if(b61.symmetry_code == "C") {
	int n = (int)b61.coef.size();
	num_n = 2*n;
	fir_n = (float *)malloc(num_n*sizeof(float));
	for(j = 0; j < n; j++) {
	    fir_n[j] = b61.coef[j];
	    fir_n[n] = b61.coef[j];
	}
    }
    if(num_n > 0) {
	Blockette57 *b57;
	for(j = 0; j < (int)chan.response.size(); j++) {
	    if( (b57 = chan.response[j]->getBlockette57()) ) break;
	}
	if(j == (int)chan.response.size()) {
	    fprintf(stderr, "missing blockette 057\n");
	    return;
	}
	if(b53_type == "D") snprintf(line, sizeof(line), " theoretical  %d    digitizer fir\n", b61.stage);
	else snprintf(line, sizeof(line)," theoretical  %d   instrument fir\n", b61.stage);
	os_data << line;
	snprintf(line, sizeof(line), "%g\n", b57->input_sample_rate);
	os_data << line;
	snprintf(line, sizeof(line), "%d\n", num_n);
	os_data << line;
	for(j = 0; j < num_n; j++) {
	    snprintf(line, sizeof(line), "%13E    0.00E+00\n", fir_n[j]);
	    os_data << line;
	}
	snprintf(line, sizeof(line), "0\n");
	os_data << line;
    }
    free(fir_n);
}
