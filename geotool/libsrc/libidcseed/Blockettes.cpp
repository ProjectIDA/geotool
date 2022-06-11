/** \file Blockettes.cpp
 *  \brief Defines blockette parsing methods
 *  \author Ivan Henson
 */
#include "config.h"
#include <string.h>
#include <iostream>
#include "seed/Blockettes.h"
#include "seed/ByteOrder.h"
using namespace std;

void Blockette5::load(const string &blockette) throw(FormatException)
{
    version_of_format = trim(blockette.substr(0, 4));
    logical_record_length = parseInt(blockette.substr(4, 6), "005.logical_record_length");
    string v = variable(blockette.substr(6));
    beginning_time = SeedTime(v, "005.beginning_time");
}

string Blockette5::str()
{
    ostringstream out;
    out << "version=" << version_of_format << " "
	<< "reclen=" << logical_record_length << " "
	<< "beg=" << beginning_time.str();
    return out.str();
}

void Blockette8::load(const string &blockette) throw(FormatException)
{
    int n;
    string v;

    version_of_format = trim(blockette.substr(0, 4));
    logical_record_length = parseInt(blockette.substr(4, 2), "008.logical_record_length");

    station = trim(blockette.substr(6, 5));
    location = trim(blockette.substr(11, 2));
    channel = trim(blockette.substr(13, 3));
    n = 16;
    v = variable(blockette.substr(n));
    beginning_time = SeedTime(v, "008.beginning_time");

    n += v.length() + 1;
    v = variable(blockette.substr(n));
    end_time = SeedTime(v, "008.end_time");

    n += v.length() + 1;
    v = variable(blockette.substr(n));
    station_date = SeedTime(v, "008.station_date");

    n += v.length() + 1;
    v = variable(blockette.substr(n));
    channel_date = SeedTime(v, "008.channel_date");

    n += v.length() + 1;
    network = trim(blockette.substr(n, 2));
}

string Blockette8::str()
{
    ostringstream out;
    out << "version=" << version_of_format << " "
	<< "reclen=" << logical_record_length << " "
	<< "sta=" << station << " "
	<< "loc=" << location << " "
	<< "chan=" << channel << " "
	<< "beg=" << beginning_time.str() << " "
	<< "end=" << end_time.str() << " "
	<< "sta_date=" << station_date.str() << " "
	<< "chan_date=" << channel_date.str() << " "
	<< "net=" << network;
    return out.str();
}

string Blockette8::longStr()
{
    ostringstream out;
    out << "version of format:      " << version_of_format << endl
	<< "logical record length:  " << logical_record_length << endl
	<< "station identifier:     " << station << endl
	<< "location identifier:    " << location << endl
	<< "channel identifier:     " << channel << endl
	<< "beginning of volume:    " << beginning_time.str() << endl
	<< "end of volume:          " << end_time.str() << endl
	<< "station effective date: " << station_date.str() << endl
	<< "channel effective date: " << channel_date.str() << endl
	<< "network code:           " << network;
    return out.str();
}

void Blockette10::load(const string &blockette) throw(FormatException)
{
    int n;
    string v;

    version_of_format = trim(blockette.substr(0, 4));
    logical_record_length = parseInt(blockette.substr(4, 2),
				"010.logical_record_length");

    n = 6;
    v = variable(blockette.substr(n));
    beginning_time = SeedTime(v, "010.beginning_time");

    n += v.length() + 1;
    v = variable(blockette.substr(n));
    end_time = SeedTime(v, "010.end_time");

    n += v.length() + 1;
    v = variable(blockette.substr(n));
    volume_time = SeedTime(v, "010.volume_time");

    n += v.length() + 1;
    organization = trim(variable(blockette.substr(n)));

    n += organization.length() + 1;
    label = trim(variable(blockette.substr(n)));
}

string Blockette10::str()
{
    ostringstream out;
    out << "version=" << version_of_format << " "
	<< "reclen=" << logical_record_length << " "
	<< "beg=" << beginning_time.str() << " "
	<< "end=" << end_time.str() << " "
	<< "vtime=" << volume_time.str() << " "
	<< "org=" << organization << " "
	<< "lab=" << label;
    return out.str();
}

string Blockette10::longStr()
{
    ostringstream out;
    out << "version of format:     " << version_of_format << endl
	<< "logical record length: " << logical_record_length << endl
	<< "beginning time:        " << beginning_time.str() << endl
	<< "end time:              " << end_time.str() << endl
	<< "volume time:           " << volume_time.str() << endl
	<< "organization:          " << organization << endl
	<< "label:                 " << label;
    return out.str();
}

void Blockette11::load(const string &blockette) throw(FormatException)
{
    int i, j, num;

    num = parseInt(blockette.substr(0, 3), "011.numStations");

    for(i = 0; i < num; i++) {
	j = 3 + i*11;
	station.push_back(trim(blockette.substr(j, 5)));
	seqno.push_back(parseInt(blockette.substr(j+5, 6), "011.seqno"));
    }
}

string Blockette11::str()
{
    ostringstream out;
    out << "num_sta=" << (int)station.size();
    return out.str();
}

string Blockette11::longStr()
{
    char s[50];
    ostringstream out;
    out << "num_sta: " << (int)station.size();
    if((int)station.size() > 0) {
	out << endl << "  sta    seqno";
	for(int i = 0; i < (int)station.size(); i++) {
	    snprintf(s, sizeof(s),"\n  %-5.5s  %d",station[i].c_str(),seqno[i]);
	    out << s;
	}
    }
    return out.str();
}

void Blockette12::load(const string &blockette) throw(FormatException)
{
    int i, j, num;
    string v;

    num = parseInt(blockette.substr(0,4), "012.numSpans");

    for(i = 0, j = 4; i < num; i++) {
	v = variable(blockette.substr(j));
	beg.push_back(SeedTime(v, "012.beginning_span"));
	j += v.length() + 1;
	v = variable(blockette.substr(j));
	end.push_back(SeedTime(v, "012.end_span"));
	j += v.length() + 1;
	seqno.push_back(parseInt(blockette.substr(j, 6), "012.seqno"));
	j += 6;
    }
}

string Blockette12::str()
{
    ostringstream out;
    out << "num_spans=" << (int)beg.size();
    return out.str();
}

string Blockette12::longStr()
{
    ostringstream out;
    out << "num_spans: " << (int)beg.size();
    if((int)beg.size() > 0) {
	out << endl
	 << "beginning of span       end of span             sequence number";
	for(int i = 0; i < (int)beg.size(); i++) {
	    out<<endl<<beg[i].str() << "  " << end[i].str() << "  " << seqno[i];
	}
    }
    return out.str();
}

void Blockette30::load(const string &blockette) throw(FormatException)
{
    int i, j, num_keys;
    string v;

    name = variable(blockette.substr(0));
    j = name.length() + 1;
    lookup_code = parseInt(blockette.substr(j, 4), "030.code");
    j += 4;
    family_type = parseInt(blockette.substr(j, 3), "030.familyType");
    j += 3;
    num_keys = parseInt(blockette.substr(j, 2), "030.numKeys");
    j += 2;

    for(i = 0; i < num_keys; i++) {
	v = variable(blockette.substr(j));
	keys.push_back(v);
	j += v.length() + 1;
    }
}

string Blockette30::str()
{
    ostringstream out;
    out << "name=" << name << " "
	<< "code=" << lookup_code << " "
	<< "family=" << family_type << " "
	<< "num_keys=" << (int)keys.size();
    return out.str();
}

string Blockette30::longStr()
{
    ostringstream out;
    out << "name: " << name << endl
	<< "format code: " << lookup_code << "  num_keys: " <<(int)keys.size();
    if((int)keys.size() > 0) {
	for(int i = 0; i < (int)keys.size(); i++) {
	    out << endl << "     " << keys[i];
	}
    }
    return out.str();
}

void Blockette31::load(const string &blockette) throw(FormatException)
{
    int j;

    lookup_code = parseInt(blockette.substr(0, 4), "031.commentCode");
    class_code = blockette.substr(4, 1);

    comment = variable(blockette.substr(5));
    j = 5 + comment.length() + 1;
    units = parseInt(blockette.substr(j, 3), "031.units");
}

string Blockette31::str()
{
    ostringstream out;
    out << "code=" << lookup_code << " "
	<< "class=" << class_code << " "
	<< "comment=" << comment << " "
	<< "units=" << units;
    return out.str();
}

void Blockette32::load(const string &blockette) throw(FormatException)
{
    int j;

    lookup_code = parseInt(blockette.substr(0, 2), "032.lookupCode");
    author = variable(blockette.substr(2));
    j = 2 + author.length() + 1;
    catalog = variable(blockette.substr(j));
    j += catalog.length() + 1;
    publisher = variable(blockette.substr(j));
}

string Blockette32::str()
{
    ostringstream out;
    out << "code=" << lookup_code << " "
	<< "author=" << author << " "
	<< "date=" << catalog << " "
	<< "publisher=" << publisher;
    return out.str();
}

void Blockette33::load(const string &blockette) throw(FormatException)
{
    lookup_code = parseInt(blockette.substr(0, 3), "033.lookUpCode");
    description = variable(blockette.substr(3));
}

string Blockette33::str()
{
    ostringstream out;
    out << "code=" << lookup_code << " "
	<< "description=" << description;
    return out.str();
}

void Blockette34::load(const string &blockette) throw(FormatException)
{
    int j;

    lookup_code = parseInt(blockette.substr(0, 3), "034.lookUpCode");
    name = variable(blockette.substr(3));
    j = 3 + name.length() + 1;
    description = variable(blockette.substr(j));
}

string Blockette34::str()
{
    ostringstream out;
    out << "code=" << lookup_code << " "
	<< "name=" << name << " "
	<< "description=" << description;
    return out.str();
}

void Blockette35::load(const string &blockette) throw(FormatException)
{
    int i, j, num_components;

    lookup_code = parseInt(blockette.substr(0, 3), "035.lookUpCode");
    num_components = parseInt(blockette.substr(3, 4), "035.numComponents");

    for(i = 0, j = 7; i < num_components; i++) {
	station.push_back(trim(blockette.substr(j, 5))); j += 5;
	location.push_back(trim(blockette.substr(j, 2))); j += 2;
	channel.push_back(trim(blockette.substr(j, 3))); j += 3;
	subchannel.push_back(parseInt(blockette.substr(j, 4), "035.subchannel")); j += 4;
	weight.push_back(parseDouble(blockette.substr(j, 5), "035.weight")); j += 5;
    }
}

string Blockette35::str()
{
    ostringstream out;
    out << "code=" << lookup_code << " "
	<< "num_components=" << (int)station.size();
    return out.str();
}

string Blockette35::longStr()
{
    char s[50];
    ostringstream out;
    out << "beam lookup code:     " << lookup_code << endl
	<< "number of components: " << (int)station.size() << endl;
    if((int)station.size() > 0) {
	out << endl << "  sta    loc  chan  sub-chan  weight";
	for(int i = 0; i < (int)station.size(); i++) {
	    snprintf(s, sizeof(s),
		"\n  %-5.5s  %-2.2s   %-3.3s   %4d     %5.3f",
		station[i].c_str(), location[i].c_str(), channel[i].c_str(),
		subchannel[i], weight[i]);
	    out << s;
	}
    }
    return out.str();
}

void Blockette41::load(const string &blockette) throw(FormatException)
{
    int i, j, num;

    lookup_code = parseInt(blockette.substr(0, 4), "041.lookupCode");
    name = variable(blockette.substr(4));
    j = 4 + name.length() + 1;
    symmetry_code = blockette.substr(j,1); j++;
    input_units = parseInt(blockette.substr(j, 3), "041.inputUnits"); j += 3;
    output_units = parseInt(blockette.substr(j, 3), "041.outputUnits"); j += 3;
    num = parseInt(blockette.substr(j, 4), "041.numFactors"); j += 4;

    for(i = 0; i < num; i++) {
	coef.push_back(parseDouble(blockette.substr(j, 14), "041.coefficient"));
	j += 14;
    }
}

string Blockette41::str()
{
    ostringstream out;
    out << "code=" << lookup_code << " "
	<< "name=" << name << " "
	<< "symmetry=" << symmetry_code << " "
	<< "input=" << input_units << " "
	<< "output=" << output_units << " "
	<< "num_coef=" << (int)coef.size();
    return out.str();
}

string Blockette41::longStr()
{
    char s[20];
    ostringstream out;
    out << "response lookup key: " << lookup_code << endl
	<< "response name:       " << name << endl
	<< "symmetry code:       " << symmetry_code << endl
	<< "signal in units:     " << input_units << endl
	<< "signal out units:    " << output_units << endl
	<< "number of factors:   " << (int)coef.size();
    if((int)coef.size() > 0) {
	out << endl;
	for(int i = 0; i < (int)coef.size(); i++) {
	    snprintf(s, sizeof(s), "%14.7e", coef[i]);
	    out << s;
	    if(i < (int)coef.size()-1) {
		out << " ";
		if((i+1) % 5 == 0) out << endl;
	    }
	}
    }
    return out.str();
}

void Blockette42::load(const string &blockette) throw(FormatException)
{
    int i, j, num_coef;

    lookup_code = parseInt(blockette.substr(0, 4), "042.lookupCode");
    name = variable(blockette.substr(4));
    j = 4 + name.length() + 1;
    transfer_type = blockette.substr(j,1); j++;
    input_units = parseInt(blockette.substr(j, 3), "042.inputUnits"); j += 3;
    output_units = parseInt(blockette.substr(j, 3), "042.outputUnits"); j += 3;
    poly_type = blockette.substr(j,1); j++;
    freq_units = blockette.substr(j,1); j++;
    min_freq = parseDouble(blockette.substr(j, 12),"042.minFreq"); j += 12;
    max_freq = parseDouble(blockette.substr(j, 12),"042.maxFreq"); j += 12;
    min_approx = parseDouble(blockette.substr(j, 12),"042.minApprox"); j += 12;
    max_approx = parseDouble(blockette.substr(j, 12),"042.maxApprox"); j += 12;
    max_error = parseDouble(blockette.substr(j, 12),"042.maxError"); j += 12;
    num_coef = parseInt(blockette.substr(j, 3), "042.numFactors"); j += 3;

    for(i = 0; i < num_coef; i++) {
	coef.push_back(parseDouble(blockette.substr(j, 12), "042.coefficient")); j += 12;
	error.push_back(parseDouble(blockette.substr(j, 12), "042.error")); j += 12;
    }
}

string Blockette42::str()
{
    ostringstream out;
    out << "code=" << lookup_code << " "
	<< "name=" << name << " "
	<< "type=" << transfer_type << " "
	<< "input=" << input_units << " "
	<< "output=" << output_units << " "
	<< "poly_type=" << poly_type << " "
	<< "freq_units=" << freq_units << " "
	<< setiosflags(ios::scientific) << setprecision(5)
	<< "min_freq=" << min_freq << " "
	<< "max_freq=" << max_freq << " "
	<< "min_approx=" << min_approx << " "
	<< "max_approx=" << max_approx << " "
	<< "max_error=" << max_error << " "
	<< "num_coed=" << (int)coef.size();
    return out.str();
}

string Blockette42::longStr()
{
    char s[50];
    ostringstream out;
    out << "response lookup key:          " << lookup_code << endl
	<< "response name:                " << name << endl
	<< "transfer function type:       " << transfer_type << endl
	<< "stage signal input units:     " << input_units << endl
	<< "stage signal output units:    " << output_units << endl
	<< "polynomial approximation:     " << poly_type << endl
	<< "valid frequency units:        " << freq_units << endl
	<< setiosflags(ios::scientific) << setprecision(5)
	<< "lower valid frequency bound:  " << min_freq << endl
	<< "upper valid frequency bound:  " << max_freq << endl
	<< "lower bound of approximation: " << min_approx << endl
	<< "upper bound of approximation: " << max_approx << endl
	<< "maximum absolute error:       " << max_error << endl
	<< "number of poly coefficients:  " << (int)coef.size();
    if((int)coef.size() > 0) {
	out << endl << "coefficient    error";
	for(int i = 0; i < (int)coef.size(); i++) {
	    snprintf(s, sizeof(s), "\n%12.5e   %12.5e", coef[i], error[i]);
	    out << s;
	}
    }
    return out.str();
}

void Blockette43::load(const string &blockette) throw(FormatException)
{
    int i, j, num_zeros, num_poles;

    lookup_code = parseInt(blockette.substr(0, 4), "043.lookUpCode");
    name = variable(blockette.substr(4));
    j = 4 + name.length() + 1;
    type = blockette.substr(j,1); j++;
    input_units = parseInt(blockette.substr(j, 3), "043.inputUnits"); j += 3;
    output_units = parseInt(blockette.substr(j, 3), "043.outputUnits"); j += 3;
    a0_norm = parseDouble(blockette.substr(j, 12), "043.a0Norm"); j += 12;
    norm_freq = parseDouble(blockette.substr(j, 12),"043.normFreq"); j += 12;

    num_zeros = parseInt(blockette.substr(j, 3),"043.numZeros"); j += 3;

    for(i = 0; i < num_zeros; i++) {
	zr.push_back(parseDouble(blockette.substr(j, 12),"043.real zero")); j += 12;
	zi.push_back(parseDouble(blockette.substr(j, 12),"043.imag zero")); j += 12;
	zr_error.push_back(parseDouble(blockette.substr(j, 12),"043.real zero-error")); j += 12;
	zi_error.push_back(parseDouble(blockette.substr(j, 12),"043.imag zero-error")); j += 12;
    }

    num_poles = parseInt(blockette.substr(j, 3), "043.numPoles"); j += 3;

    for(i = 0; i < num_poles; i++) {
	pr.push_back(parseDouble(blockette.substr(j, 12), "043.real pole")); j += 12;
	pi.push_back(parseDouble(blockette.substr(j, 12), "043.imag pole")); j += 12;
	pr_error.push_back(parseDouble(blockette.substr(j, 12), "043.real pole-error")); j += 12;
	pi_error.push_back(parseDouble(blockette.substr(j, 12), "043.imag pole-error")); j += 12;
    }
}

string Blockette43::str()
{
    ostringstream out;
    out << "code=" << lookup_code << " "
	<< "name=" << name << " "
	<< "type=" << type << " "
	<< "input=" << input_units << " "
	<< "output=" << output_units << " "
	<< setiosflags(ios::scientific) << setprecision(5)
	<< "a0=" << a0_norm << " "
	<< "norm_freq=" << norm_freq << " "
	<< "num_zeros=" << (int)zr.size() << " "
	<< "num_poles=" << (int)pr.size();
    return out.str();
}

string Blockette43::longStr()
{
    char s[80];
    ostringstream out;
    out << "response lookup key:       " << lookup_code << endl
	<< "response name:             " << name << endl
	<< "response type:             " << type << endl
	<< "stage signal input units:  " << input_units << endl
	<< "stage signal output units: " << output_units << endl
	<< "A0 normalization factor:   " << a0_norm << endl
	<< "normalization frequency:   " << norm_freq << endl
	<< "number of complex zeros:   " << (int)zr.size() << endl;

    if((int)zr.size() > 0) {
	out << "real          imaginary     real error    imaginary error";
	for(int i = 0; i < (int)zr.size(); i++) {
	    snprintf(s, sizeof(s), "\n%12.5e  %12.5e  %12.5e  %12.5e",
		zr[i], zi[i], zr_error[i], zi_error[i]);
	    out << s;
	}
    }

    out << "number of complex poles:   " << (int)pr.size();
    if((int)pr.size() > 0) {
	out << endl;
	out << "real          imaginary     real error    imaginary error";
	for(int i = 0; i < (int)pr.size(); i++) {
	    snprintf(s, sizeof(s), "\n%12.5e  %12.5e  %12.5e  %12.5e",
		pr[i], pi[i], pr_error[i], pi_error[i]);
	    out << s;
	}
    }

    return out.str();
}

void Blockette44::load(const string &blockette) throw(FormatException)
{
    int i, j, num_n, num_d;

    lookup_code = parseInt(blockette.substr(0, 4), "044.lookupCode");
    name = variable(blockette.substr(4));
    j = 4 + name.length() + 1;
    type = blockette.substr(j,1); j++;
    input_units = parseInt(blockette.substr(j, 3), "044.inputUnits"); j += 3;
    output_units = parseInt(blockette.substr(j, 3), "044.outputUnits"); j += 3;

    num_n = parseInt(blockette.substr(j, 4), "044.num numerators"); j += 4;

    for(i = 0; i < num_n; i++) {
	numerator.push_back(parseDouble(blockette.substr(j, 12),"044.numerator")); j+=12;
	nerror.push_back(parseDouble(blockette.substr(j, 12),"044.numerator-error")); j+=12;
    }

    num_d = parseInt(blockette.substr(j, 4), "044.num denominators"); j += 4;

    for(i = 0; i < num_d; i++) {
	denominator.push_back(parseDouble(blockette.substr(j, 12), "044.denominator")); j+=12;
	derror.push_back(parseDouble(blockette.substr(j, 12), "044.denom-eror")); j+=12;
    }
}

string Blockette44::str()
{
    ostringstream out;
    out << "code=" << lookup_code << " "
	<< "name=" << name << " "
	<< "type=" << type << " "
	<< "input=" << input_units << " "
	<< "output=" << output_units << " "
	<< "num_n=" << (int)numerator.size() << " "
	<< "num_d=" << (int)denominator.size();
    return out.str();
}

string Blockette44::longStr()
{
    char s[80];
    ostringstream out;
    out << "response lookup key:       " << lookup_code << endl
	<< "response name:             " << name << endl
	<< "response type:             " << type << endl
	<< "stage signal input units:  " << input_units << endl
	<< "stage signal output units: " << output_units << endl
	<< "number of numerators:      " << (int)numerator.size() << endl;

    if((int)numerator.size() > 0) {
	out << "coefficient   error";
	for(int i = 0; i < (int)numerator.size(); i++) {
	    snprintf(s, sizeof(s), "\n%12.5e  %12.5e", numerator[i], nerror[i]);
	    out << s;
	}
    }

    out << "number of denominators:   " << (int)denominator.size();
    if((int)denominator.size() > 0) {
	out << endl;
	out << "coefficient   error";
	for(int i = 0; i < (int)denominator.size(); i++) {
	    snprintf(s, sizeof(s), "\n%12.5e  %12.5e",denominator[i],derror[i]);
	    out << s;
	}
    }

    return out.str();
}

void Blockette45::load(const string &blockette) throw(FormatException)
{
    int i, j, num;

    lookup_code = parseInt(blockette.substr(0, 4), "045.lookupCode");
    name = variable(blockette.substr(4));
    j = 4 + name.length() + 1;
    input_units = parseInt(blockette.substr(j, 3), "045.inputUnits"); j += 3;
    output_units = parseInt(blockette.substr(j, 3), "045.outputUnits"); j += 3;

    num = parseInt(blockette.substr(j, 4), "045.num responses"); j += 4;

    for(i = 0; i < num; i++) {
	frequency.push_back(parseDouble(blockette.substr(j, 12), "045.frequency")); j+=12;
	amplitude.push_back(parseDouble(blockette.substr(j, 12), "045.amplitude")); j+=12;
	amp_error.push_back(parseDouble(blockette.substr(j, 12), "045.ampError")); j+=12;
	phase.push_back(parseDouble(blockette.substr(j, 12), "045.phase")); j+=12;
	phase_error.push_back(parseDouble(blockette.substr(j, 12), "045.phaseError")); j+=12;
    }
}

string Blockette45::str()
{
    ostringstream out;
    out << "code=" << lookup_code << " "
	<< "name=" << name << " "
	<< "input=" << input_units << " "
	<< "output=" << output_units << " "
	<< "num=" << (int)phase.size();
	return out.str();
}

string Blockette45::longStr()
{
    char s[80];
    ostringstream out;
    out << "response lookup key:       " << lookup_code << endl
	<< "response name:             " << name << endl
	<< "response type:             " << type << endl
	<< "stage signal input units:  " << input_units << endl
	<< "stage signal output units: " << output_units << endl
	<< "number of responses:       " << (int)frequency.size() << endl;

    if((int)frequency.size() > 0) {
	out <<
	"frequency     amplitude     amplitude err  phase angle   phase error";
	for(int i = 0; i < (int)frequency.size(); i++) {
	    snprintf(s, sizeof(s), "\n%12.5e  %12.5e  %12.5e   %12.5e  %12.5e",
		frequency[i], amplitude[i], amp_error[i], phase[i],
		phase_error[i]);
	    out << s;
	}
    }
    return out.str();
}

void Blockette46::load(const string &blockette) throw(FormatException)
{
    int i, j, num;

    lookup_code = parseInt(blockette.substr(0, 4), "046.lookupCode");
    name = variable(blockette.substr(4));
    j = 4 + name.length() + 1;
    input_units = parseInt(blockette.substr(j, 3), "046.inputUnits"); j += 3;
    output_units = parseInt(blockette.substr(j, 3), "046.outputUnits"); j += 3;

    num = parseInt(blockette.substr(j, 4), "046.num responses"); j += 4;

    for(i = 0; i < num; i++) {
	corner_freq.push_back(parseDouble(blockette.substr(j,12),"046.cornerFreq")); j+=12;
	corner_slope.push_back(parseDouble(blockette.substr(j,12),"046.cornerSlope")); j+=12;
    }
}

string Blockette46::str()
{
    ostringstream out;
    out << "code=" << lookup_code << " "
	<< "name=" << name << " "
	<< "input=" << input_units << " "
	<< "output=" << output_units << " "
	<< "num=" << (int)corner_freq.size();
    return out.str();
}

string Blockette46::longStr()
{
    char s[80];
    ostringstream out;
    out << "response lookup key:       " << lookup_code << endl
	<< "response name:             " << name << endl
	<< "stage signal input units:  " << input_units << endl
	<< "stage signal output units: " << output_units << endl
	<< "number of corners:         " << (int)corner_freq.size() << endl;

    if((int)corner_freq.size() > 0) {
	out << "corner frequency(Hz)   corner slope (db/decade)";
	for(int i = 0; i < (int)corner_freq.size(); i++) {
	    snprintf(s, sizeof(s), "\n%12.5e           %12.5e",
		corner_freq[i], corner_slope[i]);
	    out << s;
	}
    }
    return out.str();
}

void Blockette47::load(const string &blockette) throw(FormatException)
{
    int j;

    lookup_code = parseInt(blockette.substr(0, 4), "047.lookupCode");
    name = variable(blockette.substr(4));
    j = 4 + name.length() + 1;
    input_sample_rate = parseDouble(blockette.substr(j, 10), "047.inputSampleRate"); j+=10;
    decimation_factor = parseInt(blockette.substr(j, 5), "047.decimationFactor"); j += 5;
    decimation_offset = parseInt(blockette.substr(j, 5), "047.decimationOffset"); j += 5;
    delay = parseDouble(blockette.substr(j, 11), "047.delay"); j += 11;
    correction = parseDouble(blockette.substr(j, 11), "047.correction");
}

string Blockette47::str()
{
    ostringstream out;
    out << "code=" << lookup_code << " "
	<< "name=" << name << " "
	<< setiosflags(ios::scientific) << setprecision(4)
	<< "in_samprate=" << input_sample_rate << " "
	<< "deci=" << decimation_factor << " "
	<< "offset=" << decimation_offset << " "
	<< "delay=" << delay << " "
	<< "corr=" << correction;
    return out.str();
}

string Blockette47::longStr()
{
    ostringstream out;
    out << "response lookup key:    " << lookup_code << endl
	<< "response name:          " << name << endl
	<< setiosflags(ios::scientific) << setprecision(4)
	<< "input sample rate:      " << input_sample_rate << endl
	<< "decimation factor:      " << decimation_factor << endl
	<< "decimation offset:      " << decimation_offset << endl
	<< "estimated delay (secs): " << delay << endl
	<< "correction applied:     " << correction;
    return out.str();
}

void Blockette48::load(const string &blockette) throw(FormatException)
{
    int i, j, num;
    string v;

    lookup_code = parseInt(blockette.substr(0, 4), "048.lookupCode");
    name = variable(blockette.substr(4));
    j = 4 + name.length() + 1;
    sensitivity = parseDouble(blockette.substr(j, 12), "048.sensitivity"); j += 12;
    frequency = parseDouble(blockette.substr(j, 12), "048.frequency"); j += 12;

    num = parseInt(blockette.substr(j, 2), "048.num histories"); j += 2;

    for(i = 0; i < num; i++) {
	cal_sensitivity.push_back(
		parseDouble(blockette.substr(j,12), "048.calSensitivity")); j+=12;
	cal_frequency.push_back(
		parseDouble(blockette.substr(j,12), "048.calFrequency")); j+=12;
	v = variable(blockette.substr(j));
	cal_time.push_back(SeedTime(v, "048.time")); j += v.length() + 1;
    }
}

string Blockette48::str()
{
    ostringstream out;
    out << "code=" << lookup_code << " "
	<< "name=" << name << " "
	<< setiosflags(ios::scientific) << setprecision(5)
	<< "sensitivity=" << sensitivity << " "
	<< "freq=" << frequency << " "
	<< "time=" << (int)cal_time.size();
    return out.str();
}

string Blockette48::longStr()
{
    char s[80];
    ostringstream out;
    out << "response lookup key:      " << lookup_code << endl
	<< "response name:            " << name << endl;
    snprintf(s, sizeof(s), "%12.5e", sensitivity);
    out << "sensitivity/gain:         " << s << endl;
    snprintf(s, sizeof(s), "%12.5e", frequency);
    out << "frequency (Hz):           " << s << endl;
    out << "number of history values: " << (int)cal_time.size();

    if((int)cal_time.size() > 0) {
	out << endl << "sensitivity   frequency     time" << endl;
	for(int i = 0; i < (int)cal_time.size(); i++) {
	    snprintf(s, sizeof(s), "\n%12.5e  %12.5e  %s",
		cal_sensitivity[i], cal_frequency[i],cal_time[i].str().c_str());
	    out << s;
	}
    }
    return out.str();
}

void Blockette50::load(const string &blockette) throw(FormatException)
{
    int j;
    string v;

    station = trim(blockette.substr(0, 5));
    latitude = parseDouble(blockette.substr(5, 10),"050.latitude");
    longitude = parseDouble(blockette.substr(15, 11),"050.longitude");
    elevation = parseDouble(blockette.substr(26, 7),"050.elevation");
    num_channels = parseInt(blockette.substr(33, 4),"050.numChannels");
    num_comments = parseInt(blockette.substr(37, 3),"050.numComments");
    name = variable(blockette.substr(40));
    j = 40 + name.length() + 1;
    network_id = parseInt(blockette.substr(j, 3),"050.networkId"); j += 3;
    word_order = blockette.substr(j,4); j += 4;
    short_order = blockette.substr(j,2); j += 2;
    v = variable(blockette.substr(j));
    start = SeedTime(v, "050.start_date"); j += v.length() + 1;
    v = variable(blockette.substr(j));
    end = SeedTime(v, "050.end_date"); j += v.length() + 1;
    update = blockette.substr(j, 1); j++;
    if((int)blockette.length() >= j+2) {
	// version >= 2.3
	network = trim(blockette.substr(j, 2));
    }
}

string Blockette50::str()
{
    ostringstream out;
    out << "sta=" << station << " "
	<< setiosflags(ios::fixed) << setprecision(6)
	<< "lat=" << latitude << " "
	<< "lon=" << longitude << " "
	<< setprecision(1)
	<< "elev=" << elevation << " "
	<< "nchan=" << num_channels << " "
	<< "ncmts=" << num_comments << " "
	<< "site=" << name << " "
	<< "netid=" << network_id << " "
	<< "32bit=" << word_order << " "
	<< "16bit=" << short_order << " "
	<< "start=" << start.str() << " "
	<< "end=" << end.str() << " "
	<< "update=" << update << " "
	<< "net=" << network;
    return out.str();
}

string Blockette50::longStr()
{
    ostringstream out;
    out << "station:                     " << station << endl
	<< setiosflags(ios::fixed) << setprecision(6)
	<< "latitude:                    " << latitude << endl
	<< "longitude:                   " << longitude << endl
	<< setprecision(1)
	<< "elevation(m):                " << elevation << endl
	<< "number of channels:          " << num_channels << endl
	<< "number of station comments:  " << num_comments << endl
	<< "site name:                   " << name << endl
	<< "network identification code: " << network_id << endl
	<< "32 bit word order:           " << word_order << endl
	<< "16 bit word order:           " << short_order << endl
	<< "start effective date:        " << start.str() << endl
	<< "end effective date:          " << end.str() << endl
	<< "update flag:                 " << update << endl
	<< "network code:                " << network;
    return out.str();
}

void Blockette51::load(const string &blockette) throw(FormatException)
{
    int j;
    string v;

    j = 0;
    v = variable(blockette.substr(j));
    beg = SeedTime(v, "051.beginning_time"); j += v.length() + 1;
    v = variable(blockette.substr(j));
    end = SeedTime(v, "051.end_time"); j += v.length() + 1;
    comment_code = parseInt(blockette.substr(j, 4),"051.commentCode");
    comment_level = parseInt(blockette.substr(j, 6),"051.commentLevel");
}

string Blockette51::str()
{
    ostringstream out;
    out << "beg=" << beg.str() << "  "
	<< "end=" << end.str() << " "
	<< "code=" << comment_code << " "
	<< "level=" << comment_level;
    return out.str();
}

string Blockette51::longStr()
{
    ostringstream out;
    out << "beginning effective time: " << beg.str() << endl
	<< "end effective time:       " << end.str() << endl
	<< "comment code key:         " << comment_code << endl
	<< "comment level:            " << comment_level;
    return out.str();
}

void Blockette52::load(const string &blockette) throw(FormatException)
{
    int j;
    string v;

    location = trim(blockette.substr(0, 2));
    channel = trim(blockette.substr(2, 3));
    subchannel = parseInt(blockette.substr(5, 4), "052.subchannel");
    instrument = parseInt(blockette.substr(9, 3), "052.instrument");
    j = 12;
    comment = variable(blockette.substr(j));
    j += comment.length() + 1;
    signal_units = parseInt(blockette.substr(j, 3),"052.signalUnits"); j += 3;
    calib_units = parseInt(blockette.substr(j, 3),"052.calibUnits"); j += 3;
    latitude = parseDouble(blockette.substr(j, 10),"052.latitude"); j += 10;
    longitude = parseDouble(blockette.substr(j, 11),"052.longitude"); j += 11;
    elevation = parseDouble(blockette.substr(j, 7),"052.elevation"); j += 7;
    local_depth = parseDouble(blockette.substr(j, 5),"052.localDepth"); j += 5;
    azimuth = parseDouble(blockette.substr(j, 5),"052.azimuth"); j += 5;
    dip = parseDouble(blockette.substr(j, 5),"052.dip"); j += 5;
    format_code = parseInt(blockette.substr(j, 4),"052.formatCode"); j += 4;
    reclen = parseInt(blockette.substr(j, 2),"052.reclen"); j += 2;
    sample_rate = parseDouble(blockette.substr(j, 10),"052.sampleRate"); j += 10;
    clock_drift = parseDouble(blockette.substr(j, 10),"052.clockDrift"); j += 10;
    num = parseInt(blockette.substr(j, 4),"052.numComments"); j += 4;
    channel_flags = variable(blockette.substr(j));
    j += channel_flags.length() + 1;
		
    v = variable(blockette.substr(j));
    start = SeedTime(v, "052.start_date"); j += v.length() + 1;
    v = variable(blockette.substr(j));
    end = SeedTime(v, "052.end_date"); j += v.length() + 1;
    update = blockette.substr(j, 1);
}

string Blockette52::str()
{
    ostringstream out;
    out << "loc=" << location << " "
	<< "chan=" << channel << " "
	<< "sub=" << subchannel << " "
	<< "inst=" << instrument << " "
	<< "cmt=" << comment << " "
	<< "sig=" << signal_units << " "
	<< "cal=" << calib_units << " "
	<< setiosflags(ios::fixed) << setprecision(6)
	<< "lat=" << latitude << " "
	<< "lon=" << longitude << " "
	<< setprecision(1)
	<< "elev=" << elevation << " "
	<< "depth=" << local_depth << " "
	<< "az=" << azimuth << " "
	<< "dip=" << dip << " "
	<< "fmt=" << format_code << " "
	<< "rlen=" << reclen << " "
	<< setiosflags(ios::scientific) << setprecision(4)
	<< "srate=" << sample_rate << " "
	<< "drift=" << clock_drift << " "
	<< "num=" << num << " "
	<< "flags=" << channel_flags << " "
	<< "start=" << start.str() << " "
	<< "end=" << end.str() << " "
	<< "update=" << update;
    return out.str();
}

string Blockette52::longStr()
{
    ostringstream out;
    out << "location identifier:         " << location << endl
	<< "channel identifier:          " << channel << endl
	<< "subchannel identifier:       " << subchannel << endl
	<< "instrument identifier:       " << instrument << endl
	<< "optional comment:            " << comment << endl
	<< "units of signal response:    " << signal_units << endl
	<< "units of calibration input:  " << calib_units << endl
	<< setiosflags(ios::fixed) << setprecision(6)
	<< "latitude (degrees):          " << latitude << endl
	<< "longitude (degrees):         " << longitude << endl
	<< setprecision(1)
	<< "elevation (m):               " << elevation << endl
	<< "local depth (m):             " << local_depth << endl
	<< "azimuth (degrees):           " << azimuth << endl
	<< "dip (degrees):               " << dip << endl
	<< "data format identifier code: " << format_code << endl
	<< "data record length:          " << reclen << endl
	<< setiosflags(ios::scientific) << setprecision(4)
	<< "sample rate (Hz):            " << sample_rate << endl
	<< "max clock drift (seconds):   " << clock_drift << endl
	<< "number of comments:          " << num << endl
	<< "channel flags:               " << channel_flags << endl
	<< "start date:                  " << start.str() << endl
	<< "end date:                    " << end.str() << endl
	<< "update flag:                 " << update;
    return out.str();
}

void Blockette53::load(const string &blockette) throw(FormatException)
{
    int i, j, num_zeros, num_poles;

    type = blockette.substr(0,1);
    stage = parseInt(blockette.substr(1, 2), "053.stage");
    input_units = parseInt(blockette.substr(3, 3),"053.inputUnits");
    output_units = parseInt(blockette.substr(6, 3),"053.outputUnits");
    a0_norm = parseDouble(blockette.substr(9, 12),"053.a0Norm");
    norm_freq = parseDouble(blockette.substr(21, 11),"053.normFreq");

    num_zeros = parseInt(blockette.substr(33, 3),"053.numZeros");

    j = 36;
    for(i = 0; i < num_zeros; i++) {
	zr.push_back(parseDouble(blockette.substr(j, 12),"053.real zero")); j += 12;
	zi.push_back(parseDouble(blockette.substr(j, 12),"053.imag zero")); j += 12;
	zr_error.push_back(parseDouble(blockette.substr(j, 12), "053.real zero-error")); j += 12;
	zi_error.push_back(parseDouble(blockette.substr(j, 12), "053.imag zero-error")); j += 12;
    }

    num_poles = parseInt(blockette.substr(j, 3),"053.numPoles"); j += 3;

    for(i = 0; i < num_poles; i++) {
	pr.push_back(parseDouble(blockette.substr(j, 12),"053.real pole")); j += 12;
	pi.push_back(parseDouble(blockette.substr(j, 12),"053.imag Pole")); j += 12;
	pr_error.push_back(parseDouble(blockette.substr(j, 12), "053.real pole-error")); j += 12;
	pi_error.push_back(parseDouble(blockette.substr(j, 12), "053.real pole-error")); j += 12;
    }
}

Blockette53::Blockette53(int stage_number, const Blockette43 &b) :
		Seed("053"), from_b43(true)
{
    stage = stage_number;
    type = b.type;
    input_units = b.input_units;
    output_units = b.output_units;
    a0_norm = b.a0_norm;
    norm_freq = b.norm_freq;

    zr.assign(b.zr.begin(), b.zr.end());
    zi.assign(b.zi.begin(), b.zi.end());
    zr_error.assign(b.zr_error.begin(), b.zr_error.end());
    zi_error.assign(b.zi_error.begin(), b.zi_error.end());

    pr.assign(b.pr.begin(), b.pr.end());
    pi.assign(b.pi.begin(), b.pi.end());
    pr_error.assign(b.pr_error.begin(), b.pr_error.end());
    pi_error.assign(b.pi_error.begin(), b.pi_error.end());
}

string Blockette53::str()
{
    ostringstream out;
    out << "type=" << type << " "
	<< "stage=" << stage << " "
	<< "input=" << input_units << " "
	<< "output=" << output_units << " "
	<< setiosflags(ios::scientific) << setprecision(5)
	<< "a0=" << a0_norm << " "
	<< "norm_freq=" << norm_freq << " "
	<< "num_z=" << (int)zr.size() <<  " "
	<< "num_p=" << (int)pr.size();
    return out.str();
}

string Blockette53::longStr()
{
    char s[80];
    ostringstream out;
    out << "transfer function type:    " << type << endl
	<< "stage signal input units:  " << input_units << endl
	<< "stage signal output units: " << output_units << endl
	<< "A0 normalization factor:   " << a0_norm << endl
	<< "normalization frequency:   " << norm_freq << endl
	<< "number of complex zeros:   " << (int)zr.size();

    if((int)zr.size() > 0) {
	out << endl;
	out << "real          imaginary     real error    imaginary error";
	for(int i = 0; i < (int)zr.size(); i++) {
	    snprintf(s, sizeof(s), "\n%12.5e  %12.5e  %12.5e  %12.5e",
		zr[i], zi[i], zr_error[i], zi_error[i]);
	    out << s;
	}
    }

    out << endl << "number of complex poles:   " << (int)pr.size();
    if((int)pr.size() > 0) {
	out << endl;
	out << "real          imaginary     real error    imaginary error";
	for(int i = 0; i < (int)pr.size(); i++) {
	    snprintf(s, sizeof(s), "\n%12.5e  %12.5e  %12.5e  %12.5e",
		pr[i], pi[i], pr_error[i], pi_error[i]);
	    out << s;
	}
    }

    return out.str();
}

void Blockette54::load(const string &blockette) throw(FormatException)
{
    int i, j, num_n, num_d;

    type = blockette.substr(0,1);
    stage = parseInt(blockette.substr(1, 2), "054.stage");
    input_units = parseInt(blockette.substr(3, 3), "054.inputUnits");
    output_units = parseInt(blockette.substr(6, 3), "054.outputUnits");

    num_n = parseInt(blockette.substr(9, 4), "054.num numerators");

    j = 13;
    for(i = 0; i < num_n; i++) {
	numerator.push_back(parseDouble(blockette.substr(j, 12), "054.numerator")); j+=12;
	nerror.push_back(parseDouble(blockette.substr(j, 12), "054.n-error")); j+=12;
    }

    num_d = parseInt(blockette.substr(j, 4), "054.num denominators"); j += 4;

    for(i = 0; i < num_d; i++) {
	denominator.push_back(parseDouble(blockette.substr(j, 12), "054.denom")); j+=12;
	derror.push_back(parseDouble(blockette.substr(j, 12), "054.denom-error")); j+=12;
    }
}

Blockette54::Blockette54(int stage_number, const Blockette44 &b)
		: Seed("054"), from_b44(true)
{
    stage = stage_number;
    type = b.type;
    input_units = b.input_units;
    output_units = b.output_units;

    numerator.assign(b.numerator.begin(), b.numerator.end());
    nerror.assign(b.nerror.begin(), b.nerror.end());
    denominator.assign(b.denominator.begin(), b.denominator.end());
    derror.assign(b.derror.begin(), b.derror.end());
}

string Blockette54::str()
{
    ostringstream out;
    out << "type=" << type << " "
	<< "stage=" << stage << " "
	<< "input=" << input_units << " "
	<< "output=" << output_units << " "
	<< "num_n=" << (int)numerator.size() << " "
	<< "num_d=" << (int)denominator.size();
    return out.str();
}

string Blockette54::longStr()
{
    char s[80];
    ostringstream out;
    out << "response type:          " << type << endl
	<< "stage sequence number:  " << stage << endl
	<< "signal input units:     " << input_units << endl
	<< "signal output units:    " << output_units << endl
	<< "number of numerators:   " << (int)numerator.size();

    if((int)numerator.size() > 0) {
	out << endl << "coefficient   error";
	for(int i = 0; i < (int)numerator.size(); i++) {
	    snprintf(s, sizeof(s), "\n%12.5e  %12.5e", numerator[i], nerror[i]);
	    out << s;
	}
    }

    out << endl << "number of denominators: " << (int)denominator.size();
    if((int)denominator.size() > 0) {
	out << endl << "coefficient   error";
	for(int i = 0; i < (int)denominator.size(); i++) {
	    snprintf(s, sizeof(s), "\n%12.5e  %12.5e",denominator[i],derror[i]);
	    out << s;
	}
    }
    return out.str();
}

void Blockette55::load(const string &blockette) throw(FormatException)
{
    int i, j, num;

    stage = parseInt(blockette.substr(0, 2), "055.stage");
    input_units = parseInt(blockette.substr(2, 3), "055.inputUnits");
    output_units = parseInt(blockette.substr(5, 3), "055.outputUnits");

    num = parseInt(blockette.substr(8, 4), "055.num responses");

    j = 12;
    for(i = 0; i < num; i++) {
	frequency.push_back(parseDouble(blockette.substr(j, 12), "055.frequency")); j+=12;
	amplitude.push_back(parseDouble(blockette.substr(j, 12), "055.amplitude")); j+=12;
	amp_error.push_back(parseDouble(blockette.substr(j, 12), "055.ampError")); j+=12;
	phase.push_back(parseDouble(blockette.substr(j, 12), "055.phase")); j+=12;
	phase_error.push_back(parseDouble(blockette.substr(j, 12), "055.PhaseError")); j+=12;
    }
}

Blockette55::Blockette55(int stage_number, const Blockette45 &b)
		: Seed("055"), from_b45(true)
{
    stage = stage_number;
    input_units = b.input_units;
    output_units = b.output_units;

    frequency.assign(b.frequency.begin(), b.frequency.end());
    amplitude.assign(b.amplitude.begin(), b.amplitude.end());
    amp_error.assign(b.amp_error.begin(), b.amp_error.end());
    phase.assign(b.phase.begin(), b.phase.end());
    phase_error.assign(b.phase_error.begin(), b.phase_error.end());
}

string Blockette55::str()
{
    ostringstream out;
    out << "stage=" << stage << " "
	<< "input=" << input_units << " "
	<< "output=" << output_units << " "
	<< "num=" << (int)phase.size();
    return out.str();
}

string Blockette55::longStr()
{
    char s[80];
    ostringstream out;
    out << "stage sequence number: " << stage << endl
	<< "signal input units:    " << input_units << endl
	<< "signal output units:   " << output_units << endl
	<< "number of responses:   " << (int)frequency.size() << endl;

    if((int)frequency.size() > 0) {
	out <<
	"frequency     amplitude     amplitude err  phase angle   phase error";
	for(int i = 0; i < (int)frequency.size(); i++) {
	    snprintf(s, sizeof(s), "\n%12.5e  %12.5e  %12.5e   %12.5e  %12.5e",
		frequency[i], amplitude[i], amp_error[i], phase[i],
		phase_error[i]);
	    out << s;
	}
    }
    return out.str();
}

void Blockette56::load(const string &blockette) throw(FormatException)
{
    int i, j, num;

    stage = parseInt(blockette.substr(0, 2), "056.stage");
    input_units = parseInt(blockette.substr(2, 3), "056.inputUnits");
    output_units = parseInt(blockette.substr(5, 3), "056.outputUnits");

    num = parseInt(blockette.substr(8, 4), "056.num responses");

    j = 12;
    for(i = 0; i < num; i++) {
	corner_freq.push_back(parseDouble(blockette.substr(j,12), "056.cornerFreq")); j+=12;
	corner_slope.push_back(parseDouble(blockette.substr(j,12), "056.cornerSlope")); j+=12;
    }
}

Blockette56::Blockette56(int stage_number, const Blockette46 &b)
		: Seed("056"), from_b46(true)
{
    stage = stage_number;
    input_units = b.input_units;
    output_units = b.output_units;

    corner_freq.assign(b.corner_freq.begin(), b.corner_freq.end());
    corner_slope.assign(b.corner_slope.begin(), b.corner_slope.end());
}

string Blockette56::str()
{
    ostringstream out;
    out << "stage=" << stage << " "
	<< "input=" << input_units << " "
	<< "output=" << output_units << " "
	<< "num=" << (int)corner_freq.size();
    return out.str();
}

string Blockette56::longStr()
{
    char s[80];
    ostringstream out;
    out << "stage sequence number: " << stage << endl
	<< "signal input units:    " << input_units << endl
	<< "signal output units:   " << output_units << endl
	<< "number of corners:     " << (int)corner_freq.size() << endl;

    if((int)corner_freq.size() > 0) {
	out <<
	"corner frequency(Hz)   corner slope (db/decade)";
	for(int i = 0; i < (int)corner_freq.size(); i++) {
	    snprintf(s, sizeof(s), "\n%12.5e           %12.5e",
		corner_freq[i], corner_slope[i]);
	    out << s;
	}
    }
    return out.str();
}

void Blockette57::load(const string &blockette) throw(FormatException)
{
    stage = parseInt(blockette.substr(0, 2), "057.stage");
    input_sample_rate = parseDouble(blockette.substr(2, 10), "057.inputSampleRate");
    decimation_factor = parseInt(blockette.substr(12, 5), "057.decimationFactor");
    decimation_offset = parseInt(blockette.substr(17, 5), "057.decimationOffset");
    delay = parseDouble(blockette.substr(22, 11), "057.delay");
    correction = parseDouble(blockette.substr(33, 11), "057.correction");
}

Blockette57::Blockette57(int stage_number, const Blockette47 &b)
		: Seed("057"), from_b47(true)
{
    stage = stage_number;
    input_sample_rate = b.input_sample_rate;
    decimation_factor = b.decimation_factor;
    decimation_offset = b.decimation_offset;
    delay = b.delay;
    correction = b.correction;
}

string Blockette57::str()
{
    ostringstream out;
    out << "stage=" << stage << " "
	<< setiosflags(ios::scientific) << setprecision(4)
	<< "input=" << input_sample_rate << " "
	<< "deci=" << decimation_factor << " "
	<< "offset=" << decimation_offset << " "
	<< "delay=" << delay << " "
	<< "corr=" << correction;
    return out.str();
}

string Blockette57::longStr()
{
    ostringstream out;
    out << "stage sequence number:  " << stage << endl
	<< setiosflags(ios::scientific) << setprecision(4)
	<< "input sample rate (Hz): " << input_sample_rate << endl
	<< "decimation factor:      " << decimation_factor << endl
	<< "decimation offset:      " << decimation_offset << endl
	<< "extimated delay (secs): " << delay << endl
	<< "correction applied:     " << correction;
    return out.str();
}

void Blockette58::load(const string &blockette) throw(FormatException)
{
    int i, j, num;
    string v;

    stage = parseInt(blockette.substr(0, 2), "058.stage");
    sensitivity = parseDouble(blockette.substr(2, 12),"058.sensitivity");
    frequency = parseDouble(blockette.substr(14, 12),"058.frequency");

    num = parseInt(blockette.substr(26, 2), "058.num histories");

    j = 28;
    for(i = 0; i < num; i++) {
	cal_sensitivity.push_back(
		parseDouble(blockette.substr(j,12), "058.calSensitivity")); j+=12;
	cal_frequency.push_back(
		parseDouble(blockette.substr(j,12), "058.calFrequency")); j+=12;
	v = variable(blockette.substr(j));
	cal_time.push_back(SeedTime(v, "058.time")); j += v.length() + 1;
    }
}

Blockette58::Blockette58(int stage_number, const Blockette48 &b)
		: Seed("058"), from_b48(true)
{
    stage = stage_number;
    sensitivity = b.sensitivity;
    frequency = b.frequency;

    cal_sensitivity.assign(b.cal_sensitivity.begin(), b.cal_sensitivity.end());
    cal_frequency.assign(b.cal_frequency.begin(), b.cal_frequency.end());
    cal_time.assign(b.cal_time.begin(), b.cal_time.end());
}

string Blockette58::str()
{
    ostringstream out;
    out << "stage=" << stage << " "
	<< setiosflags(ios::scientific) << setprecision(5)
	<< "sensitivity=" << sensitivity << " "
	<< "freq=" << frequency << " "
	<< "time=" << (int)cal_time.size();
    return out.str();
}

string Blockette58::longStr()
{
    char s[80];
    ostringstream out;
    out << "stage sequence number:    " << stage << endl;
    snprintf(s, sizeof(s), "%12.5e", sensitivity);
    out << "sensitivity/gain:         " << s << endl;
    snprintf(s, sizeof(s), "%12.5e", frequency);
    out << "frequency (Hz):           " << s << endl;
    out << "number of history values: " << (int)cal_time.size();

    if((int)cal_time.size() > 0) {
	out << endl << "sensitivity   frequency     time" << endl;
	for(int i = 0; i < (int)cal_time.size(); i++) {
	    snprintf(s, sizeof(s), "\n%12.5e  %12.5e  %s",
		cal_sensitivity[i], cal_frequency[i],cal_time[i].str().c_str());
	    out << s;
	}
    }
    return out.str();
}

void Blockette59::load(const string &blockette) throw(FormatException)
{
    int j;
    string v;

    v = variable(blockette.substr(0));
    beg = SeedTime(v, "059.beginning_time");
    j = v.length() + 1;
    v = variable(blockette.substr(j));
    end = SeedTime(v, "059.end_time");
    j += v.length() + 1;
    comment_code = parseInt(blockette.substr(j, 4),"059.commentCode"); j += 4;
    comment_level = parseInt(blockette.substr(j, 6),"059.commentLevel");
}

string Blockette59::str()
{
    ostringstream out;
    out << "beg=" << beg.str() << " "
	<< "end=" << end.str() << " "
	<< "code=" << comment_code << " "
	<< "level=" << comment_level;
    return out.str();
}

string Blockette59::longStr()
{
    ostringstream out;
    out << "beginning effective time: " << beg.str() << endl
	<< "end effective time:       " << end.str() << endl
	<< "comment code key:         " << comment_code << endl
	<< "comment level:            " << comment_level;
    return out.str();
}

void Blockette60::load(const string &blockette) throw(FormatException)
{
    int i, j, k, num_stages, num;

    num_stages = parseInt(blockette.substr(0, 2),"060.numStages");
    j = 2;
    for(i = 0; i < num_stages; i++) {
	ResponseStage r;
	r.stage = parseInt(blockette.substr(j, 2),"060.stage"); j += 2;

	num = parseInt(blockette.substr(j, 2),"060.numResponses"); j += 2;

	for(k = 0; k < num; k++) {
	    r.code.push_back(
		parseInt(blockette.substr(j, 4), "060.lookupCode")); j += 4;
	}
	response.push_back(r);
    }
}

string Blockette60::str()
{
    ostringstream out;
    out << "num_stages=" << (int)response.size();
    return out.str();
}

string Blockette60::longStr()
{
    ostringstream out;
    out << "number of stages: " << (int)response.size();
    if((int)response.size() > 0) {
	for(int i = 0; i < (int)response.size(); i++) {
	    out << endl << "stage sequence number:   " << response[i].stage;
	    out << endl << "    number of responses: "
				<< (int)response[i].code.size();
	    for(int j = 0; j < (int)response[i].code.size(); j++) {
		out << endl << "    " << response[i].code[j];
	    }
	}
    }
    return out.str();
}

void Blockette61::load(const string &blockette) throw(FormatException)
{
    int i, j, num;

    stage = parseInt(blockette.substr(0, 2), "061.stage");
    name = variable(blockette.substr(2));
    j = 2 + name.length() + 1;
    symmetry_code = blockette.substr(j,1); j++;
    input_units = parseInt(blockette.substr(j, 3),"061.inputUnits"); j += 3;
    output_units = parseInt(blockette.substr(j, 3),"061.outputUnits"); j += 3;
    num = parseInt(blockette.substr(j, 4), "061.numFactors"); j += 4;

    for(i = 0; i < num; i++) {
	coef.push_back(parseDouble(blockette.substr(j, 14), "061.coefficient"));
	j += 14;
    }
}

Blockette61::Blockette61(int stage_number, const Blockette41 &b)
		: Seed("061"), from_b41(true)
{
    stage = stage_number;
    name = b.name;
    symmetry_code = b.symmetry_code;
    input_units = b.input_units;
    output_units = b.output_units;
    coef.assign(b.coef.begin(), b.coef.end());
}

string Blockette61::str()
{
    ostringstream out;
    out << "stage=" << stage << " "
	<< "name=" << name << " "
	<< "symmetry=" << symmetry_code << " "
	<< "input=" << input_units << " "
	<< "output=" << output_units << " "
	<< "num_coef=" << (int)coef.size();
    return out.str();
}

string Blockette61::longStr()
{
    char s[20];
    ostringstream out;
    out << "stage sequence number:  " << stage << endl
	<< "response name:          " << name << endl
	<< "symmetry code:          " << symmetry_code << endl
	<< "signal in units:        " << input_units << endl
	<< "signal out units:       " << output_units << endl
	<< "number of coefficients: " << (int)coef.size();
    if((int)coef.size() > 0) {
	out << endl;
	for(int i = 0; i < (int)coef.size(); i++) {
	    snprintf(s, sizeof(s), "%14.7e", coef[i]);
	    out << s;
	    if(i < (int)coef.size()-1) {
		out << " ";
		if((i+1) % 5 == 0) out << endl;
	    }
	}
    }
    return out.str();
}

void Blockette62::load(const string &blockette) throw(FormatException)
{
    int i, j, num;

    transfer_type = blockette.substr(0,1); j = 1;
    stage = parseInt(blockette.substr(j, 2), "062.stage"); j +=2;
    input_units = parseInt(blockette.substr(j, 3),"062.inputUnits"); j += 3;
    output_units = parseInt(blockette.substr(j, 3),"062.outputUnits"); j += 3;
    poly_type = blockette.substr(j,1); j++;
    freq_units = blockette.substr(j,1); j++;
    min_freq = parseDouble(blockette.substr(j, 12), "062.minFreq"); j += 12;
    max_freq = parseDouble(blockette.substr(j, 12), "062.maxFreq"); j += 12;
    min_approx = parseDouble(blockette.substr(j, 12), "062.minApprox"); j += 12;
    max_approx = parseDouble(blockette.substr(j, 12), "062.maxApprox"); j += 12;
    max_error = parseDouble(blockette.substr(j, 12), "062.maxError"); j += 12;
    num = parseInt(blockette.substr(j, 3), "062.numFactors"); j += 3;

    for(i = 0; i < num; i++) {
	coef.push_back(parseDouble(blockette.substr(j, 12), "062.coefficient")); j += 12;
	error.push_back(parseDouble(blockette.substr(j, 12), "062.error")); j += 12;
    }
}

string Blockette62::str()
{
    ostringstream out;
    out << "type=" << transfer_type << " "
	<< "stage=" << stage << " "
	<< "input=" << input_units << " "
	<< "output=" << output_units << " "
	<< "poly_type=" << poly_type << " "
	<< "freq_units=" << freq_units << " "
	<< setiosflags(ios::scientific) << setprecision(5)
	<< "min_freq=" << min_freq << " "
	<< "max_freq=" << max_freq << " "
	<< "min_approx=" << min_approx << " "
	<< "max_approx=" << max_approx << " "
	<< "max_error=" << max_error << " "
	<< "num_coef=" << (int)coef.size();
    return out.str();
}

string Blockette62::longStr()
{
    char s[50];
    ostringstream out;
    out << "transfer function type:       " << transfer_type << endl
	<< "stage sequence number:        " << stage << endl
	<< "stage signal input units:     " << input_units << endl
	<< "stage signal output units:    " << output_units << endl
	<< "polynomial approximation:     " << poly_type << endl
	<< "valid frequency units:        " << freq_units << endl
	<< setiosflags(ios::scientific) << setprecision(5)
	<< "lower valid frequency bound:  " << min_freq << endl
	<< "upper valid frequency bound:  " << max_freq << endl
	<< "lower bound of approximation: " << min_approx << endl
	<< "upper bound of approximation: " << max_approx << endl
	<< "maximum absolute error:       " << max_error << endl
	<< "number of poly coefficients:  " << (int)coef.size();
    if((int)coef.size() > 0) {
	out << endl << "coefficient    error";
	for(int i = 0; i < (int)coef.size(); i++) {
	    snprintf(s, sizeof(s), "\n%12.5e   %12.5e", coef[i], error[i]);
	    out << s;
	}
    }
    return out.str();
}

void Blockette70::load(const string &blockette) throw(FormatException)
{
    int j;
    string v;

    flag = blockette.substr(0, 1);
    v = variable(blockette.substr(1));
    beg = SeedTime(v, "070.beginning_time");
    j = 1 + v.length() + 1;
    v = variable(blockette.substr(j));
    end = SeedTime(v, "070.end_time");
}

string Blockette70::str()
{
    ostringstream out;
    out << "flag=" << flag << " "
	<< "beg=" << beg.str() << "  "
	<< "end=" << end.str();
    return out.str();
}

void Blockette71::load(const string &blockette) throw(FormatException)
{
    int i, j, num;
    string v;

    v = variable(blockette.substr(0));
    origin_time = SeedTime(v, "071.origin_time");
    j = v.length() + 1;

    source_code = parseInt(blockette.substr(j, 2),"071.sourceCode"); j += 2;
    latitude = parseDouble(blockette.substr(j, 10),"071.latitude"); j += 10;
    longitude = parseDouble(blockette.substr(j, 11),"071.longitude"); j += 11;
    depth = parseDouble(blockette.substr(j, 7),"071.depth"); j += 7;

    num = parseInt(blockette.substr(j, 2),"071.numMagnitudes"); j += 2;

    for(i = 0; i < num; i++) {
	magnitude.push_back(parseDouble(blockette.substr(j,5), "071.magnitude")); j+=5;
	mag_type.push_back(variable(blockette.substr(j)));
	j += mag_type[i].length() + 1;
	mag_source.push_back(parseInt(blockette.substr(j,2), "071.magSource")); j+=2;
    }

    if((int)blockette.length() >= j + 7) {
	// version >= 2.3
	seismic_region = parseInt(blockette.substr(j, 3), "071.seismicRegion");
	j += 3;
	seismic_location = parseInt(blockette.substr(j, 4),
				"071.seismicLocation");
	j += 4;
	region_name = variable(blockette.substr(j));
    }
}

string Blockette71::str()
{
    ostringstream out;
    out << "time=" << origin_time.str() << " "
	<< "hypo=" << source_code << " "
	<< setiosflags(ios::fixed) << setprecision(6)
	<< "lat=" << latitude << " "
	<< "lon=" << longitude << " "
	<< setprecision(2)
	<< "depth=" << depth << " "
	<< "num_mag=" << (int)magnitude.size() << " "
	<< "region=" << seismic_region << " "
	<< "loc=" << seismic_location << " "
	<< "name=" << region_name;
    return out.str();
}

string Blockette71::longStr()
{
    char s[60];
    ostringstream out;
    out << "origin time of event:         " << origin_time.str() << endl
	<< "hypocenter source identifier: " << source_code << endl
	<< setiosflags(ios::fixed) << setprecision(6)
	<< "latitude of event:            " << latitude << endl
	<< "longitude of event:           " << longitude << endl
	<< setprecision(2)
	<< "depth (km):                   " << depth << endl
	<< "number of magnitudes:         " << (int)magnitude.size();

    if((int)magnitude.size() > 0) {
	out << endl << "    magnitude  type        source";
	for(int i = 0; i < (int)magnitude.size(); i++) {
	    snprintf(s, sizeof(s), "\n    %5.2f      %-10.10s  %d",
		magnitude[i], mag_type[i].c_str(), mag_source[i]);
	    out << s;
	}
    }
    out << endl
	<< "seismic region:   " << seismic_region << endl
	<< "seismic location: " << seismic_location << endl
	<< "region name:      " << region_name;
    return out.str();
}

void Blockette72::load(const string &blockette) throw(FormatException)
{
    int j;
    string v;

    station = trim(blockette.substr(0, 5));
    location = trim(blockette.substr(5, 2));
    channel = trim(blockette.substr(7, 3));
    v = variable(blockette.substr(10));
    time = SeedTime(v, "072.arrival_time");
    j = 10 + v.length() + 1;

    amplitude = parseDouble(blockette.substr(j, 10),"072.amplitude"); j += 10;
    period = parseDouble(blockette.substr(j, 10),"072.period"); j += 10;
    snr = parseDouble(blockette.substr(j, 10),"072.snr"); j += 10;
    phase_name = variable(blockette.substr(j));
    j += phase_name.length() + 1;

    if((int)blockette.length() >= j + 4) {
	// version >= 2.3
	source = parseInt(blockette.substr(j, 2),"072.source"); j += 2;
	network = trim(blockette.substr(j, 2));
    }
}

string Blockette72::str()
{
    ostringstream out;
    out << "sta=" << station << " "
	<< "loc=" << location << " "
	<< "chan=" << channel << " "
	<< "time=" << time.str() << " "
	<< setiosflags(ios::scientific) << setprecision(4)
	<< "amp=" << amplitude << " "
	<< "period=" << period << " "
	<< "snr=" << snr << " "
	<< "phase=" << phase_name << " "
	<< "src=" << source << " "
	<< "net=" << network;
    return out.str();
}

string Blockette72::longStr()
{
    ostringstream out;
    out << "station:       " << station << endl
	<< "location:      " << location << endl
	<< "channel:       " << channel << endl
	<< "arrival time:  " << time.str() << endl
	<< setiosflags(ios::scientific) << setprecision(4)
	<< "amplitude:     " << amplitude << endl
	<< "period:        " << period << endl
	<< "snr:           " << snr << endl
	<< "name of phase: " << phase_name << endl
	<< "source:        " << source << endl
	<< "network:       " << network;
    return out.str();
}

void Blockette73::load(const string &blockette) throw(FormatException)
{
    int i, j, num;
    string v;

    num = parseInt(blockette.substr(0, 4), "073.num data");

    j = 4;
    for(i = 0; i < num; i++) {
	station.push_back(trim(blockette.substr(j, 5))); j += 5;
	location.push_back(trim(blockette.substr(j, 2))); j += 2;
	channel.push_back(trim(blockette.substr(j, 3))); j += 3;
	v = variable(blockette.substr(j));
	time.push_back(SeedTime(v, "073.time"));
	j += v.length() + 1;
	seqno.push_back(parseInt(blockette.substr(j, 6),"073.seqno")); j += 6;
	subseqno.push_back(parseInt(blockette.substr(j, 2),"073.subseqno")); j += 2;
    }
}

string Blockette73::str()
{
    ostringstream out;
    out << "num=" << (int)station.size();
    return out.str();
}

void Blockette74::load(const string &blockette) throw(FormatException)
{
    int i, j, num;
    string v;

    station = trim(blockette.substr(0, 5));
    location = trim(blockette.substr(5, 2));
    channel = trim(blockette.substr(7, 3));
    v = variable(blockette.substr(10));
    start_time = SeedTime(v, "074.series_start_time");
    j = 10 + v.length() + 1;
    start_seqno = parseInt(blockette.substr(j, 6), "074.startSeqno"); j += 6;
    start_subseqno = parseInt(blockette.substr(j, 2), "074.startSubseqno"); j += 2;
    v = variable(blockette.substr(j));
    end_time = SeedTime(v, "074.series_end_time");
    j += v.length() + 1;
    end_seqno = parseInt(blockette.substr(j, 6),"074.endSeqno"); j += 6;
    end_subseqno = parseInt(blockette.substr(j, 2),"074.endSubseqno"); j += 2;

    num = parseInt(blockette.substr(j, 3),"074.numAccels"); j += 3;

    for(i = 0; i < num; i++) {
	v = variable(blockette.substr(j));
	accel_time.push_back(SeedTime(v, "074.record_time"));
	j += v.length() + 1;
	accel_seqno.push_back(parseInt(blockette.substr(j, 6),"074.accelSeqno")); j += 6;
	accel_subseqno.push_back(parseInt(blockette.substr(j, 2),"074.accelSubseqno")); j += 2;
    }
    if((int)blockette.length() >= j + 2) {
	network = trim(blockette.substr(j, 2));
    }
}

string Blockette74::str()
{
    char s[200];
    ostringstream out;
    snprintf(s, sizeof(s),
	"%-5.5s %-2.2s %-3.3s %-22.22s %6d %2d %-22.22s %6d %2d %3d %-2.2s",
	station.c_str(), location.c_str(), channel.c_str(),
	start_time.str().c_str(), start_seqno, start_subseqno,
	end_time.str().c_str(), end_seqno, end_subseqno, (int)accel_time.size(),
	network.c_str());
    out << s;
/*
    out << "sta=" << station << " "
	<< "loc=" << location << " "
	<< "chan=" << channel << " "
	<< "start=" << start_time.str() << " "
	<< "startno=" << start_seqno << " "
	<< "startsub=" << start_subseqno << " "
	<< "end=" << end_time.str() << " "
	<< "endno=" << end_seqno << " "
	<< "endsub=" << end_subseqno << " "
	<< "num_accel=" << (int)accel_time.size() << " "
	<< "net=" << network;
*/
    return out.str();
}

string Blockette74::longStr()
{
    char s[100];
    ostringstream out;
    out << "station:                        " << station << endl
	<< "location:                       " << location << endl
	<< "channel:                        " << channel << endl
	<< "series start time:              " << start_time.str() << endl
	<< "sequence number of first data:  " << start_seqno << endl
	<< "sub-sequence number:            " << start_subseqno << endl
	<< "series end time:                " << end_time.str() << endl
	<< "sequence number of last record: " << end_seqno << endl
	<< "sub-sequence number:            " << end_subseqno << endl
	<< "number of accelerator repeats:  " << (int)accel_time.size();
    if((int)accel_time.size() > 0) {
	out << endl << "    record start time       seqno   sub-seqno";
	for(int i = 0; i < (int)accel_time.size(); i++) {
	    snprintf(s, sizeof(s), "\n    %-22.22s  %6d  %2d",
		accel_time[i].str().c_str(), accel_seqno[i], accel_subseqno[i]);
	    out << s;
	}
    }
    out << endl << "network: " << network;
    return out.str();
}

void Blockette100::loadDB(const string &bytes, int *wo, int *so)
			throw(FormatException)
{
    int i;
    FLOAT u;

    if(bytes.length() < 8) {
	throw LenException("Short Sample Rate Blockette.", type);
    }
    for(i = 0; i < 4; i++) u.a[wo[i]] = bytes[i];
    sample_rate = u.f;

    flags = bytes[4];

    reserved[0] = bytes[5];
    reserved[1] = bytes[6];
    reserved[2] = bytes[7];
}

string Blockette100::str()
{
    char out[20];
    snprintf(out, sizeof(out), "samprate=%.4g", sample_rate);
    return string(out);
}

void Blockette200::loadDB(const string &bytes, int *wo, int *so)
			throw(FormatException)
{
    int i;
    FLOAT u;

    if((int)bytes.length() < 48) {
	throw LenException("Short Sample Rate Blockette.", type);
    }

    for(i = 0; i < 4; i++) u.a[wo[i]] = bytes[i];
    amplitude = u.f;
    for(i = 0; i < 4; i++) u.a[wo[i]] = bytes[4+i];
    period = u.f;
    for(i = 0; i < 4; i++) u.a[wo[i]] = bytes[8+i];
    background = u.f;

    flags = bytes[12];
    reserved = bytes[13];

    time = SeedTime(bytes.substr(14), so);
    name = trim(bytes.substr(24, 24));
}

string Blockette200::str()
{
    ostringstream out;
    out << setiosflags(ios::scientific) << setprecision(4)
	<< "amp=" << amplitude << " "
	<< "per=" << period << " "
	<< "background=" << background << " "
	<< "time=" << time.str() << " "
	<< "name=" << name;
    return out.str();
}

void Blockette201::loadDB(const string &bytes, int *wo, int *so)
			throw(FormatException)
{
    int i;
    FLOAT u;

    if((int)bytes.length() < 56) {
	throw LenException("Short Murdock Detection Blockette.", type);
    }

    for(i = 0; i < 4; i++) u.a[wo[i]] = bytes[i];
    amplitude = u.f;
    for(i = 0; i < 4; i++) u.a[wo[i]] = bytes[4+i];
    period = u.f;
    for(i = 0; i < 4; i++) u.a[wo[i]] = bytes[8+i];
    background = u.f;

    flags = bytes[12];
    reserved = bytes[13];

    time = SeedTime(bytes.substr(14), so);
    for(i = 0; i < 6; i++) snr[i] = bytes[24+i];

    look_back = bytes[30];
    algorithm = bytes[31];

    name = trim(bytes.substr(32, 24));
}

string Blockette201::str()
{
    ostringstream out;
    out << setiosflags(ios::scientific) << setprecision(4)
	<< "amp=" << amplitude << " "
	<< "per=" << period << " "
	<< "background=" << background << " "
	<< "time=" << time.str() << " "
	<< "snr=" << snr << " "
	<< "name=" << name;
    return out.str();
}

void Blockette300::loadDB(const string &bytes, int *wo, int *so)
			throw(FormatException)
{
    int i;
    ULONG u;
    FLOAT f;

    if((int)bytes.length() < 56) {
	throw LenException("Short Step Calibration Blockette.", type);
    }

    time = SeedTime(bytes, so);

    num_steps = (int)((unsigned char )bytes[10]);
    flags = bytes[11];

    for(i = 0; i < 4; i++) u.a[wo[i]] = bytes[12+i];
    step = u.i;
    for(i = 0; i < 4; i++) u.a[wo[i]] = bytes[16+i];
    interval = u.i;
    for(i = 0; i < 4; i++) f.a[wo[i]] = bytes[20+i];
    amplitude = f.f;
    channel = trim(bytes.substr(24, 3));
    reserved = bytes[27];
    for(i = 0; i < 4; i++) u.a[wo[i]] = bytes[28+i];
    ref_amp = u.i;
    coupling = trim(bytes.substr(32, 12));
    rolloff = trim(bytes.substr(44, 12));
}

string Blockette300::str()
{
    ostringstream out;
    out << setiosflags(ios::scientific) << setprecision(4)
	<< "time=" << time.str() << " "
	<< "num_steps=" << num_steps << " "
	<< "step=" << step << " "
	<< "interval=" << interval << " "
	<< "amp=" << amplitude << " "
	<< "chan=" << channel << " "
	<< "refamp=" << ref_amp << " "
	<< "coupling=" << coupling << " "
	<< "rolloff=" << rolloff;
    return out.str();
}

void Blockette310::loadDB(const string &bytes, int *wo, int *so)
			throw(FormatException)
{
    int i;
    ULONG u;
    FLOAT f;

    if((int)bytes.length() < 56) {
	throw LenException("Short Sine Calibration Blockette.", type);
    }

    time = SeedTime(bytes, so);

    reserved1 = bytes[10];
    flags = bytes[11];

    for(i = 0; i < 4; i++) u.a[wo[i]] = bytes[12+i];
    duration = u.i;
    for(i = 0; i < 4; i++) f.a[wo[i]] = bytes[16+i];
    period = f.f;
    for(i = 0; i < 4; i++) f.a[wo[i]] = bytes[20+i];
    amplitude = f.f;
    channel = trim(bytes.substr(24, 3));
    reserved2 = bytes[27];
    for(i = 0; i < 4; i++) u.a[wo[i]] = bytes[28+i];
    ref_amp = u.i;
    coupling = trim(bytes.substr(32, 12));
    rolloff = trim(bytes.substr(44, 12));
}

string Blockette310::str()
{
    ostringstream out;
    out << setiosflags(ios::scientific) << setprecision(4)
	<< "time=" << time.str() << " "
	<< "dur=" << duration << " "
	<< "per=" << period << " "
	<< "amp=" << amplitude << " "
	<< "chan=" << channel << " "
	<< "refamp=" << ref_amp << " "
	<< "coupling=" << coupling << " "
	<< "rolloff=" << rolloff;
    return out.str();
}

void Blockette320::loadDB(const string &bytes, int *wo, int *so)
			throw(FormatException)
{
    int i;
    ULONG u;
    FLOAT f;

    if((int)bytes.length() < 60) {
	throw LenException("Short Pseudo-random Calibration Blockette.",
			type);
    }

    time = SeedTime(bytes, so);

    reserved1 = bytes[10];
    flags = bytes[11];

    for(i = 0; i < 4; i++) u.a[wo[i]] = bytes[12+i];
    duration = u.i;
    for(i = 0; i < 4; i++) f.a[wo[i]] = bytes[16+i];
    amplitude = f.f;
    channel = trim(bytes.substr(20, 3));
    reserved2 = bytes[23];
    for(i = 0; i < 4; i++) u.a[wo[i]] = bytes[24+i];
    ref_amp = u.i;
    coupling = trim(bytes.substr(28, 12));
    rolloff = trim(bytes.substr(40, 12));
    rolloff = trim(bytes.substr(52, 8));
}

string Blockette320::str()
{
    ostringstream out;
    out << setiosflags(ios::scientific) << setprecision(4)
	<< "time=" << time.str() << " "
	<< "dur=" << duration << " "
	<< "amp=" << amplitude << " "
	<< "chan=" << channel << " "
	<< "refamp=" << ref_amp << " "
	<< "coupling=" << coupling << " "
	<< "rolloff=" << rolloff << " "
	<< "noise=" << noise;
    return out.str();
}

void Blockette390::loadDB(const string &bytes, int *wo, int *so)
			throw(FormatException)
{
    int i;
    ULONG u;
    FLOAT f;

    if((int)bytes.length() < 24) {
	throw LenException("Short Pseudo-random Calibration Blockette.", type);
    }

    time = SeedTime(bytes, so);

    reserved1 = bytes[10];
    flags = bytes[11];

    for(i = 0; i < 4; i++) u.a[wo[i]] = bytes[12+i];
    duration = u.i;
    for(i = 0; i < 4; i++) f.a[wo[i]] = bytes[16+i];
    amplitude = f.f;
    channel = trim(bytes.substr(20, 3));
    reserved2 = bytes[23];
}

string Blockette390::str()
{
    ostringstream out;
    out << setiosflags(ios::scientific) << setprecision(4)
	<< "time=" << time.str() << " "
	<< "dur=" << duration << " "
	<< "amp=" << amplitude << " "
	<< "chan=" << channel;
    return out.str();
}

void Blockette395::loadDB(const string &bytes, int *wo, int *so)
			throw(FormatException)
{
    if((int)bytes.length() < 12) {
	throw LenException("Short Abort Calibration Blockette.", type);
    }

    end_time = SeedTime(bytes, so);

    reserved[0] = bytes[10];
    reserved[1] = bytes[11];
}

string Blockette395::str()
{
    ostringstream out;
    out << "end=" << end_time.str();
    return out.str();
}

void Blockette400::loadDB(const string &bytes, int *wo, int *so)
			throw(FormatException)
{
    int i;
    UWORD u;
    FLOAT f;

    if((int)bytes.length() < 12) {
	throw LenException("Short Beam Blockette.", type);
    }

    for(i = 0; i < 4; i++) f.a[wo[i]] = bytes[i];
    azimuth = f.f;
    for(i = 0; i < 4; i++) f.a[wo[i]] = bytes[4+i];
    slowness = f.f;

    for(i = 0; i < 2; i++) u.a[so[i]] = bytes[8+i];
    config = u.s;

    reserved[0] = bytes[10];
    reserved[1] = bytes[11];
}

string Blockette400::str()
{
    ostringstream out;
    out << setiosflags(ios::scientific) << setprecision(4)
	<< "az=" << azimuth << " "
	<< "slowness=" << slowness << " "
	<< "config=" << config;
    return out.str();
}

void Blockette405::loadDB(const string &bytes, int *wo, int *so)
			throw(FormatException)
{
    int i;
    UWORD u;

    if((int)bytes.length() < 2) {
	throw LenException("Short Beam Delay Blockette.", type);
    }
    for(i = 0; i < 2; i++) u.a[so[i]] = bytes[i];
    delay = u.s;
}

string Blockette405::str()
{
    ostringstream out;
    out << "delay=" << delay;
    return out.str();
}

void Blockette500::loadDB(const string &bytes, int *wo, int *so)
			throw(FormatException)
{
    int i;
    ULONG u;
    FLOAT f;

    if((int)bytes.length() < 196) {
	throw LenException("Short Timing Blockette.", type);
    }
    for(i = 0; i < 4; i++) f.a[wo[i]] = bytes[i];
    correction = f.f;
    time = SeedTime(bytes.substr(4, 10), so);
    micro_sec = (int)((signed char)bytes[14]);
    quality = (int)(unsigned char)bytes[15];
    for(i = 0; i < 4; i++) u.a[wo[i]] = bytes[16+i];
    count = u.i;
    type = trim(bytes.substr(20, 16));
    model = trim(bytes.substr(36, 32));
    status = trim(bytes.substr(68, 128));
}

string Blockette500::str()
{
    ostringstream out;
    out << setiosflags(ios::scientific) << setprecision(4)
	<< "corr=" << correction << " "
	<< "time=" << time.str() << " "
	<< "ms=" << micro_sec << " "
	<< "qual=" << quality << " "
	<< "count=" << count << " "
	<< "type=" << type << " "
	<< "model=" << model << " "
	<< "status=" << status;
    return out.str();
}

void Blockette1000::loadDB(const string &bytes, int *wo, int *so)
			throw(FormatException)
{
    if((int)bytes.length() < 4) {
	throw LenException("Short Data Only SEED Blockette.", type);
    }
    format = bytes[0];
    word_order = bytes[1];
    lreclen = (int)(unsigned char)bytes[2];
    reserved = bytes[3];
}

string Blockette1000::str()
{
    ostringstream out;
    out << "format=" << (int)format << " "
	<< "wo=" << (int)word_order << " "
	<< "lreclen=" << lreclen;
    return out.str();
}

void Blockette1001::loadDB(const string &bytes, int *wo, int *so)
			throw(FormatException)
{
    if((int)bytes.length() < 4) {
	throw LenException("Short Data Extension SEED Blockette.", type);
    }
    timing = (int)((unsigned char)bytes[0]);
    micro_sec = (int)((signed char)bytes[1]);
    reserved = bytes[3];
    count = (int)((unsigned char)bytes[2]);
}

string Blockette1001::str()
{
    ostringstream out;
    out << "timing=" << timing << " "
	<< "ms=" << micro_sec << " "
	<< "count=" << count;
    return out.str();
}

void Blockette2000::loadDB(const string &bytes, int *wo, int *so)
			throw(FormatException)
{
    int i, num;
    UWORD u;
    ULONG l;
    if((int)bytes.length() < 2) {
	throw LenException("Short Variable Length Opaque Data Blockette.",
			type);
    }
    for(i = 0; i < 2; i++) u.a[so[i]] = bytes[i];
    length = u.s;
    len = length;
    if((int)bytes.length() < length-4) {
	throw LenException("Short Variable Length Opaque Data Blockette.",
			type);
    }
    for(i = 0; i < 2; i++) u.a[so[i]] = bytes[2+i];
    offset = u.s;
    for(i = 0; i < 4; i++) l.a[wo[i]] = bytes[4+i];
    record = l.i;

    big_endian = bytes[8];
    flags = bytes[9];
    num = (int)((unsigned char)bytes[10]);
    if((int)bytes.length() < 0) {
	throw LenException("Number of header fields < 0", type);
    }

    if(num > 0) {
	fields = bytes.substr(11, num);
    }
    if(offset > length) {
	throw LenException("Data offset > blockette length", type);
    }

    int data_length = length - offset;
    if(data_length > 0) {
	data = bytes.substr(offset-4, data_length);
    }
}

string Blockette2000::str()
{
    ostringstream out;
    out << "nbytes=" << length << " "
	<< "offset=" << offset << " "
	<< "recno=" << record << " "
	<< "bigendian=" << (int)big_endian << " "
	<< "num=" << num_fields;
    return out.str();
}

Seed * Seed::createBlockette(const string &s)
{
    if(!s.compare("Blockette5")) return new Blockette5();
    else if(!s.compare("Blockette8")) return new Blockette8();
    else if(!s.compare("Blockette10")) return new Blockette10();
    else if(!s.compare("Blockette11")) return new Blockette11();
    else if(!s.compare("Blockette12")) return new Blockette12();
    else if(!s.compare("Blockette30")) return new Blockette30();
    else if(!s.compare("Blockette31")) return new Blockette31();
    else if(!s.compare("Blockette32")) return new Blockette32();
    else if(!s.compare("Blockette33")) return new Blockette33();
    else if(!s.compare("Blockette34")) return new Blockette34();
    else if(!s.compare("Blockette35")) return new Blockette35();
    else if(!s.compare("Blockette41")) return new Blockette41();
    else if(!s.compare("Blockette42")) return new Blockette42();
    else if(!s.compare("Blockette43")) return new Blockette43();
    else if(!s.compare("Blockette44")) return new Blockette44();
    else if(!s.compare("Blockette45")) return new Blockette45();
    else if(!s.compare("Blockette46")) return new Blockette46();
    else if(!s.compare("Blockette47")) return new Blockette47();
    else if(!s.compare("Blockette48")) return new Blockette48();
    else if(!s.compare("Blockette50")) return new Blockette50();
    else if(!s.compare("Blockette51")) return new Blockette51();
    else if(!s.compare("Blockette52")) return new Blockette52();
    else if(!s.compare("Blockette53")) return new Blockette53();
    else if(!s.compare("Blockette54")) return new Blockette54();
    else if(!s.compare("Blockette55")) return new Blockette55();
    else if(!s.compare("Blockette56")) return new Blockette56();
    else if(!s.compare("Blockette57")) return new Blockette57();
    else if(!s.compare("Blockette58")) return new Blockette58();
    else if(!s.compare("Blockette59")) return new Blockette59();
    else if(!s.compare("Blockette60")) return new Blockette60();
    else if(!s.compare("Blockette61")) return new Blockette61();
    else if(!s.compare("Blockette62")) return new Blockette62();
    else if(!s.compare("Blockette70")) return new Blockette70();
    else if(!s.compare("Blockette71")) return new Blockette71();
    else if(!s.compare("Blockette72")) return new Blockette72();
    else if(!s.compare("Blockette73")) return new Blockette73();
    else if(!s.compare("Blockette74")) return new Blockette74();
    else if(!s.compare("Blockette100")) return new Blockette100();
    else if(!s.compare("Blockette200")) return new Blockette200();
    else if(!s.compare("Blockette201")) return new Blockette201();
    else if(!s.compare("Blockette300")) return new Blockette300();
    else if(!s.compare("Blockette310")) return new Blockette310();
    else if(!s.compare("Blockette320")) return new Blockette320();
    else if(!s.compare("Blockette390")) return new Blockette390();
    else if(!s.compare("Blockette395")) return new Blockette395();
    else if(!s.compare("Blockette400")) return new Blockette400();
    else if(!s.compare("Blockette405")) return new Blockette405();
    else if(!s.compare("Blockette500")) return new Blockette500();
    else if(!s.compare("Blockette1000")) return new Blockette1000();
    else if(!s.compare("Blockette1001")) return new Blockette1001();
    else if(!s.compare("Blockette2000")) return new Blockette2000();
    return NULL;
}

DataBlockette * Seed::createDataBlockette(const string &s)
{
    if(!s.compare("Blockette100")) return new Blockette100();
    else if(!s.compare("Blockette200")) return new Blockette200();
    else if(!s.compare("Blockette201")) return new Blockette201();
    else if(!s.compare("Blockette300")) return new Blockette300();
    else if(!s.compare("Blockette310")) return new Blockette310();
    else if(!s.compare("Blockette320")) return new Blockette320();
    else if(!s.compare("Blockette390")) return new Blockette390();
    else if(!s.compare("Blockette395")) return new Blockette395();
    else if(!s.compare("Blockette400")) return new Blockette400();
    else if(!s.compare("Blockette405")) return new Blockette405();
    else if(!s.compare("Blockette500")) return new Blockette500();
    else if(!s.compare("Blockette1000")) return new Blockette1000();
    else if(!s.compare("Blockette1001")) return new Blockette1001();
    else if(!s.compare("Blockette2000")) return new Blockette2000();
    return NULL;
}

DataBlockette * Seed::createDataBlockette(DataBlockette &s)
{
    Blockette100 *a100, *b100;
    Blockette200 *a200, *b200;
    Blockette201 *a201, *b201;
    Blockette300 *a300, *b300;
    Blockette310 *a310, *b310;
    Blockette320 *a320, *b320;
    Blockette390 *a390, *b390;
    Blockette395 *a395, *b395;
    Blockette400 *a400, *b400;
    Blockette405 *a405, *b405;
    Blockette500 *a500, *b500;
    Blockette1000 *a1000, *b1000;
    Blockette1001 *a1001, *b1001;
    Blockette2000 *a2000, *b2000;

    if( (a100 = s.getBlockette100()) ) {
	b100 = new Blockette100(); *b100 = *a100; return b100; }
    else if( (a200 = s.getBlockette200()) ) {
	b200 = new Blockette200(); *b200 = *a200; return b200; }
    else if( (a201 = s.getBlockette201()) ) {
	b201 = new Blockette201(); *b201 = *a201; return b201; }
    else if( (a300 = s.getBlockette300()) ) {
	b300 = new Blockette300(); *b300 = *a300; return b300; }
    else if( (a310 = s.getBlockette310()) ) {
	b310 = new Blockette310(); *b310 = *a310; return b310; }
    else if( (a320 = s.getBlockette320()) ) {
	b320 = new Blockette320(); *b320 = *a320; return b320; }
    else if( (a390 = s.getBlockette390()) ) {
	b390 = new Blockette390(); *b390 = *a390; return b390; }
    else if( (a395 = s.getBlockette395()) ) {
	b395 = new Blockette395(); *b395 = *a395; return b395; }
    else if( (a400 = s.getBlockette400()) ) {
	b400 = new Blockette400(); *b400 = *a400; return b400; }
    else if( (a405 = s.getBlockette405()) ) {
	b405 = new Blockette405(); *b405 = *a405; return b405; }
    else if( (a500 = s.getBlockette500()) ) {
	b500 = new Blockette500(); *b500 = *a500; return b500; }
    else if( (a1000 = s.getBlockette1000()) ) {
	b1000 = new Blockette1000(); *b1000 = *a1000; return b1000; }
    else if( (a1001 = s.getBlockette1001()) ) {
	b1001 = new Blockette1001(); *b1001 = *a1001; return b1001; }
    else if( (a2000 = s.getBlockette2000()) ) {
	b2000 = new Blockette2000(); *b2000 = *a2000; return b2000; }
    return NULL;
}
