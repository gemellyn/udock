#ifndef __FILTER_LP__
#define __FILTER_LP__

/**
*Filtre abstrait
*/
#include "filter.h"


class FilterLP : public FilterAudio
{
public:

	void setAlpha(float alpha)
	{
		_Alpha = alpha;
	}

	FilterLP() : FilterAudio()
	{
		_Last = 0;
		_Alpha = 0.1f;
	}

	virtual float doFilter(float ech)
	{
		//float res = _Alpha * _Last + (1.0f-_Alpha) * (ech - _Last);
		float res = _Last + (_Alpha) * (ech - _Last);
		_Last = res;
		return res;
	}	

private :
	float _Last;
	float _Alpha;


};


#endif
