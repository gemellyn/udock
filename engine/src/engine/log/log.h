#ifndef	__NY_LOG__
#define	__NY_LOG__

#include <SDL2/SDL.h>

#include <string>
#include <vector>

using namespace std;

class Log
{

	public:

		typedef enum {
			USER_INFO,
			USER_ERROR,
			ENGINE_INFO,
			ENGINE_ERROR
		}MSG_TYPE;

	protected:
		static bool _HideEngineLog;
		static unsigned long _LastMessageTime;
		static vector<Log*> _Logs;
		std::string _LogType;

		virtual void logMsg(MSG_TYPE type, const char *message, bool intro=true, bool ret=true) = 0;
	public:

		//Pour faire une chaine de logs
		static void addLog(Log * log)
		{
			_Logs.push_back(log);
			Log::log(ENGINE_INFO,(log->_LogType+" added").c_str());
		}

		static void removeLog(Log * log)
		{
			std::vector<Log*>::iterator it = _Logs.begin();
			while(it != _Logs.end())
			{
				if(*it == log)
					it = _Logs.erase(it);
                else
                    it++;
			}
			Log::log(ENGINE_INFO,(log->_LogType+" removed").c_str());
		}

		static void log(MSG_TYPE type, const char *message, bool intro=true, bool ret=true)
		{
			if(_Logs.size() == 0)
				return;
			if(_HideEngineLog && (type == ENGINE_ERROR || type == ENGINE_INFO))
				return;

			for(unsigned int i=0;i<_Logs.size();i++)
				_Logs[i]->logMsg(type,message,intro,ret);

			_LastMessageTime = SDL_GetTicks();
		}

		static void showEngineLog(bool show)
		{
			_HideEngineLog = !show;
		}

};

#endif
