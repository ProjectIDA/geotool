/** \file FormatExceptions.h
 *  \brief Declares the runtime_error subclasses used by libseed.
 *  \author Ivan Henson
 */
#ifndef _FORMAT_EXCEPTION_H
#define _FORMAT_EXCEPTION_H

#include <stdio.h>
#include <string>
#include <stdexcept>
using namespace std;

class FormatException : public runtime_error
{
    public:
	FormatException(const string &s) : runtime_error(s) {}
	FormatException(const string &s, const string btype) :
		runtime_error(s), type(btype) {}
	FormatException(const string &s, const string btype,
		const string member_name) :
		runtime_error(s), type(btype), member(member_name) {}
	~FormatException() throw() {}

	string type;
	string member;
};

class SeqnoException : public FormatException
{
    public:
	SeqnoException(const string &s) : FormatException(s) {}
	~SeqnoException() throw() {}
};

class HdrException : public FormatException
{
    public:
	HdrException(const string &s) : FormatException(s) {}
	~HdrException() throw() {}
};

class FmtException : public FormatException
{
    public:
	FmtException(const string &s) : FormatException(s) {}
	~FmtException() throw() {}
};

class LenException : public FormatException
{
    public:
	LenException(const string &s) : FormatException(s) {}
	LenException(const string &s, const string btype) :
		FormatException(s, btype) {}
	~LenException() throw() {}
};

class SkipException : public FormatException
{
    public:
	SkipException(const string &s) : FormatException(s) {}
	~SkipException() throw() {}
};

class IOException : public FormatException
{
    public:
	IOException(const string &s) : FormatException(s) {}
	IOException() : FormatException("") {}
	~IOException() throw() {}
};

#endif
