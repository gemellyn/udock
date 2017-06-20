#ifndef __LOG_SCREEN_H__
#define __LOG_SCREEN_H__

#include "global.h"
#include "engine/log/log.h"
#include "engine/gui/label.h"

class LogScreen : public Log
{
	public:
		LogScreen() : Log()
		{
			_LabelLog = NULL;
			_LogType = "LogScreen";
		}

		static void setLabel(GUILabel * label)
		{
			_LabelLog = label;
		}

		static GUILabel * getLabel(void)
		{
			return _LabelLog;
		}

	private :

		static GUILabel * _LabelLog;

		void logMsg(MSG_TYPE type, const char *message, bool intro=true, bool ret=true)
		{
			if(_LabelLog)
				_LabelLog->Text = message;
		}
};

#endif
