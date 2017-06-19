
#ifndef __LOGIN_AUTO_STATE__
#define __LOGIN_AUTO_STATE__

#include "game_states/login_state.h"

#define AUTO_LOGIN "autologin"

class LoginAutoState : public LoginState
{
	public:

		LoginAutoState() : LoginState()
		{

		}

		
		bool activate(bool active)
		{
			GameState::activate(active);
			
			if(active)
			{
				_ScreenManager->setActiveScreen(_Screen);

				_BtnLogin->Visible = false;
				_EdtLogin->Visible = false;
				_EdtLogin->Text = "Anonymous";
				_EdtPass->Visible = false;

				if(!_LastInstance->_JobLogin->_JobRunning)
				{
					_LastInstance->_JobLogin->_Login = AUTO_LOGIN;
					_LastInstance->_JobLogin->_Pass = "";
					_LastInstance->_JobLogin->launchJob();
					_LastInstance->_Loading->Visible = true;
				}
			}
			
			return active;
		}

		void preState(float elapsed)
		{
			if(_JobLogin->_JobRunning)
			{ 
				if(_JobLogin->_JobDone)
				{
					_JobLogin->_JobRunning = false;
					_JobLogin->_JobDone = false;
					_Loading->Visible = false;

					PlayerInfo::getInstance()->_IdUser = _JobLogin->_IdUser;
					if(PlayerInfo::getInstance()->_IdUser > 0)
					{
						Log::log(Log::USER_INFO,"Login ok");

						PlayerInfo::getInstance()->_Login = _LastInstance->_EdtLogin->Text;
						LogActions::getInstance()->openLog(PlayerInfo::getInstance()->_IdUser);

						if(LogActions::getInstance()->logFileExists())
						{
							_JobUploadLog->_UserId = PlayerInfo::getInstance()->_IdUser;
							LogActions::getInstance()->getFileName(_JobUploadLog->_FileName);
							_JobUploadLog->launchJob();
							_Loading->Visible = true;
						}
					}

					if(PlayerInfo::getInstance()->_IdUser < 0)
					{
						_LblVersion->Text = "You need to download the last version to play";
						_LblVersion->ColorBorder = NYColor(1.0f,0.85f,0.85f,1.0f);
					}

					if(PlayerInfo::getInstance()->_IdUser == 0)
					{
						_LblVersion->Text = "Login error. Did you create an account ?";
						_LblVersion->ColorBorder = NYColor(1.0f,0.85f,0.85f,1.0f);
					}
				}
			}

			if(_JobUploadLog->_JobRunning)
			{ 
				if(_JobUploadLog->_JobDone)
				{
					_JobUploadLog->_JobRunning = false;
					_JobUploadLog->_JobDone = false;

					if(_JobUploadLog->_UploadOk)
						LogActions::getInstance()->eraseLog();

					_LastInstance->activate(false);
					_LastInstance->_ChooseMolState->activate(true);	
					_Loading->Visible = false;
				}
			}

			
		}
		

};

#endif
