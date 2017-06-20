#ifndef	__NY_LOG_FILE__
#define	__NY_LOG_FILE__

#include "engine/utils/types.h"
#include "engine/utils/ny_utils.h"
#include "engine/log/log_console.h"
#include "log.h"
#include <cstdio>
#include <string>
#include <vector>

#ifdef MSVC
#include <direct.h>
#else
#include <unistd.h>
#endif

using namespace std;

/** Interface pour les composants de gestion bas niveau du réseau
	*/
class LogFile : public Log
{
	private:
		string _File;

	public:
		LogFile() : Log()
		{
			_File = "log.txt";
			FILE * fs = fopen(_File.c_str(),"w");
			if(fs)
			   fclose(fs);

            _LogType = "LogFile";

			//On redirige stderr et stdout vers du fichier
			//freopen( "stdout.log", "w", stdout );
			//freopen( "stderr.log", "w", stderr );
			//_cprintf("Cwd : %s\n",_getcwd(NULL,0));

		}

	private:

		void logMsg(MSG_TYPE type, const char *message, bool intro=true, bool ret=true)
		{
			FILE * fs = fopen(_File.c_str(),"a");

			if(!fs)
			{
                Log::removeLog(this);

				#ifdef MSVC
				char* buffer = _getcwd(NULL,0);
				#else
				char* buffer = getcwd(NULL,0);
				#endif
			    Log::log(ENGINE_ERROR,("Unable to open log file " + _File + " in " + toString(buffer) + ", removing file log (lost msg following)").c_str(),true,false);
				Log::log(type,message,intro,ret);
				return;
			}

			if(intro)
			{
				switch(type)
				{
					case ENGINE_ERROR : fprintf(fs,"ENGINE_ERROR "); break;
					case ENGINE_INFO : fprintf(fs,"ENGINE_INFO "); break;
					case USER_ERROR : fprintf(fs,"USER_ERROR "); break;
					case USER_INFO : fprintf(fs,"USER_INFO "); break;
					default: fprintf(fs,"UNKN_TYPE: "); break;
				}
				fprintf(fs,"[%06ld] ",SDL_GetTicks()-_LastMessageTime);
			}

			fprintf(fs,"%s",message);

			if(ret)
				fprintf(fs,"\n");

			fclose(fs);
		}






};

#endif
