#ifndef __MOL2_LOADING_H__
#define __MOL2_LOADING_H__

#include "stdio.h"
#include "engine/utils/ny_utils.h"
#include "engine/utils/types_3d.h"

class Mol2Loader
{
	public :
		#define MOL2_LINE_SIZE 200
		static char g_current_mol2_line[MOL2_LINE_SIZE]; 

		/**
		  * Lit une ligne du fichier dans le buffer
		  */
		static inline int readLine(FILE * fe)
		{
			memset(g_current_mol2_line,0x00,MOL2_LINE_SIZE);
			if(fgets(g_current_mol2_line,MOL2_LINE_SIZE,fe) == NULL)
				return 1;

			if(feof(fe))
				return 2;

			if(g_current_mol2_line[0] == '@')
				return 3;
			return 0;
		}

		static inline char * toFirstCar(char * line)
		{
			char * ptLine = line;
			for(int i=0; line[i] == ' ' && line[i] != 0x00; i++)
				ptLine++;
			return ptLine;
		}

		static inline char * toFirstNewLineChar(char * line)
		{
			char * ptLine = line;
			for(int i=0; line[i] != '\n' && line[i] != 0x00; i++)
				ptLine++;
			return ptLine;
		}

		static void stopAtFirstNewLineChar(char * line)
		{
			char * ptLine = line;
			for(int i=0; line[i] != '\n' && line[i] != 0x00; i++)
				ptLine++;
			*ptLine = 0;
		}

		static inline char * toFirstSpace(char * line)
		{
			char * ptLine = line;
			for(int i=0; line[i] != ' ' && line[i] != 0x00; i++)
				ptLine++;
			return ptLine;
		}

		static int findAtomsFromStart(FILE * fe)
		{
			memset(g_current_mol2_line,0x00,MOL2_LINE_SIZE);
			int ret = 0;
			while(strncmp(g_current_mol2_line,"@<TRIPOS>ATOM",13) != 0 && ret != 2 && ret != 1)
				 ret = readLine(fe);
			return ret;
		}

		static int findMoleculeFromStart(FILE * fe)
		{
			memset(g_current_mol2_line,0x00,MOL2_LINE_SIZE);
			int ret = 0;
			while(strncmp(g_current_mol2_line,"@<TRIPOS>MOLECULE",17) != 0 && ret != 2 && ret != 1)
				 ret = readLine(fe);
			return ret;
		}

		static int findBondsFromStart(FILE * fe)
		{
			memset(g_current_mol2_line,0x00,MOL2_LINE_SIZE);
			int ret = 0;
			while(strncmp(g_current_mol2_line,"@<TRIPOS>BOND",13) != 0 && ret != 2 && ret != 1)
				 ret = readLine(fe);
			return ret;
		}

		static int parseEntete(FILE * fe, std::string & molName, std::string & molType, std::string & chargeType, int * nbBonds )
		{
			int ret = findMoleculeFromStart(fe);
			if(ret == 3)
			{
				int useless;
				readLine(fe);
				char * line = Mol2Loader::toFirstCar(g_current_mol2_line);
				stopAtFirstNewLineChar(line);
				molName = line;
				readLine(fe);
				sscanf (g_current_mol2_line,"%d %d",&useless,nbBonds);
				readLine(fe);
				line = Mol2Loader::toFirstCar(g_current_mol2_line);
				stopAtFirstNewLineChar(line);
				molType = line;
				readLine(fe);
				line = Mol2Loader::toFirstCar(g_current_mol2_line);
				stopAtFirstNewLineChar(line);
				chargeType = line;

				return 0;
			}
			return ret;
		}


		/**
		* SAUVEGARDE
		*/
		static void writeEntete(FILE * fs, const char * molName, int nbAtoms, int nbBonds, const char * molType, const char * chargeType )
		{
			fprintf(fs,"@<TRIPOS>MOLECULE\n");
			fprintf(fs,"%s\n",molName);
			fprintf(fs,"%d %d\n",nbAtoms,nbBonds);
			fprintf(fs,"%s\n",molType);
			fprintf(fs,"%s\n",chargeType);
			fprintf(fs,"\n");

			fprintf(fs,"@<TRIPOS>ATOM\n");
		}

		//On ouvre le fichier de la molécule, on y cherche les liens et on les recopie dans le nouveau fichier
		static void writeBonds(FILE * fs, FILE * fe)
		{
			int ret = findBondsFromStart(fe);
			if(ret == 3)
			{
				fprintf(fs,"@<TRIPOS>BOND\n");

				while(readLine(fe) == 0)
				{
					fputs(g_current_mol2_line,fs);
				}
			}
		}

};

class Mol2Atom
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
		Mol2Atom()
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


		void buildFromLine(char * line)
		{		
			//On avance jusqu'au serial number
			line = Mol2Loader::toFirstCar(line);
			sscanf(line,"%d",&_SerialNumber);
			line = Mol2Loader::toFirstSpace(line);
			line = Mol2Loader::toFirstCar(line);
			sscanf(line,"%s",_Name);
			line = Mol2Loader::toFirstSpace(line);
			line = Mol2Loader::toFirstCar(line);
			sscanf(line,"%f",&(_Position.X));
			line = Mol2Loader::toFirstSpace(line);
			line = Mol2Loader::toFirstCar(line);
			sscanf(line,"%f",&(_Position.Y));
			line = Mol2Loader::toFirstSpace(line);
			line = Mol2Loader::toFirstCar(line);
			sscanf(line,"%f",&(_Position.Z));
			line = Mol2Loader::toFirstSpace(line);
			line = Mol2Loader::toFirstCar(line);
			sscanf(line,"%s",_Type);
			line = Mol2Loader::toFirstSpace(line);
			line = Mol2Loader::toFirstCar(line);
			sscanf(line,"%d",&_SubstId);
			line = Mol2Loader::toFirstSpace(line);
			line = Mol2Loader::toFirstCar(line);
			sscanf(line,"%s",_SubstName);
			line = Mol2Loader::toFirstSpace(line);
			line = Mol2Loader::toFirstCar(line);
			sscanf(line,"%f",&_Charge);
		}

		void saveToFile(FILE * fs)
		{
			fprintf(fs,"%d %s %f %f %f %s %d %s %f\n",_SerialNumber,_Name,_Position.X,_Position.Y,_Position.Z, _Type,_SubstId,_SubstName,_Charge);
		}
};




#endif