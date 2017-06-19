#ifndef __SINUS_SOUND__
#define __SINUS_SOUND__

/**
* Synthé sinusoidal.
*/
#include "continuous_sound.h"

class SinusSound : public ContinuousSound
{
public:
	
	SinusSound() : ContinuousSound()
	{
		_T = 0;
		setFreq(440,1.0f);
		_CurFreqSin = 440;
		_NbEch = 0;
		_PrevVal = 0;
		_DeltaSin = 0;
		_Phase = 0;
		_CurrRnd = 0;
	}

	void setFreq(float freq, float stepFreqPerPeriod)
	{
		_FreqSin = freq;
		_StepFreq = stepFreqPerPeriod;
	}
private:
	float _T;
	float _FreqSin;
	float _CurFreqSin;
	float _StepFreq;
	int _NbEch;
	float _DeltaSin;
	float _PrevVal;
	float _Phase;
	float _CurrRnd ;
	float _DureeTransition;

protected :
	virtual void init()
	{
		_T = 0;
	}

	/**
	  * Remplissage du buffer de synthèse, jusqu'au pointeur de lecture
	  */
	virtual float getNextSample()
	{
		//On set l'echantillon
		float val =  sin(_T * _CurFreqSin * 2.0f * M_PI + _Phase);

		//Si fin de periode
		if(_PrevVal < 0 && val >= 0)
		{
			if(abs(_CurFreqSin - _FreqSin) > _StepFreq*2)
			{
				_CurFreqSin += (_CurFreqSin < _FreqSin) ? _StepFreq : -_StepFreq;
				_Phase = asin(val);
				_Phase = nanToZero(_Phase);
				_T = 0;
			}
			else
			{
				if(_CurFreqSin != _FreqSin)
				{
					_CurFreqSin = _FreqSin;
					_Phase = asin(val);
					_Phase = nanToZero(_Phase);
					_T = 0;
				}
			}
		}
			
		//Valeur precedente
		_PrevVal = val;

		//On ecrit
		val = val * 0.5f;	

		//Le temps avance
		_T += 1/_Frequency;

		_NbEch++;

		return val;
	}	

};


#endif
