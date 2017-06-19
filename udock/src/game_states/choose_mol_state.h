#ifndef __CHOOSE_MOL_STATE__
#define __CHOOSE_MOL_STATE__

#include "game_states/game_state.h"
#include "game_states/load_state.h"

#include "engine/gui/screen_manager.h"
#include "network/http_manager_udock.h"

class ChooseMolState : public GameState
{


	public:

		static ChooseMolState * _LastInstance; //Lazy Singleton

		GUIComboBox * _CbMol1;
		GUIComboBox * _CbMol2;
		GUIBouton * _BoutonVal;
		LoadState * _LoadState;
		GUIScreenManager * _ScreenManager;
		GUIScreen * _Screen;
		GUILstBox * _LstHighScores;

		NYTexFile * _TexSelect;

		std::vector<HttpManagerUdock::HttpMolStruct> _Mols; //Recupere par http

		ChooseMolState() : GameState()
		{
			_ScreenManager = NULL;
			_Screen = NULL;
			_LastInstance = this;
			_TexSelect = NYTexManager::getInstance()->loadTexture(string("textures/select.png"));
		}

		static void onClickVal(GUIBouton * bouton)
		{
			int selMol1 = ChooseMolState::_LastInstance->_CbMol1->getSelIndex();
			int selMol2 = ChooseMolState::_LastInstance->_CbMol2->getSelIndex();

			if(selMol1 >= 0 && selMol2 >= 0)
			{
				ChooseMolState::_LastInstance->_LoadState->setToLoad(ChooseMolState::_LastInstance->_Mols[selMol1].id,
																	 ChooseMolState::_LastInstance->_Mols[selMol2].id,
																	 ChooseMolState::_LastInstance->_Mols[selMol1].filename,
																	 ChooseMolState::_LastInstance->_Mols[selMol2].filename);
				ChooseMolState::_LastInstance->activate(false);
				ChooseMolState::_LastInstance->_LoadState->activate(true);
			}
		}

		void render2D(void)
		{
			NYTexManager::getInstance()->drawTex2D(_Renderer->_ScreenWidth/2.0f-_TexSelect->SizeX/2.0f,_Renderer->_ScreenHeight/2.0f-_TexSelect->SizeY/2 - 60,1,1,0,0,*_TexSelect);
		}

		bool activate(bool active)
		{
			GameState::activate(active);

			if(active)
			{
				_ScreenManager->setActiveScreen(_Screen);

				_BoutonVal->setOnClick(onClickVal);

				//On affiche les highscores
				std::vector<std::string> ldb;
				int posUser;
				HttpManagerUdock::getInstance()->getLeaderboard(&ldb,PlayerInfo::getInstance()->_IdUser, &posUser);
				_LstHighScores->clear();
				for(unsigned int i=0;i<ldb.size();i++)
					_LstHighScores->addElement(ldb[i]);

				_LstHighScores->setMaxElementsFromHeight(_Renderer->_ScreenHeight);
				_LstHighScores->Y = 0;
				_LstHighScores->setSelIndex(posUser);


				//On reset
				_CbMol1->LstBox->clear();
				_CbMol2->LstBox->clear();

				//On recup les elements en ligne
				_Mols.clear();
				HttpManagerUdock::getInstance()->getMolList(&_Mols);

				//On checke si elles existent
				bool manqueFichier = false;
				std::vector<HttpManagerUdock::HttpMolStruct>::iterator it = _Mols.begin();
				while(it != _Mols.end())
				{
                    //TODONY was _wfopen : is it working ?
                    std::string filename = wstring_to_utf8(it->filename);
					FILE * fe = fopen(filename.c_str(),"r");
					if(fe)
					{
						fclose(fe);
						it++;
					}
					else
					{
						manqueFichier = true;
						it = _Mols.erase(it);
					}
				}

				//On le dit
				//if(manqueFichier)
					//Log::log(Log::USER_INFO,"You do not have all available molecules : download last udock version !");

				for(unsigned int i=0;i<_Mols.size();i++)
				{
					_CbMol1->LstBox->addElement(_Mols[i].name);
					_CbMol2->LstBox->addElement(_Mols[i].name);
				}
			}

			return active;
		}


		void preState(float elapsed)
		{

		}


};

#endif
