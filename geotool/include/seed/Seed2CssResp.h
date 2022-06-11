/** \file Seed2CssResp.h
 *  \brief Declares the Seed2CssResp class members and methods.
 *  \author Ivan Henson
 */
#ifndef _SEED_2_CSS_RESP_H
#define _SEED_2_CSS_RESP_H

#include "seed/Seed.h"
#include "seed/Blockettes.h"
#include "seed/Station.h"
#include "seed/SeedData.h"
#include "seed/Dictionary.h"

/**
 *  A Seed2CssResp is a class for converting Seed response information to CSS response file format.
 *
 *@see Seed
 */
class Seed2CssResp
{
    public:
	static string cssResponse(Station &sta, Channel &chan, Dictionary &d);

    protected:
	static void process53(Dictionary &d, ostringstream &os_header, ostringstream &os_data, Blockette53 &b53, double *calib);
	static void process54(Dictionary &d, ostringstream &os_header, ostringstream &os_data, Blockette54 &b54,
			Blockette54 *b54b, Channel &chan);
	static void process55(Dictionary &d, ostringstream &os_header, ostringstream &os_data, Blockette55 &b55,
			double norm_freq, double *calib);
	static void process57(Dictionary &d, ostringstream &os_header, Blockette57 &b57);
	static void process58(ostringstream &os_header, Blockette58 &b58, int stage, double *scaled_sens,
			double sensitivity_freq);
	static void process61(Dictionary &d, ostringstream &os_header, ostringstream &os_data, Blockette61 &b61,
			string b53_type, Channel &chan);
};

#endif
