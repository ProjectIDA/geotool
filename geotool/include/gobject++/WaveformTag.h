#ifndef WAVEFORM_TAG_H_
#define WAVEFORM_TAG_H_

#include <string>
#include <vector>

using namespace std;

/**
 *  @ingroup libgobject
 */
class WaveformTag
{
    public:

	WaveformTag(void) { }
        WaveformTag(const WaveformTag &w);
        WaveformTag & operator=(const WaveformTag &w);
	~WaveformTag(void) { }

	void setMembers(int num, const int *tag_members, string tag_string);
	vector<int> members;
	string ud_string;
};

#endif
