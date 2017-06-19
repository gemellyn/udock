#ifndef __FILTER_REVERB__
#define __FILTER_REVERB__

/**
*Filtre abstrait
*/
#include "filter.h"

#define NB_SAMPLES_REVERB 80000

class FilterReverb : public FilterAudio
{
public:

	void setReverb(float reverb)
	{
		_Amount = reverb;
	}

	FilterReverb() : FilterAudio()
	{
		_PosDelay = 0;
		_Amount = 0.5f;
		_NbSteps = 10;
		memset(_Delay,0,sizeof(float) * NB_SAMPLES_REVERB);
	}

	void init()
	{
		memset(_Delay,0,sizeof(float) * NB_SAMPLES_REVERB);
	}

	virtual float doFilter(float ech)
	{
		/*float reverb = 0.0f;
		float gain = 0.9f;
		int nbSteps = 20;
		float offsetBuffer = NB_SAMPLES_REVERB / (nbSteps+1);
		for(int i=0;i<nbSteps;i++)
		{
			int posOld = _PosDelay-((offsetBuffer)*(i+1));
			while(posOld < 0)
				posOld += NB_SAMPLES_REVERB;
			reverb += _Delay[posOld] * gain - ech * gain;
			gain *= gain;
		}

		float res = ech + reverb * 0.3f * _Amount;

		_Delay[_PosDelay] = res;
		
		_PosDelay++;
		if(_PosDelay >= NB_SAMPLES_REVERB)
			_PosDelay = 0;*/

		/*float gain = 0.3f;
		float reverb = 0.0f;
		float offsetBuffer = NB_SAMPLES_REVERB / (_NbSteps+1);
		float stock = ech;

		for(int i=0;i<_NbSteps;i++)
		{
			int posOldEch = _PosDelay-((offsetBuffer)*(i+1));
			while(posOldEch < 0)
				posOldEch += NB_SAMPLES_REVERB;

			float oldEch = _Delay[posOldEch];
			stock += oldEch * gain;
			reverb += stock;
			gain *= gain + (1.0f-gain)*0.4f;
		}

		_Delay[_PosDelay] = stock;
		
		_PosDelay++;
		if(_PosDelay >= NB_SAMPLES_REVERB)
			_PosDelay = 0;

		return ech + _Delay[_PosDelay]-1;*/

		float reverb = 0.0f;
		float gain = 0.9f;
		
		float offsetBuffer = NB_SAMPLES_REVERB / (_NbSteps+1);
		for(int i=0;i<_NbSteps;i++)
		{
			int posOld = _PosDelay-((offsetBuffer)*(i+1));
			while(posOld < 0)
				posOld += NB_SAMPLES_REVERB;
			reverb += _Delay[posOld] * gain - ech * gain;
			gain *= gain;
		}

		float res = ech + reverb * 0.3f;

		_Delay[_PosDelay] = res;

		_PosDelay++;
		if(_PosDelay >= NB_SAMPLES_REVERB)
			_PosDelay = 0;

		return  ech + reverb * 0.3f * _Amount;
	}	

private :
	float _Delay[NB_SAMPLES_REVERB];
	int _PosDelay;
	float _Amount;
	int _NbSteps;


};


#endif
