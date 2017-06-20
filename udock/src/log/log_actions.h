#ifndef __LOG_ACTIONS_H__
#define __LOG_ACTIONS_H__

#include <string>
#include <stdio.h>
#include "engine/utils/types_3d.h"
#include "engine/network/http_manager.h"

#include "global.h"

class LogActions
{
	private:
		static LogActions * _Instance;
		std::string _FileName;
		int _UserId;
		FILE * _Fs;
		bool _Opened;

		LogActions()
		{
			_Opened = false;
		}

		bool openFile(void)
		{
			if(!_Opened)
				return false;

			_Fs = fopen(_FileName.c_str(),"ab");
			if(_Fs != NULL)
				return true;
			return false;
		}

		void closeFile(void)
		{
			if(_Fs != NULL)
			{
				fclose(_Fs);
				_Fs = NULL;
			}
		}

		void writeEvtEntete(int evtNum)
		{
			fwrite(&evtNum,sizeof(int),1,_Fs);
			uint32 time = SDL_GetTicks();
			fwrite(&time,sizeof(uint32),1,_Fs);
			fwrite(&(PlayerInfo::getInstance()->_IdUser),sizeof(int),1,_Fs);
		}

		static void makeFileNameFromUserId(int userId, std::string & name)
		{
			static int maquillage = 0xAc94dbf2; //au pif
			//On "maquille" le numero de log
			int idMaquille = userId ^ maquillage;

			//On en fait un nom de fichier
			char buffer[100];
			sprintf(buffer,"%x.bin",idMaquille);

			name = std::string(buffer);
		}



	public:

		static LogActions * getInstance(void)
		{
			if(_Instance == NULL)
				_Instance = new LogActions();
			return _Instance;
		}

		static int getUserIdFromFileName(std::string & name)
		{
			static int maquillage = 0xAc94dbf2; //au pif

			int idMaquille;

			//On en fait un nom de fichier
			sscanf(name.c_str(),"%x.bin",&idMaquille);

			return idMaquille ^ maquillage;

		}

		void openLog(int userId)
		{
			makeFileNameFromUserId(userId,_FileName);
			_UserId = userId;
			_Opened = true;
			this->logUdockVersion();
		}

		void getFileName(std::string & filename)
		{
			filename = _FileName;
		}

		bool logFileExists(void)
		{
			if(_Opened)
			{
				FILE * fe = fopen(_FileName.c_str(),"r");
				if(fe)
				{
					fclose(fe);
					return true;
				}
			}
			return false;
		}

		void eraseLog(void)
		{
			_Fs = fopen(_FileName.c_str(),"wb");
			closeFile();
		}

		void logStartDocking(int idMol1, int idMol2)
		{
			if(openFile())
			{
				writeEvtEntete(1);
				fwrite(&idMol1,sizeof(int),1,_Fs);
				fwrite(&idMol2,sizeof(int),1,_Fs);
				closeFile();
			}
		}

		void logComplex(int idMol1, int idMol2, Positionator Mol1Pos, Positionator Mol2Pos)
		{
			if(openFile())
			{
				writeEvtEntete(2);
				fwrite(&idMol1,sizeof(int),1,_Fs);
				fwrite(&idMol2,sizeof(int),1,_Fs);
				fwrite(&(Mol1Pos._Pos.X),sizeof(float),1,_Fs);
				fwrite(&(Mol1Pos._Pos.Y),sizeof(float),1,_Fs);
				fwrite(&(Mol1Pos._Pos.Z),sizeof(float),1,_Fs);
				float euler[3];
				Mol1Pos._Rot.toEulerXYZ(euler);
				fwrite(&(euler[0]),sizeof(float),1,_Fs);
				fwrite(&(euler[1]),sizeof(float),1,_Fs);
				fwrite(&(euler[2]),sizeof(float),1,_Fs);

				fwrite(&(Mol2Pos._Pos.X),sizeof(float),1,_Fs);
				fwrite(&(Mol2Pos._Pos.Y),sizeof(float),1,_Fs);
				fwrite(&(Mol2Pos._Pos.Z),sizeof(float),1,_Fs);
				Mol2Pos._Rot.toEulerXYZ(euler);
				fwrite(&(euler[0]),sizeof(float),1,_Fs);
				fwrite(&(euler[1]),sizeof(float),1,_Fs);
				fwrite(&(euler[2]),sizeof(float),1,_Fs);

				closeFile();
			}
		}

		void logSetHarpoonHandle(int idMol, NYVert3Df localPos)
		{
			if(openFile())
			{
				writeEvtEntete(3);
				fwrite(&idMol,sizeof(int),1,_Fs);
				fwrite(&(localPos.X),sizeof(float),1,_Fs);
				fwrite(&(localPos.Y),sizeof(float),1,_Fs);
				fwrite(&(localPos.Z),sizeof(float),1,_Fs);
				closeFile();
			}
		}

		void logScore(float score)
		{
			if(openFile())
			{
				writeEvtEntete(4);
				fwrite(&score,sizeof(float),1,_Fs);
				closeFile();
			}
		}

		void logUseOptim(int idMol1, int idMol2, Positionator Mol1Pos, Positionator Mol2Pos)
		{
			if(openFile())
			{
				writeEvtEntete(5);
				fwrite(&idMol1,sizeof(int),1,_Fs);
				fwrite(&idMol2,sizeof(int),1,_Fs);
				fwrite(&(Mol1Pos._Pos.X),sizeof(float),1,_Fs);
				fwrite(&(Mol1Pos._Pos.Y),sizeof(float),1,_Fs);
				fwrite(&(Mol1Pos._Pos.Z),sizeof(float),1,_Fs);
				float euler[3];
				Mol1Pos._Rot.toEulerXYZ(euler);
				fwrite(&(euler[0]),sizeof(float),1,_Fs);
				fwrite(&(euler[1]),sizeof(float),1,_Fs);
				fwrite(&(euler[2]),sizeof(float),1,_Fs);

				fwrite(&(Mol2Pos._Pos.X),sizeof(float),1,_Fs);
				fwrite(&(Mol2Pos._Pos.Y),sizeof(float),1,_Fs);
				fwrite(&(Mol2Pos._Pos.Z),sizeof(float),1,_Fs);
				Mol2Pos._Rot.toEulerXYZ(euler);
				fwrite(&(euler[0]),sizeof(float),1,_Fs);
				fwrite(&(euler[1]),sizeof(float),1,_Fs);
				fwrite(&(euler[2]),sizeof(float),1,_Fs);
				closeFile();
			}
		}

		void logUdockVersion(void)
		{
			if(openFile())
			{
				writeEvtEntete(6);
				int overall = UDOCK_OVERALL_VERSION;
				fwrite(&(overall),sizeof(int),1,_Fs);
				int major = UDOCK_MAJOR_VERSION;
				fwrite(&(major),sizeof(int),1,_Fs);
				int minor = UDOCK_MINOR_VERSION;
				fwrite(&(minor),sizeof(int),1,_Fs);
				closeFile();
			}
		}

		void logStartFlying(void)
		{
			if(openFile())
			{
				writeEvtEntete(7);
				closeFile();
			}
		}





};

#endif
