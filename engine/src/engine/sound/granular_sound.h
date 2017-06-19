#ifndef __GRANULAR_SOUND__
#define __GRANULAR_SOUND__

/**
* Synthé granulaire.
* Lui charger un fichier de base ou il pioche les grains
* utiliser setGrainParam pour modifier les params d'extraction et d'ajout des grains
*/

#include "continuous_sound.h"

#define GRAIN_SYNTHE_BUFFER_LENGTH 1.0f //On prend 1s de buffer en memoire

/**
  * Utilise plusieurs buffers :
  * un buffer qui contient le fichier de base
  * un buffer dans lequel on fait la synthèse : on y ajoute les grains les un a la suite des autres
  *   en tenant compte de params de synthèse. C'est un buffer tournant.
  * un buffer temporaire qui permet de créer
  */
class SoundGrain : public ContinuousSound
{
private:

	//Datas du fichier de base
	float _DureeBase; ///< Duree du buffer de base
	void * _BaseBuffer; ///< Buffer qui contient les échantillons du fichier de base
	uint8 * _PtRdFinBaseBuffer; ///< Pointeur sur la fin du buffer de base
	ALenum _Format; ///< Format du fichier de base (enum OpenAL comme AL_FORMAT_MONO16)
	ALsizei _Size; ///< Taille du fichier de base en octets
	ALfloat _Frequency; ///< Frequence d'échantillonage du fichier de base (44100 par exemple)
	int _NbPistes; ///< Nombre de pistes du fichier de base (1 mono, 2 stéréo)
	int _NbBits; ///< Nombre de bits par échantillon (16 ou 8, surtout utile pour affichage)
	int _TailleEchantillon; ///< Taille d'un échantillon en octets (plus utile pour les calculs)

	//Buffer pour faire la synthèse (buffer tournant)
	//Nécessaire car overlap des grains 
	float _DureeBufferSyntheGrain; ///<Duree du buffer de synthèse
	void * _DatasSyntheGrain; ///< Pointeur sur le début du buffer de synthèse
	uint8 * _PtFinBufferSyntheGrain; ///< Pointeur sur la fin du buffer de synthe
	int _TailleBufferSyntheGrain; ///< Taille du buffer de synthèse
	uint8 * _PtRdSyntheGrain; ///< Pointeur de lecture du buffer de synthese
	uint8 * _PtWrSyntheGrain; ///< Pointeur d'ecriture dans le buffer de synthe
	
	//Paramètres de la generation de grains
	float _PosGrainf; ///< Position de prise du grain entre 0 et 1
	float _DureeGrain; ///< Durée du grain en secondes 
	int _TailleGrain; ///< Taille du grain en octets
	int _PosGraini; ///< Position du grain en octets
	int _RandomWidth; //Taille du random quand on chope un grain
	int _SizeOverlap; //Taille d'overlapp entre les grains
	float _CrossFade; //Si on utilise un crossfade linéaire ou equalpower (signal corelé ou pas)

	//Pour la lecture des grains. 
	uint8 * _PtRdGrain; ///< Pointeur de lecture du grain courant
	uint8 * _PtRdFinGrain; ///< Pointeur de fin du grain courant
	int _TailleLastGrain; ///< Taille du grain courant

public :
	void loadBaseFile(char * filename)
	{
		//Chargement du fichier de base en mémoire (pas dans un buffer openal)
		_BaseBuffer = alutLoadMemoryFromFile(filename,&_Format,&_Size,&_Frequency);
		if (_BaseBuffer == NULL)
		{
			Log::log(Log::ENGINE_ERROR,("Error loading base file "+string(filename)+": "+string(alutGetErrorString(alutGetError()))).c_str());
			return;
		}		

		//On recup les params du fichier de base
		_NbPistes = (_Format == AL_FORMAT_STEREO8 || _Format == AL_FORMAT_STEREO16) ? 2 : 1;
		_NbBits = (_Format == AL_FORMAT_MONO16 || _Format == AL_FORMAT_STEREO16) ? 16 : 8;
		_TailleEchantillon = _NbPistes * (_NbBits == 16 ? 2 : 1);
		_DureeBase = (_Size / _Frequency / _TailleEchantillon);

		Log::log(Log::USER_INFO,("Chargement de "+std::string(filename)+":").c_str());
		Log::log(Log::USER_INFO,("- Nb Pistes : "+toString(_NbPistes)).c_str());
		Log::log(Log::USER_INFO,("- Format : "+toString(_NbBits) + " bits").c_str());
		Log::log(Log::USER_INFO,("- Frequence : "+toString(_Frequency) + " hz").c_str());
		Log::log(Log::USER_INFO,("- Taille d'un echantillon : "+toString(_TailleEchantillon) + " octets").c_str());
		Log::log(Log::USER_INFO,("- Nombre total d'echantillons : "+toString(_Size)).c_str());
		Log::log(Log::USER_INFO,("- Duree du fichier : "+toString(_DureeBase) + " seconds").c_str());	

		//On se crée un buffer tournant pour la synthèse
		_DureeBufferSyntheGrain = GRAIN_SYNTHE_BUFFER_LENGTH;
		_TailleBufferSyntheGrain = (int) ( _DureeBufferSyntheGrain * _Frequency * _TailleEchantillon );
		_TailleBufferSyntheGrain = (_TailleBufferSyntheGrain/4)*4; //alignement 16bits stereo
		_DatasSyntheGrain = new uint8[_TailleBufferSyntheGrain];
		memset(_DatasSyntheGrain,0,_TailleBufferSyntheGrain);
		_PtFinBufferSyntheGrain = ((uint8*)_DatasSyntheGrain) + _TailleBufferSyntheGrain;
		_PtRdSyntheGrain = ((uint8*)_DatasSyntheGrain);
		_PtWrSyntheGrain = ((uint8*)_DatasSyntheGrain) + _TailleEchantillon;

		//Init grain
		_PtRdGrain = NULL;
		_PtRdFinGrain = NULL;
		_TailleLastGrain = 0;

		//On se set des vars pour la lecture de grain
		_PtRdFinBaseBuffer = ((uint8*) _BaseBuffer) + _Size;

		//On set des params par defaut au cas ou
		setGrainParam(0.5f,0.2f,0.2f,0.05f,0.5f);
	}

	void unload()
	{
		SAFEDELETE_TAB(_DatasSyntheGrain);
		free(_BaseBuffer); _BaseBuffer = NULL; //AlutLoad* = malloc;
	}

	void setGrainParam(float posGrain, float dureeGrain, float randomPos, float partOverlap, float crossFade)
	{
		_PosGrainf = posGrain;
		_DureeGrain = dureeGrain;
		_TailleGrain = (int) ( _DureeGrain * _Frequency * _TailleEchantillon );
		_TailleGrain = (_TailleGrain/4)*4; //On s'aligne sur stereo 16 bits
		_PosGraini = (int) (_Size * _PosGrainf);
		_PosGraini = (_PosGraini / 4) * 4;  //On s'aligne sur 16 bits deux voies au cas ou 
		_RandomWidth = (int) ( randomPos * _Frequency *_TailleEchantillon );
		_SizeOverlap = (int ) ( partOverlap * _DureeGrain * _Frequency * _TailleEchantillon );
		_SizeOverlap = (_SizeOverlap/4)*4; //On aligne
		_CrossFade = crossFade;
	}


private :

	/**
	  * Phase de synthèse
	  * Cette méthode va lire le buffer qui contient le fichier de base pour en extraire de grains
	  * qu'elle va copier au fur et à mesure dans le buffer de synthèse
	  * Quand elle a fini la synthèse, elle retourne un échantillon pris dans le buffer de synthèse, 
	  * sous le pointeur de lecture, et avance ce pointeur de lecture.
	  */
	virtual float getNextSample()
	{
		//Tant qu'on a pas atteint le pointeur de lecture, on genere et on écrit
		while(_PtWrSyntheGrain != _PtRdSyntheGrain)
		{
			//Si besoin d'un nouveau grain (on est ptet toujours sur la fin du grain précédent)
			if(_PtRdGrain == NULL || _PtRdGrain >= _PtRdFinGrain)
			{
				//On va calculer la position ou on va extraire le grain			
				int posGrain = _PosGraini;
				
				//Si on a un offset aléatoire on le calcule et on l'ajoute
				int offsetRand = 0;
				if(_RandomWidth > 0)
					offsetRand = (int)((randf() * _RandomWidth) - (_RandomWidth/2));
				posGrain += offsetRand;
				posGrain = (posGrain/4)*4;  //On s'aligne 16bits stereo (multiple de 4) au cas ou

				//On place le pointeur de lecture sur le grain
				_PtRdGrain = (uint8*) _BaseBuffer;
				_PtRdGrain += posGrain;

				//On verifie les valeurs extremes (sortie du buffer)
				if(_PtRdGrain < (uint8*) _BaseBuffer)
					_PtRdGrain = (uint8*) _BaseBuffer;
				if(_PtRdGrain > _PtRdFinBaseBuffer)
					_PtRdGrain = _PtRdFinBaseBuffer;

				//On place un pointeur a la fin du grain
				_PtRdFinGrain = _PtRdGrain + _TailleGrain;
				if(_PtRdFinGrain > _PtRdFinBaseBuffer)
					_PtRdFinGrain = _PtRdFinBaseBuffer;

				//On sauve la taille du grain qu'on ajoute (utile par exemple pour le lissage)
				_TailleLastGrain = _PtRdFinGrain-_PtRdGrain;

				//On est sur un nouveau grain. Si les grains se revouvrent, il faut qu'on recule
				//le pointeur d'écriture dans le buffer pour qu'on vienne ensuite se mixer aux grains précédents
				for(int i=0;i<_SizeOverlap;i++)
				{
					//On recule
					_PtWrSyntheGrain --;

					//On fait le tour du buffer tournant si on recule plus loin que le début
					if(_PtWrSyntheGrain < (uint8*) _DatasSyntheGrain)
						_PtWrSyntheGrain = _PtFinBufferSyntheGrain-1;

					//Si on croise le pointeur de lecture, stop, c'est notre t=0. (Comme on génère
					//jusq'à ce qu'on croise le pointeur de lecture, le dernier octet juste avant le pointeur 
					//de lecture est l'échantillon avec le t maximal, et celui juste après le t=0)
					if(_PtWrSyntheGrain == _PtRdSyntheGrain)
					{
						_PtWrSyntheGrain = _PtRdSyntheGrain+_TailleEchantillon;
						if(_PtWrSyntheGrain >= _PtFinBufferSyntheGrain)
							_PtWrSyntheGrain = (uint8*) _DatasSyntheGrain;
						break;
					}
				}
			}

			//On ajoute le nouveau grain au buffer
			//On s'arrete si on atteint la fin du grain, ou si on croise le pointeur de lecture.
			while(_PtWrSyntheGrain != _PtRdSyntheGrain && _PtRdGrain < _PtRdFinGrain)
			{
				//Notre position à l'interieur du grain
				int pos = _TailleLastGrain - (_PtRdFinGrain - _PtRdGrain);

				//Calcul du fade
				float fade = 1.0f;

				//Part de fade linéaire et de fade "equal power"
				float linearEqualPowerBalance = _CrossFade;

				//Fade in
				if(pos <= _SizeOverlap)
				{
					//t entre 0 et 1 sur la duree du fade in
					float t = (float)pos / (float)_SizeOverlap;

					//On multiplie car si on a un fade in et un fade out > 0.5, on sera à un moment dans le fade in ET le fade out
					fade *=(1.0f-linearEqualPowerBalance) * (t) + linearEqualPowerBalance * sqrt(t);
				}

				//Fade out
				if(pos >= (_TailleLastGrain - _SizeOverlap))
				{
					//t entre 0 et 1 sur la duree du fade out
					float t = (float)(pos-(_TailleLastGrain - _SizeOverlap)) / (float)(_SizeOverlap);					

					//Idem, si fade in et fade out, on multiplie
					fade *= (1.0f-linearEqualPowerBalance) * (1.0f-t) + linearEqualPowerBalance * sqrt(1-t);
				}
			
				//On recup un échantillon du grain
				sint16 valRdi = *((sint16*)_PtRdGrain);
				float valRd = valRdi;

				//On recup la valeur actuelle présente dans le buffer de synthèse pour la mixer
				sint16 valWri = *((sint16*)_PtWrSyntheGrain);
				float valWr = valWri;

				//On calcule la nouvelle valeur
				float val = fade * valRd + valWr;

				//On écrit cette valeur dans notre buffer
				sint16 vali = (sint16)val;
				*((sint16*)_PtWrSyntheGrain) = vali;

				//On avance le pointeur d'écriture
				_PtWrSyntheGrain+=2;
				if(_PtWrSyntheGrain >= _PtFinBufferSyntheGrain)
					_PtWrSyntheGrain = (uint8*) _DatasSyntheGrain;
			
				//On avance le pointeur de lecture du grain
				_PtRdGrain+=2;
			}
		}

		//Ici on a atteint le pointeur de lecture, le buffer est plein.

		//On doit retourner un échantillon, au niveau du pointeur de lecture global
		float res = *((sint16*)_PtRdSyntheGrain); //On recup la valeur
		*((sint16*)_PtRdSyntheGrain) = 0; //On efface
		_PtRdSyntheGrain+=2; //On avance
		if(_PtRdSyntheGrain >= _PtFinBufferSyntheGrain)
			_PtRdSyntheGrain = (uint8*) _DatasSyntheGrain;

		//On retourne la valeur (attention, float entre -1 et 1)
		return res / (65535.0f/2.0f);
	}

	
};

#endif
