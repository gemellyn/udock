#ifndef	__NY_LOG_CONSOLE__
#define	__NY_LOG_CONSOLE__

#include "engine/utils/types.h"
#include "engine/utils/ny_utils.h"
#include "log.h"
#include <string>
#include <vector>

#ifdef MSVC
#include <conio.h>
#include <windows.h>
#endif

using namespace std;

class LogConsole : public Log
{
	public:

		LogConsole() : Log()
		{
            #ifdef MSVC
			AllocConsole();
			freopen("CON","w",stdout);
			freopen("CON","w",stderr);
            #endif
            _LogType = "LogConsole";
		}

		static void reopenConsole()
		{
			#ifdef MSVC
			FreeConsole();
			AllocConsole();
			freopen("CON", "w", stdout);
			freopen("CON", "w", stderr);
			#endif
		}
	private:

		void logMsg(MSG_TYPE type, const char *message, bool intro=true, bool ret=true)
		{
            #ifdef MSVC
			switch(type)
			{
				case ENGINE_ERROR : SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12); break;
				case ENGINE_INFO : SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7); break;
				case USER_ERROR : SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12); break;
				case USER_INFO : SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15); break;
				default: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7); break;
			}
			#endif

			if(intro)
			{
				switch(type)
				{
					case ENGINE_ERROR : fprintf(stdout,"ENG_ERROR "); break;
					case ENGINE_INFO : fprintf(stdout,"ENG_INFO "); break;
					case USER_ERROR : fprintf(stdout,"USR_ERROR "); break;
					case USER_INFO : fprintf(stdout,"USR_INFO "); break;
					default: fprintf(stdout,"UNK_TYP: "); break;
				}
				fprintf(stdout,"[%06ld] ",SDL_GetTicks()-_LastMessageTime);
			}

			fprintf(stdout,"%s",message);

			if(ret)
				fprintf(stdout,"\n");
		}





};

#endif
