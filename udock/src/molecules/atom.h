#ifndef __ATOM_H__
#define __ATOM_H__

#include "engine/hasher_3d.h"
#include "engine/utils/types_3d.h"
#include "loaders/mol2_loader.h"

#define BIGGEST_ATOM_RADIUS 2.75f

/**
  *  Un atome, qui permet de composer des molécules
  */
class Atome : public Bucketable
{

	private:
		enum
		{
			C_3 = 0, //Pour chque atome, du plus court au plus long
			C_2,
			C_AR,
			C_CAT,
			N_3,
			N_2,
			N_4,
			N_AR,
			N_AM,
			N_PL3,
			O_3,
			O_2,
			O_CO2,
			S_3,
			P_3,
			F,
			H,
			LI,
			NB_ATOM_UDOCK_ID
		};

		static unsigned int genUID;

	public:
		static std::string _TypesStr[NB_ATOM_UDOCK_ID]; //Pour faire le lien avec chaque atome et son type sybyl
		static double _AtomsRadius[NB_ATOM_UDOCK_ID]; //Radius de chaque atome
		static double _AtomsEpsilons[NB_ATOM_UDOCK_ID]; //Voir cornell 1995
		static double _AtomsEpsilonsSquared[NB_ATOM_UDOCK_ID][NB_ATOM_UDOCK_ID]; //Voir cornell 1995, pour le calcul de Aij Bij
		static bool _InitOk;
	
	public :
		char _Name[4];
		char _Type[6];
		char _SubstName[4];
		unsigned int UID;
		int _UdockId;
		int _SubstId;
		float _Radius;
		float _Charge;
		float _ChargeLissee;
		float _Surface; ///< Il faut la calculer explicitement pour l'avoir (voir molecule)
		float _Accumulateur; ///< Pour compter des trucs sur les atomes
		char _Flags; ///< N'importe quel flag sur l'atome

	public :
		Atome() : UID(++genUID)
		{
			memset(_Name,0x00,4*sizeof(char));
			memset(_Type,0x00,6*sizeof(char));
			memset(_SubstName,0x00,4*sizeof(char));
			_Radius = 0.0f;
			_Charge = 0.0f;
			_ChargeLissee = 0.0f;
			_UdockId = -1;
			_Surface = 0.0f;
			_Accumulateur = 0.0f;


			if(!_InitOk)
			{
				_InitOk = true;
				_TypesStr[C_3] = "C.3";
				_TypesStr[C_2] = "C.2";
				_TypesStr[C_AR] = "C.ar";
				_TypesStr[C_CAT] = "C.cat";
				_TypesStr[N_3] = "N.3";
				_TypesStr[N_2] = "N.2";
				_TypesStr[N_4] = "N.4";
				_TypesStr[N_AR] = "N.ar";
				_TypesStr[N_AM] = "N.am";
				_TypesStr[N_PL3] = "N.pl3";
				_TypesStr[O_3] = "O.3";
				_TypesStr[O_2] = "O.2";
				_TypesStr[O_CO2] = "O.co2";
				_TypesStr[S_3] = "S.3";
				_TypesStr[P_3] = "P.3";
				_TypesStr[F] = "F";
				_TypesStr[H] = "H";
				_TypesStr[LI] = "Li";

				_AtomsRadius[C_3] = 1.908f;
				_AtomsRadius[C_2] = 1.908f;
				_AtomsRadius[C_AR] = 1.908f;
				_AtomsRadius[C_CAT] = 1.908f;
				_AtomsRadius[N_3] = 1.875f;
				_AtomsRadius[N_2] = 1.824f;
				_AtomsRadius[N_4] = 1.824f;
				_AtomsRadius[N_AR] = 1.824f;
				_AtomsRadius[N_AM] = 1.824f;
				_AtomsRadius[N_PL3] = 1.824f;
				_AtomsRadius[O_3] = 1.721f;
				_AtomsRadius[O_2] = 1.6612f;
				_AtomsRadius[O_CO2] = 1.6612f;
				_AtomsRadius[S_3] = 2.0f;
				_AtomsRadius[P_3] = 2.1f;
				_AtomsRadius[F] = 1.75f;
				_AtomsRadius[H] = 1.4870f;
				_AtomsRadius[LI] = 1.137f;

				_AtomsEpsilons[C_3] = 0.1094f;
				_AtomsEpsilons[C_2] = 0.0860f;
				_AtomsEpsilons[C_AR] = 0.0860f;
				_AtomsEpsilons[C_CAT] = 0.0860f;
				_AtomsEpsilons[N_3] = 0.17f;
				_AtomsEpsilons[N_2] = 0.17f;
				_AtomsEpsilons[N_4] = 0.17f;
				_AtomsEpsilons[N_AR] = 0.17f;
				_AtomsEpsilons[N_AM] = 0.17f;
				_AtomsEpsilons[N_PL3] = 0.17f;
				_AtomsEpsilons[O_3] = 0.2104f;
				_AtomsEpsilons[O_2] = 0.21f;
				_AtomsEpsilons[O_CO2] = 0.21f;
				_AtomsEpsilons[S_3] = 0.25f;
				_AtomsEpsilons[P_3] = 0.2f;
				_AtomsEpsilons[F] = 0.061f;
				_AtomsEpsilons[H] = 0.0157f;
				_AtomsEpsilons[LI] = 0.0183f;

				for(int i=0;i<NB_ATOM_UDOCK_ID;i++)
				{
					for(int j=0;j<NB_ATOM_UDOCK_ID;j++)
					{
						_AtomsEpsilonsSquared[i][j] = sqrt(_AtomsEpsilons[i]*_AtomsEpsilons[j]);
					}
				}
			}
		}

		void buildFromMol2Atom(Mol2Atom * mol2Atom) 
		{
			strncpy(_Name,mol2Atom->_Name,3);
			mol2Atom->_Name[3] = 0;
			strncpy(_Type,mol2Atom->_Type,5);
			mol2Atom->_Type[5] = 0;
			strncpy(_SubstName,mol2Atom->_SubstName,3);
			mol2Atom->_SubstName[3] = 0;
			_UdockId = findAtomUdockId(); 
			_Radius = (float)_AtomsRadius[_UdockId];
			_Charge = mol2Atom->_Charge;
			_Pos = mol2Atom->_Position;
			_SubstId = mol2Atom->_SubstId;
		}	

		static void resetgenUID()
		{
			genUID = 0;
		}

	private :
		

		int findAtomUdockId(void)
		{
			for(int i=0;i<NB_ATOM_UDOCK_ID;i++)
			{
				std::string type = _Type;
				if(_TypesStr[i].substr(0,type.length()) == type)
					return i;
			}
			
			Log::log(Log::ENGINE_ERROR,("Unknown atom "+toString(_Type)).c_str()); 
			return 0;
		}

		/*float findAtomRadius (void)
		{ 
			switch(_Type[0])
			{
			case 'H': 
				switch(_Type[1])
				{
					case 0: case '.': return 1.487f; break; //Hydrogen
					default: Log::log(Log::ENGINE_ERROR,("Unknown atom "+toString(_Type)).c_str()); return 1.2f;
				}
				break;
			case 'N': 
				switch(_Type[1])
				{
					case 0: case '.': return 1.824f; break; //Nitrogen
					default: Log::log(Log::ENGINE_ERROR,("Unknown atom "+toString(_Type)).c_str()); return 1.2f;
				}
				break;

			case 'C':
				switch(_Type[1])
				{
					case '.': 
						switch(_Type[2])
						{
							case '3': return 1.908f; break;
                            case '2': return 1.908f; break;
							case 'a': return 1.908f; break;
							default: Log::log(Log::ENGINE_ERROR,("Unknown atom "+toString(_Type)).c_str()); return 1.908f;
						}
						break;
					default: Log::log(Log::ENGINE_ERROR,("Unknown atom "+toString(_Type)).c_str()); return 1.2f;
				}
				break;
			case 'O': 
				switch(_Type[1])
				{
					case '.': 
						switch(_Type[2])
						{
							case '3': return 1.721f; break;
							case '2': return 1.6612f; break;
							case 'c': return 1.6612f; break;
							default: Log::log(Log::ENGINE_ERROR,("Unknown atom "+toString(_Type)).c_str()); return 1.2f;
						}
						break;
					default: Log::log(Log::ENGINE_ERROR,("Unknown atom "+toString(_Type)).c_str()); return 1.2f;
				}
				break;
			case 'S':
				switch(_Type[1])
				{
					case 0: case '.': return 2.0f; break;
					default: Log::log(Log::ENGINE_ERROR,("Unknown atom "+toString(_Type)).c_str()); return 1.2f;
				}
				break;
			case 'F':
				switch(_Type[1])
				{
					case 0: case '.': return 1.75f; break;	
					default: Log::log(Log::ENGINE_ERROR,("Unknown atom "+toString(_Type)).c_str()); return 1.2f;
				}
				break;
				case 'L': return 1.137f; break;
				default: Log::log(Log::ENGINE_ERROR,("Unknown atom "+toString(_Type)).c_str()); return 1.2f;
			}

			Log::log(Log::ENGINE_ERROR,("Unknown atom "+toString(_Type)).c_str());  
			return 1.2f;
		}

	
		float findAtomRadiusOld2 (void)
		{ 
			switch(_Type[0])
			{
				case 'H': 
					switch(_Type[1])
					{
						case 0: case '.': return 1.2f; break; //Hydrogen
						case 'G': case 'g': return 1.55f; break; //Mercure
						default: Log::log(Log::ENGINE_ERROR,("Unknown atom "+toString(_Type)).c_str()); return 1.2f;
					}
					break;
				case 'N': 
					switch(_Type[1])
					{
						case 0: case '.': return 1.55f; break; //Nitrogen
						case 'A': case 'a': return 2.27f; break; //Sodium
						default: Log::log(Log::ENGINE_ERROR,("Unknown atom "+toString(_Type)).c_str()); return 1.2f;
					}
					break;

				case 'C':
					switch(_Type[1])
					{
						case 0: case '.': return 1.70f; break; //Carbon
						case 'U': case 'u': return 1.40f; break; //Cuivre
						case 'L': case 'l': return 1.75f; break; //Chlorine
						default: Log::log(Log::ENGINE_ERROR,("Unknown atom "+toString(_Type)).c_str()); return 1.2f;
					}
					break;
				case 'O': return 1.52f; break; //Oxygen
				case 'I': return 1.98f; break; //Iodine
				case 'P': return 1.80f; break; //Phosphorus
				case 'B':
					switch(_Type[1])
					{
						case 0: case'.': return 1.92f; break; //Boron
						case 'R': case 'r': return 1.85f; break; //Bromine
						default: Log::log(Log::ENGINE_ERROR,("Unknown atom "+toString(_Type)).c_str()); return 1.2f;
					}
					break;
				case 'S':
					switch(_Type[1])
					{
						case 0: case '.': return 1.80f; break;
						case 'E': case 'e':return 1.90f; break;
						default: Log::log(Log::ENGINE_ERROR,("Unknown atom "+toString(_Type)).c_str()); return 1.2f;
					}
					break;
				case 'F':
					switch(_Type[1])
					{
						case 0: case '.': return 1.47f; break;
						case 'E': case 'e': return 1.80f; break;
						default: Log::log(Log::ENGINE_ERROR,("Unknown atom "+toString(_Type)).c_str()); return 1.2f;
					}
					break;
				case 'K': return 2.75f; break;
				case 'M':
					switch(_Type[1])
					{
						case 'N': case 'n': return 1.73f; break;
						case 'G': case 'g': return 1.73f; break;
						default: Log::log(Log::ENGINE_ERROR,("Unknown atom "+toString(_Type)).c_str()); return 1.2f;
					}
					break;
				case 'Z': return 1.39f;	break;		
				case 'X': return 2.16f; break;
				case 'A': return 1.84f; break;
				case 'L': return 1.8f; break;
				default: Log::log(Log::ENGINE_ERROR,("Unknown atom "+toString(_Type)).c_str()); return 1.2f;
			}

			Log::log(Log::ENGINE_ERROR,("Unknown atom "+toString(_Type)).c_str()); 
			return 1.2f;
		}

		float findAtomRadiusOld (void)
		{ 
			switch(_Name[0])
			{
			case 'H': 
				switch(_Name[1])
				{
				case 0: return 1.20f;
				case 'G': return 1.8f;
				}
				break;
			case 'N': 
				switch(_Name[1])
				{
				case 0: return 1.55f;
				case 'A': return 2.27f;
				}
				break;

			case 'C':
				switch(_Name[1])
				{
				case 0: return 1.70f;
				case 'U': return 1.40f;
				case 'L': return 1.75f;
				}
				break;
			case 'O': return 1.52f;
			case 'I': return 1.98f;
			case 'P': return 1.80f;
			case 'B':
				switch(_Name[1])
				{
				case 0: return 1.85f;
				case 'R': return 1.85f;
				}
				break;
			case 'S':
				switch(_Name[1])
				{
				case 0: return 1.80f;
				case 'E': return 1.90f;
				}
				break;
			case 'F':
				switch(_Name[1])
				{
				case 0: return 1.47f;
				case 'E': return 1.80f;
				}
				break;
			case 'K': return 2.75f;
			case 'M':
				switch(_Name[1])
				{
				case 'N': return 1.73f;
				case 'G': return 1.73f;
				}
				break;
			case 'Z': return 1.39f;			
			case 'X': return 1.8f;
			case 'A': return 1.8f;
			case 'L': return 1.8f;
			case '.': return 1.8f;
			}

			return 0.0f;
		}*/
};


#endif