#ifndef __LOGIN_STATE__
#define __LOGIN_STATE__

#include "game_states/choose_mol_state.h"
#include "engine/gui/screen_manager.h"
#include "engine/gui/loading.h"
#include "engine/network/http_manager.h"
#include "session/player_info.h"
#include "network/http_job_login.h"
#include "network/http_job_upload_log.h"


class LoginState : public GameState
{
	public:

		static LoginState * _LastInstance; //Lazy Singleton

		GUIEdtBox * _EdtLogin;
		GUIEdtBox * _EdtPass;
		GUIBouton * _BtnLogin;
		GUILabel * _LblVersion;
		GUILabel* _LblBestPlayer;
		GUIScreenManager * _ScreenManager;
		GUIScreen * _Screen;
		HttpManagerUdock * _HttpManager;
		ChooseMolState * _ChooseMolState;	
		HttpJobLogin * _JobLogin; 
		HttpJobUploadLog * _JobUploadLog; 
		GUILoading * _Loading;
		
		LoginState() : GameState()
		{
			_ScreenManager = NULL;
			_Screen = NULL;
			_LastInstance = this;
			_HttpManager = HttpManagerUdock::getInstance();
			_JobLogin = new HttpJobLogin();
			_JobUploadLog = new HttpJobUploadLog();
		}

		static void onClickLogin(GUIBouton * bouton)
		{
			if(!_LastInstance->_JobLogin->_JobRunning)
			{
				_LastInstance->_JobLogin->_Login = _LastInstance->_EdtLogin->Text;
				_LastInstance->_JobLogin->_Pass = _LastInstance->_EdtPass->Text;
				_LastInstance->_JobLogin->launchJob();
				_LastInstance->_Loading->Visible = true;
			}
		}

		bool activate(bool active)
		{
			GameState::activate(active);
			
			if(active)
			{
				_ScreenManager->setActiveScreen(_Screen);

				_BtnLogin->setOnClick(onClickLogin);
				_EdtLogin->Text = "";
				_EdtPass->Text = "";

				//On get le best player
				HttpManagerUdock::getInstance()->getBestPlayerMessage(_LblBestPlayer->Text);
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

						PlayerInfo::getInstance()->_Login = _JobLogin->_Login;
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