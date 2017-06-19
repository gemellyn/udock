#ifndef __PDB_LOADING_H__
#define __PDB_LOADING_H__

#include "stdio.h"
#include "engine/utils/ny_utils.h"
#include "engine/utils/types_3d.h"

class PdbAtom
{	
	public :
		int _SerialNumber;
		NYVert3Df _Position;
		float _Charge;
		char _Name[5];
		char _Type[7];
		int _SubstId;
		char _SubstName[5];

	public :
		PdbAtom()
		{
			_SerialNumber = 0;
			_Charge = 0;
			_SubstName[4] = 0;
			_Type[6] = 0;
			_Name[4] = 0;
		}

		void setSerial(int serial)
		{
			_SerialNumber = serial;
		}
	
		void setPosition(NYVert3Df pos)
		{
			_Position = pos;
		}
		
		void setCharge(float charge)
		{
			_Charge = charge;
		}
		
		void setName(char * name)
		{
			strncpy(_Name,name,3);
			_Name[3] = 0;
		}

		void setType(char * type)
		{
			strncpy(_Type,type,5);
			_Type[5] = 0;
		}

		void setSubstId(int id)
		{
			_SubstId = id;
		}

		void setSubstName(char * subst)
		{
			strncpy(_SubstName,subst,3);
			_SubstName[3] = 0;
		}

		//Pas de alternate location indicator
		//Pas de code d'insertion des résidus
		void saveToFile(FILE * fs, int numberInFile)
		{
			int chainIdentifier = 'A' + numberInFile;
			          //ATOM 12345 nnnn rrr issss     xxxxxxxx yyyyyyyy zzzzzzzz  
			fprintf(fs,"ATOM  % 5d %c%c%c%c %c%c%c %c% 4d    % 8.3f% 8.3f% 8.3f\n",_SerialNumber,
												_Name[0],
												_Name[1] == 0 ? ' ' : _Name[1],
												_Name[2] == 0 ? ' ' : _Name[2],
												_Name[3] == 0 ? ' ' : _Name[3],
				                               _SubstName[0],
											   _SubstName[1] == 0 ? ' ' : _SubstName[1],
											   _SubstName[2] == 0 ? ' ' : _SubstName[2],
											   chainIdentifier,
											   _SubstId,
				                               _Position.X,_Position.Y,_Position.Z);
		}
};

class PdbLoader
{
	public :
		
		/**
		* SAUVEGARDE
		*/
		static void writeEntete(FILE * fs, const char * molName, int nbAtoms, int nbBonds, const char * molType, const char * chargeType )
		{
				
		}

		static void writeRemark(FILE * fs, int numRemark, const char * remark)
		{
			fprintf(fs,"REMARK %03d %s\n",numRemark,remark);
		}


		static void writeJunction(FILE * fs, PdbAtom & lastAtom, int numberInFile)
		{
			int chainIdentifier = 'A' + numberInFile;
			fprintf(fs,"TER   % 5d      %c%c%c %c% 4d\n",lastAtom._SerialNumber+1, 
				lastAtom._SubstName[0],
				lastAtom._SubstName[1] == 0 ? ' ' : lastAtom._SubstName[1],
				lastAtom._SubstName[2] == 0 ? ' ' : lastAtom._SubstName[2],
				chainIdentifier,
				lastAtom._SubstId);
		}
};




#endif