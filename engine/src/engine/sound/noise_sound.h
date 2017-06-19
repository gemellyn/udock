#ifndef __NOISE_SOUND__
#define __NOISE_SOUND__

/**
* Bruit de variabilité ajustable
*/
#include "continuous_sound.h"

#define NB_FREQ_NOISE 8

class NoiseSound : public ContinuousSound
{
public:
	
	NoiseSound() : ContinuousSound()
	{		
		setFreq(440);
		for(int i=0;i<NB_FREQ_NOISE;i++)
		{
			_CurrRnd[i] = randf();
			_T[i] = 0;
		}
	}

	void setFreq(float freq)
	{
		_FreqNoise = freq;
	}

private:
	float _T[NB_FREQ_NOISE];
	float _FreqNoise;
	float _CurrRnd[NB_FREQ_NOISE] ;
	float _EqualFade;

protected :

	/**
	  * Remplissage du buffer de synthèse, jusqu'au pointeur de lecture
	  */
	virtual float getNextSample()
	{	
		//On set l'echantillon
		float val = 0;
			
		for(int i=0;i<NB_FREQ_NOISE;i++)
		{
			val += _CurrRnd[i];
				
			if(_T[i] > 1.0f/(_FreqNoise*(i+1)))
			{
				_CurrRnd[i] = randf();
				_T[i] = 0;
			}
			_T[i] += 1/_Frequency;
		}
		
		val /= NB_FREQ_NOISE;
		val = val * 2.0f - 1.0f;
						
		//On ecrit
		val = val * 0.5f;	

		return val;
	}	

};

#endif
