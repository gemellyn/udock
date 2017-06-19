#ifndef __PLAYER_INFO_H__
#define __PLAYER_INFO_H__

#include "global.h"
#include <string>

class PlayerInfo
{
	private:
		PlayerInfo()
		{
			_IdUser = -1;
			_ShowFeedBacks = true;
		}
		
		static PlayerInfo * _Instance;

	public:

		int _IdUser;
		std::string _Login;
		bool _ShowFeedBacks;

		static PlayerInfo * getInstance(void)
		{
			if(_Instance == NULL)
				_Instance = new PlayerInfo();
			return _Instance;
		}
};

#endif
