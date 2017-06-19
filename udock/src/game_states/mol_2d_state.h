#ifndef __MOL_2D_STATE__
#define __MOL_2D_STATE__

#include "game_states/game_state.h"
#include "molecules/molecules_manager.h"
#include "engine/sound/sound.h"
#include "engine/render/camera_animated.h"
#include "engine/gui/progressbar.h"
#include "log/log_actions.h"
#include "global.h"
#include "engine/render/graph/tex_manager.h"
#include "network/http_job_post_scores.h"
#include "network/http_job_add_play_time.h"
#include "tutorial/tutorial.h"

class Mol2DState : public GameState
{
	public:

		GUIScreenManager * _ScreenManager;
		GUIScreen * _Screen;
		GUILabel * _LblEnergie;
		GUILabel * _LblBestEnergie;
		GUILabel * _LblBestLogin;
		GUIPBar * _PbarScore;
		GUIPBar * _PbarCharge;
		GUIPBar * _PbarBestScore;
		GUIPBar * _PbarOptim;
		GUIBouton * _BtnFreeze;
		MoleculesManager * _MoleculesManager;
		Molecule *  _MoleculesToMatch[2];
		Molecule * _SelectedMol;
		float _EvalElapsed;
		bool _Force;
		bool _RoateMol;
		bool _RotateCam;

		NYVert3Df _RotatePoint;
		NYVert3Df _PosNowNearMol;
		NYVert3Df _RotatePointWorld;

		SoundBasic ** _SoundsAttach;
		int _NbSoundAttach;
		SoundBasic * _SoundDetach;
		SoundBasic * _SoundHighScore;
		SoundBasic * _SoundReset;

		ParticlesEmitter * HighScoreEffect;

		///< Utile pous savoir si on sauve en local la pos des mols pour la recharger ensuite sur F7
		float _EnergySavePosMolLocal;

		//Best energy perso
		float _CurrentEnergy;
		float _EnergyPersoBest;
		Positionator _Pos1BestEnergy;
		Positionator _Pos2BestEnergy;
		bool _SavePersoBest;
		HttpJobPostScores * _JobPostScore;
		HttpJobAddPlayTime * _JobAddPlayTime;

		//Best energy global
		float _EnergyHighestScore;
		std::string _LoginBestScore;
		std::vector<HttpManagerUdock::HighScoreServer> _HighScores;

		bool _OptimizeAfterReload;
		float _DureeTotaleOptim;
		float _DureeRestanteOptim;
		float _DureeStepOptim;

		//Pour calcul a chaque optim
		float _DureeTotaleLastOptim;
		int _NbStepsTotalLastOptim;
		int _NbAtomsCouplesCalcLastOptim;
		Positionator _Pos1LastOptim;
		Positionator _Pos2LastOptim;

		NYTexFile * _TexPersoBestScore;
		NYTexFile * _TexNiceScore;
		NYTexFile * _TexBestHighScore;
		NYTexFile * _TexOptimizing;
		GUILabel * _LblBeatedNames;
		float _TexScoreTime;

		//Pour calcul du temps de jeu
		float _PlayTime;

		MolState() : GameState()
		{
			_EvalElapsed = 0.0f;
			_EnergyPersoBest = 0.0f;
			_EnergySavePosMolLocal = 0.0f;
			_Force = false;
			_RoateMol = false;
			_NbSoundAttach = 0;
			_SoundsAttach = NULL;
			_SoundDetach = NULL;
			_SoundHighScore = NULL;
			_RotateCam = false;
			_SelectedMol = NULL;
			_MoleculesToMatch[0] = NULL;
			_MoleculesToMatch[1] = NULL;
			_ScreenManager = NULL;
			_Screen = NULL;
			_PbarScore = NULL;
			_EnergyHighestScore = 0.0f;
			HighScoreEffect = new ParticlesEmitter();
			HighScoreEffect->_Speed = 200.0f;
			HighScoreEffect->_SpeedLossPerSec = HighScoreEffect->_Speed * 2.0f;
			HighScoreEffect->_StartSize = 4.0f;
			HighScoreEffect->_SizeLossPerSec = HighScoreEffect->_StartSize * 2.0f;
			HighScoreEffect->_Transform._Pos = NYVert3Df(0,0,0);
			HighScoreEffect->_RandomColor = true;
			_JobPostScore = new HttpJobPostScores();
			_JobAddPlayTime = new HttpJobAddPlayTime();

			_OptimizeAfterReload = false;
			_DureeTotaleOptim = 0;
			_DureeRestanteOptim = 0;
			_DureeStepOptim = 0;
			_TexScoreTime = 0;
			_SavePersoBest = false;
			_PlayTime = 0.0f;
		}

		void setTargetScore(float target)
		{
			_EnergyHighestScore = target;
		}

		void render(void)
		{
			if(PlayerInfo::getInstance()->_ShowFeedBacks)
				HighScoreEffect->render();
		}

		void render2D(void)
		{
			if(_PbarOptim->Visible)
				NYTexManager::getInstance()->drawTex2D(_Renderer->_ScreenWidth/2.0f-_TexOptimizing->SizeX/2.0f,_Renderer->_ScreenHeight/2.0f-_TexOptimizing->SizeY/2.0f,1,1,0,0,*_TexOptimizing);

			_LblBeatedNames->Visible = false;
			if(_TexScoreTime > 0.0f)
			{
				if(_TexNiceScore->Visible)
					NYTexManager::getInstance()->drawTex2D(_Renderer->_ScreenWidth/2.0f-_TexNiceScore->SizeX/2.0f,_Renderer->_ScreenHeight/2.0f-_TexNiceScore->SizeY/2,1,1,0,0,*_TexNiceScore);
				if(_TexPersoBestScore->Visible)
    				NYTexManager::getInstance()->drawTex2D(_Renderer->_ScreenWidth/2.0f-_TexPersoBestScore->SizeX/2.0f,_Renderer->_ScreenHeight/2.0f-_TexPersoBestScore->SizeY/2,1,1,0,0,*_TexPersoBestScore);
				if(_TexBestHighScore->Visible)
					NYTexManager::getInstance()->drawTex2D(_Renderer->_ScreenWidth/2.0f-_TexBestHighScore->SizeX/2.0f,_Renderer->_ScreenHeight/2.0f-_TexBestHighScore->SizeY/2,1,1,0,0,*_TexBestHighScore);
				if(_LblBeatedNames->Text.size() > 0)
					_LblBeatedNames->Visible = true;
			}

			Tutorial::getInstance()->render2D();
		}


		void init(void)
		{
			GameState::init();
			_TexOptimizing = NYTexManager::getInstance()->loadTexture(string("textures/optim.png"));
			_TexNiceScore = NYTexManager::getInstance()->loadTexture(string("textures/nicescore.png"));
			_TexPersoBestScore = NYTexManager::getInstance()->loadTexture(string("textures/persobest.png"));
			_TexBestHighScore = NYTexManager::getInstance()->loadTexture(string("textures/highscore.png"));
		}

		void addToGui(uint16 & x, uint16 & y,GUIScreen * screen)
		{
			_LblEnergie = new GUILabel();
			_LblEnergie->Text = "Energie";
			_LblEnergie->X = x;
			_LblEnergie->Y = y;
			_LblEnergie->Visible = true;
			screen->addElement(_LblEnergie);

			y += _LblEnergie->Height + 1;

			_LblBestEnergie = new GUILabel();
			_LblBestEnergie->Text = "Energie record";
			_LblBestEnergie->X = x;
			_LblBestEnergie->Y = y;
			_LblBestEnergie->Visible = true;
			screen->addElement(_LblBestEnergie);

			y += _LblBestEnergie->Height + 1;
		}

		void showEnergie(float energy, float energyCharge)
		{
			_LblEnergie->Text = std::string("Current score: ") + toString(-energy) + ", charge: " + toString(-energyCharge);
			_PbarScore->setValue(max(0,-energy));
			_PbarCharge->setValue(max(0,-energyCharge));
			updateScoreBars(energy,energyCharge);
			_CurrentEnergy = energy;
		}

		void showBestEnergie(float energy, float energyCharge = 1.0f)
		{
			_LblBestEnergie->Text = std::string("Current best: ") + toString(-energy);
			if(energyCharge <= 0.0f)
				_LblBestEnergie->Text = _LblBestEnergie->Text + ", charge: " + toString(-energyCharge);
			updateScoreBars(energy,energyCharge);
		}

		void updateScoreBars(float energy, float energyCharge )
		{
			_PbarBestScore->setMaxMin(max(20.0f,-_EnergyHighestScore),0);
			_PbarScore->setMaxMin(max(20.0f,-_EnergyHighestScore),0);
			_PbarCharge->setMaxMin(max(20.0f,-_EnergyHighestScore),0);
			_PbarBestScore->setValue(-energy);
			_PbarCharge->setValue(max(0,-energyCharge));

			this->_PbarBestScore->clearBars();
			for(unsigned int i=0;i<_HighScores.size();i++)
			{
				this->_PbarBestScore->addBar(-_HighScores[i].Score, NYColor(1.0,1.0,1.0,0.2f));
			}
			this->_PbarBestScore->addBar(-this->_EnergyPersoBest, NYColor(0.7,0.9,0.7,1.0f),2);
		}

		std::string getBeatedNames(float previousScore, float nextScore)
		{
			std::string res;
			int count = 0;

			//On les comptes
			for(int i=0;i<_HighScores.size();i++)
			{
				if(_HighScores[i].Score <= previousScore && _HighScores[i].Score > nextScore)
				{
					count++;
				}
			}

			//On fait la chaine
			int nbAdd = 0;
			for(int i=0;i<_HighScores.size();i++)
			{
				if(_HighScores[i].Score <= previousScore && _HighScores[i].Score > nextScore)
				{
					if(nbAdd < 4)
					{
						if(nbAdd > 0)
						{
							if(nbAdd < count-1)
								res += ", ";
							else
								res += " and ";
						}
						res += _HighScores[i].Login;
						nbAdd++;
					}
					else
					{
						if(nbAdd < count)
						{
							res += " and "+toString(count-nbAdd)+" other(s)";
							break;
						}
					}
				}
			}
			return res;
		}

		void stopBothMols(void)
		{
			if(_MoleculesToMatch[0]->_Body)
			{
				_MoleculesToMatch[0]->_Body->setAngularSpeed(NYVert3Df(0,0,0));
				_MoleculesToMatch[0]->_Body->setSpeed(NYVert3Df(0,0,0));
			}


			if(_MoleculesToMatch[1]->_Body)
			{
				_MoleculesToMatch[1]->_Body->setAngularSpeed(NYVert3Df(0,0,0));
				_MoleculesToMatch[1]->_Body->setSpeed(NYVert3Df(0,0,0));
			}
		}

		bool evalEnergie(void)
		{
			double energyCharge;
			float energy = (float)_MoleculesManager->calcEnergie(_MoleculesToMatch[0],_MoleculesToMatch[1],true,NULL,NULL,&energyCharge);
			showEnergie(energy, (float) energyCharge);

			if(energy < _EnergySavePosMolLocal)
			{
				showBestEnergie(energy,(float)energyCharge);
				_EnergySavePosMolLocal = energy;

				/*Log::log(Log::ENGINE_INFO,"Saving both mols positions and mol2 files");
				_MoleculesToMatch[0]->savePosition("mol0.pos");
				_MoleculesToMatch[1]->savePosition("mol1.pos");
				_MoleculesToMatch[0]->saveToMOL2("mol0.mol2");
				_MoleculesToMatch[1]->saveToMOL2("mol1.mol2");*/
			}

			//Si première énergie minime, on valide le boout de tuto
			if(energy < -2.0f)
			{
				Tutorial::getInstance()->valideStep(1);
			}

			if(energy < -15.0f)
			{
				Tutorial::getInstance()->valideStep(2);
				Tutorial::getInstance()->gotoStep(3);
				Tutorial::getInstance()->activate(true);
			}


			//Si niveau d'énergie pas mal
			if(energy < _EnergyPersoBest && energy < -20.0f)
			{
				stopBothMols();
				/*HighScoreEffect->explode(30);
				_TexNiceScore->Visible = true;
				_TexBestHighScore->Visible = false;
				_TexPersoBestScore->Visible = false;
				_TexScoreTime = 1.0f;*/

				//Super score : on a battu le best du serveur
				if(energy < _EnergyHighestScore)
				{
					if(PlayerInfo::getInstance()->_ShowFeedBacks)
					{
						if(!_SoundHighScore->isPlaying())
							_SoundHighScore->play();
						HighScoreEffect->explode(100);
						_TexBestHighScore->Visible = true;
						_TexNiceScore->Visible = false;
						_TexPersoBestScore->Visible = false;
						_TexScoreTime = 4.0f;
					}

					Log::log(Log::USER_INFO,("New best highscore : "+toString(energy)).c_str());
				}
				else //On a juste battu notre propre score
				{
					//Perso best
					if(PlayerInfo::getInstance()->_ShowFeedBacks)
					{
						if(!_SoundHighScore->isPlaying())
							_SoundHighScore->play();
						HighScoreEffect->explode(60);
						_TexPersoBestScore->Visible = true;
						_TexNiceScore->Visible = false;
						_TexBestHighScore->Visible = false;
						_TexScoreTime = 4.0f;
					}

					Log::log(Log::USER_INFO,("New personal best : "+toString(energy)).c_str());
				}

				//On recup ceux qu'on a vaincu
				if(PlayerInfo::getInstance()->_ShowFeedBacks)
				{
					std::string beated = getBeatedNames(this->_EnergyPersoBest,energy);
					if(beated.length()>0)
						_LblBeatedNames->Text = "You just beat "+beated+" !";
					else
						_LblBeatedNames->Text = "";
				}


				//On envoie au serveur et on sauve
				this->_Pos1BestEnergy = _MoleculesToMatch[0]->_Transform;
				this->_Pos2BestEnergy = _MoleculesToMatch[1]->_Transform;
				this->_EnergyPersoBest = energy;
				this->_SavePersoBest = true; //demande l'upload

				LogActions::getInstance()->logScore(energy);

				//Affichage sous la barre
				if(_EnergyPersoBest <= _EnergyHighestScore)
				{
					_EnergyHighestScore = (float)_EnergyPersoBest;
					_LblBestLogin->Text = "Best score is " + toString(-(int)energy) +  " by " + PlayerInfo::getInstance()->_Login;
				}
				return true;
			}

			return false;
		}

		void startOptimisation(float duree)
		{
			_DureeTotaleOptim = duree;
			_DureeRestanteOptim = duree;
			_DureeStepOptim = 200;
			_DureeTotaleLastOptim = 0.0f;
			_NbStepsTotalLastOptim = 0;
			_NbAtomsCouplesCalcLastOptim = 0;

			LogActions::getInstance()->logUseOptim(_MoleculesToMatch[0]->_Id,
														  _MoleculesToMatch[1]->_Id,
														  _MoleculesToMatch[0]->_Transform,
														  _MoleculesToMatch[1]->_Transform);
		}

		void cancelOptim()
		{
			_DureeRestanteOptim = 0.0f;
			_PbarOptim->Visible = false;
		}

		//On va optimiser la position des deux molecules, sans utiliser le moteur physique
		//On fait une tranche de temps
		bool optimizeStep(float * pcent)
		{
			if(_DureeRestanteOptim <= 0)
				return false;

			NYVert3Df translation;
			NYFloatMatrix rotation;
			Positionator lastPos;
			Positionator startPos;
			int nbStepsTry = 2;
			int nbStepsTotal = 0;
			int nbGoodSteps = 0;
			float sizeTranslationStepBase = 0.2f;
			float sizeRotationStepBase = (float)M_PI/50.0f;
			float sizeTranslationStep = sizeTranslationStepBase;
			float sizeRotationStep = sizeRotationStepBase;

			/*LARGE_INTEGER t1,t2;
			LONGLONG elapsedLong;
			QueryPerformanceCounter(&t1);
			LARGE_INTEGER li_freq;
			QueryPerformanceFrequency(&li_freq);
			LONGLONG freq = li_freq.QuadPart;
			freq /= 1000;*/

			NYTimer tStart;

			startPos = _MoleculesToMatch[0]->_Transform;

			//On se debranche du moteur physique, si c'est le début
			if(_DureeRestanteOptim == _DureeTotaleOptim )
			{
				Log::log(Log::ENGINE_INFO,"Fist step of optim");
				_MoleculesToMatch[0]->removeFromPhysicEngine(_PhysicEngine);
				_MoleculesToMatch[1]->removeFromPhysicEngine(_PhysicEngine);
			}

			//On calcule une première fois, permet aussi de mettre en cache la mol 2
			float energieStart = (float) _MoleculesManager->calcEnergie(_MoleculesToMatch[0],_MoleculesToMatch[1],true,&_NbAtomsCouplesCalcLastOptim);
			float energie = energieStart;
			float energieLastStep = energieStart;

			if(energieStart < -8.0f)
			{
				Tutorial::getInstance()->valideStep(3);
			}

			Log::log(Log::ENGINE_INFO,("Starting optim : "+toString(energieStart)).c_str());

			float elapsed = 0;
			bool lastGood = false;

			//On optim un temps donne
			while (elapsed < _DureeStepOptim)
			{
				//On sauve la position actuelle
				lastPos = _MoleculesToMatch[0]->_Transform;

				//temps ecoule
				/*QueryPerformanceCounter(&t2);
				LONGLONG elapsedLong = t2.QuadPart-t1.QuadPart;
				elapsed = (float) ((float)elapsedLong/(float)freq);*/
				elapsed = tStart.getElapsedSeconds(false);

				//Si le déplacement précédent était mauvais
				if(!lastGood)
				{
					sizeTranslationStep = sizeTranslationStepBase * (max(0,(_DureeRestanteOptim-elapsed))/_DureeTotaleOptim);
					sizeRotationStep = sizeRotationStepBase *(max(0,(_DureeRestanteOptim-elapsed))/_DureeTotaleOptim);

					//On cree une transformation aléatoire
					if(nbStepsTotal%2==0)
					{
						translation = NYVert3Df((2.0f*((float)randf()-0.5f))*sizeTranslationStep,(2.0f*((float)randf()-0.5f))*sizeTranslationStep,(2.0f*((float)randf()-0.5f))*sizeTranslationStep);
						rotation.createIdentite();
						//_cprintf("Translation : %f %f %f\n",translation.X,translation.Y,translation.Z);
					}
					else
					{
						rotation.createRotateXYZ((2.0f*((float)randf()-0.5f))*sizeRotationStep,(2.0f*((float)randf()-0.5f))*sizeRotationStep,(2.0f*((float)randf()-0.5f))*sizeRotationStep);
						translation = NYVert3Df();
					}
				}

				//On applique (la nouvelle ou la dernière si elle était bien)
				_MoleculesToMatch[0]->_Transform._Pos += translation;
				_MoleculesToMatch[0]->_Transform._Rot *= rotation;

				//On calcule l'energie, sans déplacer la mol2
				energie = (float) _MoleculesManager->calcEnergie(_MoleculesToMatch[0],_MoleculesToMatch[1],false,&_NbAtomsCouplesCalcLastOptim);

				//Si c'est mieux on garde, sinon on revient en arrière
				if(energie >= energieLastStep)
				{
					_MoleculesToMatch[0]->_Transform = lastPos;
					//_cprintf("Bad : %f -> %f\n",energieLastStep, energie);
				}
				else
				{
					//_cprintf("Good : %f -> %f\n",energieLastStep, energie);
					energieLastStep = energie;
					nbGoodSteps++;

					_Pos1LastOptim = _MoleculesToMatch[0]->_Transform;
					_Pos2LastOptim = _MoleculesToMatch[1]->_Transform;
				}
				nbStepsTotal++;

				/*if(nbStepsTotal%nbStepsTry == 0)
				{
					QueryPerformanceCounter(&t2);
					LONGLONG elapsedLong = t2.QuadPart-t1.QuadPart;
					elapsed = (float) ((float)elapsedLong/(float)freq);

					sizeTranslationStep = sizeTranslationStepBase * (max(0,(_DureeRestanteOptim-elapsed))/_DureeTotaleOptim);
					sizeRotationStep = sizeRotationStepBase *(max(0,(_DureeRestanteOptim-elapsed))/_DureeTotaleOptim);
					//_cprintf("Trans %f / rot %f , restant : %f\n",sizeTranslationStep, sizeRotationStep, _DureeRestanteOptim);

				}*/
			}

			_DureeRestanteOptim -= elapsed;
			if(_DureeRestanteOptim <= 0)
				_DureeRestanteOptim = 0;

			//On set sur le body car le moteur va y recherche la position
			_MoleculesToMatch[0]->setToPhysic();

			elapsed = tStart.getElapsedSeconds(false);

			Log::log(Log::ENGINE_INFO,("Ended opt step : " +
										toString(energieStart) +
										" -> " +
										toString(energieLastStep) +
										" " +
										toString(elapsed) +
										" " +
										toString(nbGoodSteps) +
										"/" +
										toString(nbStepsTotal)).c_str());

			Log::log(Log::ENGINE_INFO,("Step params : trans = " +
										toString(sizeTranslationStep) +
										" rot =" +
										toString(sizeRotationStep)).c_str());

			_NbStepsTotalLastOptim += nbStepsTotal;
			_DureeTotaleLastOptim += elapsed;

			//Si on a fini toute l'optimization
			if(_DureeRestanteOptim <= 0)
			{
				Log::log(Log::ENGINE_INFO,"Last step of optim");

				//On enterrine
				evalEnergie();
				_MoleculesToMatch[0]->addToPhysicEngine(_PhysicEngine);
				_MoleculesToMatch[0]->setToPhysic();
				_MoleculesToMatch[1]->addToPhysicEngine(_PhysicEngine);
				_MoleculesToMatch[1]->setToPhysic();

				//On freeze
				freezeMols(true);

				_Pos1LastOptim = _MoleculesToMatch[0]->_Transform;
				_Pos2LastOptim = _MoleculesToMatch[1]->_Transform;

				//On sort les stats
				Log::log(Log::ENGINE_INFO,("Ended optimization : " +
					toString(_NbStepsTotalLastOptim) +
					" steps in " +
					toString(_DureeTotaleLastOptim) +
					"ms,\n\t" +
					toString(_NbAtomsCouplesCalcLastOptim / 1000000) +
					"M atom couples evaluated,\n\t" +
					toString((float)_DureeTotaleLastOptim*1000000.0f / (float)_NbAtomsCouplesCalcLastOptim) +
					"ns per couple").c_str());

			}

			*pcent = 1.0f-(_DureeRestanteOptim / _DureeTotaleOptim);
			*pcent *= 100.0f;

			return true;
		}


		void loadLastPos(void)
		{
			/*_MoleculesToMatch[0]->loadPosition("mol0.pos");
			_MoleculesToMatch[0]->setToPhysic();
			_MoleculesToMatch[1]->loadPosition("mol1.pos");
			_MoleculesToMatch[1]->setToPhysic();*/
			_MoleculesToMatch[0]->_Transform = _Pos1LastOptim;
			_MoleculesToMatch[1]->_Transform = _Pos2LastOptim;
			evalEnergie();
			stopBothMols();
		}

		void applyDockingForces(void)
		{
			int nbDockingPointsActive = 0;
			for(int i=0;i<NB_DOCKING_POINTS;i++)
			{
				if(_MoleculesToMatch[0]->_DockingPoints[i]._IsSet && _MoleculesToMatch[1]->_DockingPoints[i]._IsSet)
					nbDockingPointsActive = i+1;
			}

			if(nbDockingPointsActive > 0)
			{
				stopBothMols();

				NYVert3Df pointWorldPos[2][NB_DOCKING_POINTS];
				for(int i=0;i<nbDockingPointsActive;i++)
				{
					pointWorldPos[0][i] = _MoleculesToMatch[0]->_Transform.Transform(_MoleculesToMatch[0]->_DockingPoints[i]._Pos);
					pointWorldPos[1][i] = _MoleculesToMatch[1]->_Transform.Transform(_MoleculesToMatch[1]->_DockingPoints[i]._Pos);
				}

				NYVert3Df pointRelPos[2][NB_DOCKING_POINTS];
				for(int i=0;i<nbDockingPointsActive;i++)
				{
					pointRelPos[0][i] = pointWorldPos[0][i] - _MoleculesToMatch[0]->_Transform._Pos;
					pointRelPos[1][i] = pointWorldPos[1][i] - _MoleculesToMatch[1]->_Transform._Pos;
				}

				NYVert3Df forces[NB_DOCKING_POINTS];
				NYVert3Df sumForce;
				for(int i=0;i<nbDockingPointsActive;i++)
				{
					forces[i] = pointWorldPos[1][i] - pointWorldPos[0][i];
					sumForce += forces[i];
				}

				float valForces = sumForce.getSize();
				if(valForces > 0.5f)
				{
					for(int i=0;i<nbDockingPointsActive;i++)
					{
						forces[i] /= valForces;
						forces[i] *= 300.0f;
					}
				}

				//On calcule la somme de torque a appliquer sur chaque molecule
				NYVert3Df sumTorque[2];
				for(int i=0;i<nbDockingPointsActive;i++)
				{
					sumTorque[0] += pointRelPos[0][i].vecProd(forces[i]);
					sumTorque[1] += pointRelPos[1][i].vecProd(forces[i]*-1);
				}

				float val0Torque = sumTorque[0].getSize();
				float val1Torque = sumTorque[1].getSize();

				if(val0Torque > 15000 || val1Torque > 15000)
				{
					if(_MoleculesToMatch[0]->_Body)
						_MoleculesToMatch[0]->_Body->applyTorque(sumTorque[0]);
					if(_MoleculesToMatch[1]->_Body)
						_MoleculesToMatch[1]->_Body->applyTorque(sumTorque[1]);
					//_cprintf("%f %f\n",val0Torque, val1Torque);
				}
				else
				{
					for(int i=0;i<nbDockingPointsActive;i++)
					{
						if(_MoleculesToMatch[0]->_Body)
							_MoleculesToMatch[0]->_Body->applyForce(forces[i],pointRelPos[0][i]);
						if(_MoleculesToMatch[1]->_Body)
							_MoleculesToMatch[1]->_Body->applyForce(forces[i]*-1,pointRelPos[1][i]);
					}

					//On les ramène au centre
					NYVert3Df _CentreMols = _MoleculesToMatch[0]->_Transform._Pos + _MoleculesToMatch[1]->_Transform._Pos;
					_CentreMols /= -2.0f;
					_CentreMols *= 100.f;

					if(_MoleculesToMatch[0]->_Body)
						_MoleculesToMatch[0]->_Body->applyForce(_CentreMols);
					if(_MoleculesToMatch[1]->_Body)
						_MoleculesToMatch[1]->_Body->applyForce(_CentreMols);
				}



				//MoleculesToMatch[0]->_Body->applyForce(distance);
				//distance *= -1;
				//MoleculesToMatch[1]->_Body->applyForce(distance);
			}
		}

		void dampMols(float elapsed, bool damp, Molecule * ignoreMol = NULL)
		{
			static NYVert3Df angDamping[2], linDamping[2];
			static float _DureeFade = 1.0f;
			static bool lastDamp = false;
			static float elapsedFade = 0;

			if(!damp)
			{
				lastDamp = false;
				return;
			}

			NYVert3Df angSpeed[2], linSpeed[2];

			if(_MoleculesToMatch[0]->_Body)
				angSpeed[0] = _MoleculesToMatch[0]->_Body->getAngularSpeed();
			if(_MoleculesToMatch[1]->_Body)
				angSpeed[1] = _MoleculesToMatch[1]->_Body->getAngularSpeed();

			if(_MoleculesToMatch[0]->_Body)
				linSpeed[0] = _MoleculesToMatch[0]->_Body->getSpeed();
			if(_MoleculesToMatch[1]->_Body)
				linSpeed[1] = _MoleculesToMatch[1]->_Body->getSpeed();

			if(lastDamp == false && damp == true)
			{
				angDamping[0] = angSpeed[0] / _DureeFade;
				angDamping[1] = angSpeed[1] / _DureeFade;
				linDamping[0] = linSpeed[0] / _DureeFade;
				linDamping[1] = linSpeed[1] / _DureeFade;
				lastDamp = true;
				elapsedFade = 0.0f;
			}

			if(abs(angSpeed[0].X) >= abs(angDamping[0].X)*elapsed)
				angSpeed[0] -= angDamping[0]*elapsed;
			else
				angSpeed[0] = NYVert3Df();

			if(abs(angSpeed[1].X) >= abs(angDamping[1].X)*elapsed)
				angSpeed[1] -= angDamping[1]*elapsed;
			else
				angSpeed[1] = NYVert3Df();

			if(abs(linSpeed[0].X) >= abs(linSpeed[0].X)*elapsed)
				linSpeed[0] -= linSpeed[0]*elapsed;
			else
				linSpeed[0] = NYVert3Df();

			if(abs(linSpeed[1].X) >= abs(linSpeed[1].X)*elapsed)
				linSpeed[1] -= linSpeed[1]*elapsed;
			else
				linSpeed[1] = NYVert3Df();

			if(_MoleculesToMatch[0]->_Body && ignoreMol != _MoleculesToMatch[0])
				 _MoleculesToMatch[0]->_Body->setAngularSpeed(angSpeed[0]);
			if(_MoleculesToMatch[1]->_Body && ignoreMol != _MoleculesToMatch[1])
				_MoleculesToMatch[1]->_Body->setAngularSpeed(angSpeed[1]);

			 if(_MoleculesToMatch[0]->_Body  && ignoreMol != _MoleculesToMatch[0])
				_MoleculesToMatch[0]->_Body->setSpeed(linSpeed[0]);
			 if(_MoleculesToMatch[1]->_Body && ignoreMol != _MoleculesToMatch[1])
				_MoleculesToMatch[1]->_Body->setSpeed(linSpeed[1]);

			 /*if(linSpeed[0].getMagnitude() == 0 && angSpeed[0].getMagnitude() == 0 && g_selected_mol !=  MoleculesToMatch[0])
				 MoleculesToMatch[0]->_Body->setFreeze(true);
			 else
				 if(MoleculesToMatch[0]->_Body->Frozen)
					MoleculesToMatch[0]->_Body->setFreeze(false);

			 if(linSpeed[1].getMagnitude() == 0 && angSpeed[1].getMagnitude() == 0 && g_selected_mol !=  MoleculesToMatch[1])
				 MoleculesToMatch[1]->_Body->setFreeze(true);
			 else
				 if(MoleculesToMatch[1]->_Body->Frozen)
					 MoleculesToMatch[1]->_Body->setFreeze(false);*/


			elapsedFade += elapsed;
			if(elapsedFade > _DureeFade)
			{
				elapsedFade = 0.0f;
				lastDamp = false;
			}
		}

		void resetPositions(void)
		{
			NYVert3Df rotRnd1((float)randf(),(float)randf(),(float)randf());
			NYVert3Df rotRnd2((float)randf(),(float)randf(),(float)randf());
			rotRnd1.normalize();
			rotRnd2.normalize();

			if(_MoleculesToMatch[0]->_Body)
			{
				_MoleculesToMatch[0]->_Body->setSpeed(NYVert3Df(0,0,0));
				_MoleculesToMatch[0]->_Body->setAngularSpeed(NYVert3Df(0,0,0));
				NYVert4Df rot(rotRnd1.X,rotRnd1.Y,rotRnd1.Z,1);
				_MoleculesToMatch[0]->_Body->setBodyRotation(rot);
				float size = _MoleculesToMatch[0]->_Body->getWidth();
				NYVert3Df pos(-(size/2.0f),0,0);
				_MoleculesToMatch[0]->_Body->setBodyPosition(pos);
			}


			if(_MoleculesToMatch[1]->_Body)
			{
				_MoleculesToMatch[1]->_Body->setSpeed(NYVert3Df(0,0,0));
				_MoleculesToMatch[1]->_Body->setAngularSpeed(NYVert3Df(0,0,0));
				NYVert4Df rot2(rotRnd2.X,rotRnd2.Y,rotRnd2.Z,1);
				_MoleculesToMatch[1]->_Body->setBodyRotation(rot2);
				float size = _MoleculesToMatch[1]->_Body->getWidth();
				NYVert3Df pos2(size/2.0f,0,0);
				_MoleculesToMatch[1]->_Body->setBodyPosition(pos2);
			}

			for(int i=0;i<NB_DOCKING_POINTS;i++)
			{
				_MoleculesToMatch[0]->_DockingPoints[i]._IsSet = false;
				_MoleculesToMatch[1]->_DockingPoints[i]._IsSet = false;
			}

			//g_renderer->_Camera->set(NYVert3Df(0,-190,0),NYVert3Df(0,0,0));
			((NYCameraAnimated*)(_Renderer->_Camera))->startAnimTo(NYVert3Df(0,-190,0),NYVert3Df(0,1,0),NYVert3Df(0,0,0),0.5f);

			if(PlayerInfo::getInstance()->_ShowFeedBacks)
				_SoundReset->play();
		}

		void freezeMols(bool freeze)
		{
			_MoleculesManager->removeMolsFromPhysicEngine(_PhysicEngine);
			_MoleculesManager->addMolsToPhysicEngine(_PhysicEngine);
			_MoleculesManager->freezeAllMols(freeze);
			if(freeze)
			{
				_BtnFreeze->Titre = "Unfreeze";
				_BtnFreeze->FondPlein = true;
			}
			else
			{
				_BtnFreeze->Titre = "Freeze";
				_BtnFreeze->FondPlein = false;
			}
		}

		bool activate(bool active)
		{
			GameState::activate(active);

			Tutorial::getInstance()->activate(active);

			//Le bon ecran
			if(active)
				_ScreenManager->setActiveScreen(_Screen);

			if(_MoleculesManager->_Molecules.size() >=2)
			{
				_MoleculesToMatch[0] = _MoleculesManager->_Molecules[0];
				_MoleculesToMatch[1] = _MoleculesManager->_Molecules[1];
			}

			if(_MoleculesToMatch[0] == NULL ||
			   _MoleculesToMatch[1] == NULL ||
			   _MoleculesToMatch[0] == _MoleculesToMatch[1])
			   return false;

			//On log
			if(active)
				LogActions::getInstance()->logStartDocking(_MoleculesToMatch[0]->_Id,_MoleculesToMatch[1]->_Id);

			//On recup le bon score
			if(active)
			{
				HttpManagerUdock::getInstance()->getHighScore(PlayerInfo::getInstance()->_IdUser,
														 _MoleculesToMatch[0]->_Id,
														 _MoleculesToMatch[1]->_Id,
														 &(this->_EnergyPersoBest),
														 _HighScores);
				this->_EnergyHighestScore = 0.0f;
				this->_LoginBestScore = "";
				if(_HighScores.size() > 0)
				{
					this->_EnergyHighestScore = _HighScores[_HighScores.size()-1].Score;
					this->_LoginBestScore = _HighScores[_HighScores.size()-1].Login;

					if(this->_EnergyPersoBest < this->_EnergyHighestScore)
					{
						this->_EnergyHighestScore = this->_EnergyPersoBest;
						this->_LoginBestScore = PlayerInfo::getInstance()->_Login;
					}
				}

				//Au cas ou pas de high scores, que le mien, il faut le faire une fois
				if(this->_EnergyPersoBest < this->_EnergyHighestScore)
				{
					this->_EnergyHighestScore = this->_EnergyPersoBest;
					this->_LoginBestScore = PlayerInfo::getInstance()->_Login;
				}

				updateScoreBars(0.0f,0.0f);

				//On raz l'energie minim pour fichiers locaux
				_EnergySavePosMolLocal = 0;

				/*_MoleculesToMatch[0]->savePosition("mol0.pos");
				_MoleculesToMatch[1]->savePosition("mol1.pos");
				_MoleculesToMatch[0]->saveToMOL2("mol0.mol2");
				_MoleculesToMatch[1]->saveToMOL2("mol1.mol2");*/

				_Pos1LastOptim = _MoleculesToMatch[0]->_Transform;
				_Pos2LastOptim = _MoleculesToMatch[1]->_Transform;

				showBestEnergie((float)_EnergyPersoBest);
				if(_EnergyHighestScore < 0)
					_LblBestLogin->Text = "Best score is " + toString(-(int)_EnergyHighestScore) +  " by " + _LoginBestScore;
				else
					_LblBestLogin->Text = "";
			}

			//On ajoute les molecules
			if(active)
			{
				bool frozen = false;
				if(_MoleculesToMatch[0]->_Body)
					frozen = _MoleculesToMatch[0]->_Body->Frozen;

				_MoleculesManager->hideAllMols(true);
				_MoleculesManager->removeMolsFromPhysicEngine(_PhysicEngine);
				_MoleculesToMatch[0]->addToPhysicEngine(_PhysicEngine);
				_MoleculesToMatch[1]->addToPhysicEngine(_PhysicEngine);
				_MoleculesToMatch[0]->_Visible = true;
				_MoleculesToMatch[1]->_Visible = true;
				this->freezeMols(frozen);
				//resetPositions();

				_Renderer->_Camera->setUpRef(NYVert3Df(0,0,1));
				((NYCameraAnimated*)(_Renderer->_Camera))->startAnimTo(NYVert3Df(0,-190,0),NYVert3Df(0,0,1),NYVert3Df(0,0,0),0.5f);

				//LogScreen::getInstance()->setMessage("Docking started");
			}
			else
			{
				//resetPositions();
				cancelOptim();
				_MoleculesToMatch[0]->removeFromPhysicEngine(_PhysicEngine);
				_MoleculesToMatch[1]->removeFromPhysicEngine(_PhysicEngine);
				_MoleculesToMatch[0]->_Visible = false;
				_MoleculesToMatch[1]->_Visible = false;
			}

			return active;
		}

		void preState(float elapsed)
		{
			static float elapsedLogComplex = 0;
			elapsedLogComplex += elapsed;

			//Si on ne fait rien, alors on applique un damping
			dampMols(elapsed,!_Force,_RoateMol ? _SelectedMol : NULL);

			if(_Force)
			{
				/*if(elapsedLogComplex > 3000.0f)
					LogActions::getInstance()->logComplex(_MoleculesToMatch[0]->_Id,
														  _MoleculesToMatch[1]->_Id,
														  _MoleculesToMatch[0]->_Transform,
														  _MoleculesToMatch[1]->_Transform);*/
				applyDockingForces();
			}

			HighScoreEffect->update(elapsed);
			if(_TexScoreTime > elapsed)
				_TexScoreTime -= elapsed;
			else
			{
				if(_TexScoreTime >= 0)
				{
					_TexScoreTime = 0.0f;
					_TexNiceScore->Visible = false;
					_TexPersoBestScore->Visible = false;
					_TexBestHighScore->Visible = false;

				}
			}

			Tutorial::getInstance()->update(elapsed);
		}

		void postState(float elapsed)
		{
			//Si demande de reload puis optim, on le fait ici pour ne pas laisser le moteur physique
			//avoir une chance d'intervenir aprèes le reload.
			if(_OptimizeAfterReload)
			{
				_OptimizeAfterReload = false;
				loadLastPos();
				startOptimisation(5000.0f);
			}

			//Si optim en cours on la fait
			float pcent = 0.0f;
			int nbSteps = 0;
			float duree = 0.0f;
			if(!optimizeStep(&pcent))
			{
				_PbarOptim->Visible = false;

				//Si pas optim, on fait eval auto
				_EvalElapsed += elapsed;
				if(_EvalElapsed > 0.25)
				{
					_EvalElapsed -= 0.25;
					evalEnergie();
				}
			}
			else
			{
				//On met a jour la progress bar
				_PbarOptim->setValue(pcent);
				_PbarOptim->Visible = true;

				//On met a jour la progressbar du score
				double energyCharge;
				float energy = (float)_MoleculesManager->calcEnergie(_MoleculesToMatch[0],_MoleculesToMatch[1],true,NULL,NULL,&energyCharge);
				showEnergie(energy, (float) energyCharge);
			}

			//Si on doit poster le score
			if(this->_SavePersoBest && !_JobPostScore->_JobRunning)
			{
				_JobPostScore->_UserId = PlayerInfo::getInstance()->_IdUser;
				_JobPostScore->_Pos1 = this->_Pos1BestEnergy;
				_JobPostScore->_Pos2 = this->_Pos2BestEnergy;
				_JobPostScore->_IdMol1 = _MoleculesToMatch[0]->_Id;
				_JobPostScore->_IdMol2 = _MoleculesToMatch[1]->_Id;
				_JobPostScore->_Score = this->_EnergyPersoBest;
				_JobPostScore->launchJob();
				this->_SavePersoBest = false;
			}

			//Si le job est fini, on reset, on dit si erreur
			if(_JobPostScore->_JobRunning)
			{
				if(_JobPostScore->_JobDone)
				{
					if(_JobPostScore->_PostOk == false)
					{
						Log::log(Log::USER_ERROR,"Unable to post new score on server");
						this->_SavePersoBest = true; //on retente
					}
					_JobPostScore->_JobDone = false;
					_JobPostScore->_JobRunning = false;
				}
			}

			//Si on doit poster le temps de jeu
			this->_PlayTime += elapsed;
			if(this->_PlayTime > 30.0f)
			{
				if(!(_JobAddPlayTime->_JobRunning && !_JobAddPlayTime->_JobDone))
				{
					_JobAddPlayTime->_UserId = PlayerInfo::getInstance()->_IdUser;
					_JobAddPlayTime->_Time = this->_PlayTime;
					_JobAddPlayTime->_JobDone = false;
					_JobAddPlayTime->_JobRunning = false;
					_JobAddPlayTime->launchJob();
					this->_PlayTime = 0.0f;
				}
			}
		}




		//INPUT
		void specialDownFunction(int key, int p1, int p2)
		{
			if(key == SDL_SCANCODE_F1)
			{
				_MoleculesToMatch[0]->savePosition("mol0.pos");
				_MoleculesToMatch[1]->savePosition("mol1.pos");
			}

			if(key == SDL_SCANCODE_F7)
			{
				//this->_SavePersoBest = true;
			}

			if(key == SDL_SCANCODE_F5)
			{
				startOptimisation(5000.0f);
			}

			if(key == SDL_SCANCODE_F6)
			{
				_OptimizeAfterReload = true;
			}


		}

		void keyboardDownFunction(unsigned char key, int p1, int p2)
		{
			if(key == SDL_SCANCODE_SPACE)
			{
				_Force = true;
			}

			if(key ==SDL_SCANCODE_R)
			{
				resetPositions();
			}
		}

		void keyboardUpFunction(unsigned char key, int p1, int p2)
		{
			if(key == SDL_SCANCODE_SPACE)
			{
				_Force = false;
			}
		}

		void mouseWheelFunction(int wheel, int dir, int x, int y)
		{
			NYVert3Df trans = _Renderer->_Camera->_Direction;
			trans *= 30.0f * dir;
			((NYCameraAnimated*)(_Renderer->_Camera))->startAnimTo(_Renderer->_Camera->_Position+trans,NYVert3Df(0,1,0),_Renderer->_Camera->_LookAt,0.25f);
		}

		void mouseFunction(int button, bool down, int x, int y)
		{
			NYVert3Df hitPoint;

			//Clavier
			const Uint8 *keybState = SDL_GetKeyboardState(NULL);

			//Gestion de la selection
			if(button == SDL_BUTTON_LEFT || button == SDL_BUTTON_RIGHT)
			{
				if(down)
				{
					_SelectedMol = NULL;
					NYBody * body = getPointedBody(&hitPoint);
					if(body)
					{
						if(body->BodyOwnerType == OWNER_MOLECULE)
							_SelectedMol = (Molecule*)(body->BodyOwner);
					}
				}
			}

			//Rotation d'une molecule
			if(_SelectedMol && button == SDL_BUTTON_LEFT && !(keybState[SDL_SCANCODE_LCTRL]))
			{
				if(!down)
					_RoateMol = false;
				else
				{
					//On initialise les paramètres, le polling mouse se chargera du reste
					_RotatePoint = hitPoint-_SelectedMol->_Transform._Pos;
					NYFloatMatrix rot = _SelectedMol->_Transform._Rot;
					rot.invert();
					_RotatePoint = rot * _RotatePoint;
					_RoateMol = true;
				}
			}

			//Rotation de la cam
			if(button == SDL_BUTTON_RIGHT && !(keybState[SDL_SCANCODE_LCTRL]))
			{
				if(!down)
					_RotateCam = false;
				else
					_RotateCam = true;
			}


			//Ajout ou suppression
			if((button == SDL_BUTTON_LEFT || button == SDL_BUTTON_RIGHT) && down && (keybState[SDL_SCANCODE_LCTRL]))
			{
				if(_SelectedMol)
				{
					NYVert3Df from = _Renderer->_Camera->_Position;
					double posX,posY,posZ;
					_Renderer->unProjectMousePos(x,y,1.0,&posX,&posY,&posZ);

					NYVert3Df to ((float)posX,(float)posY,(float)posZ);

					NYVert3Df relpos = hitPoint-_SelectedMol->_Transform._Pos;
					NYFloatMatrix rot = _SelectedMol->_Transform._Rot;
					rot.invert();
					relpos = rot * relpos;

					//Log::log(Log::ENGINE_INFO,("Rel pos "+ relpos.toStr()).c_str());

					//Si c'est un ajout
					if(button == SDL_BUTTON_LEFT)
					{
						bool reset = true;
						for(int i=0;i<NB_DOCKING_POINTS;i++)
						{
							if(!_SelectedMol->_DockingPoints[i]._IsSet)
							{
								_SelectedMol->setDockingPoint(i,relpos,true);
								reset = false;
								_SoundsAttach[rand()%_NbSoundAttach]->play();
								LogActions::getInstance()->logSetHarpoonHandle(_SelectedMol->_Id,relpos);
								break;
							}
						}

						//Si on a un harpon bien placé des deux cotés, on valide l'étape du tuto
						int nbDockingPointsActive = 0;
						for(int i=0;i<NB_DOCKING_POINTS;i++)
						{
							if(_MoleculesToMatch[0]->_DockingPoints[i]._IsSet && _MoleculesToMatch[1]->_DockingPoints[i]._IsSet)
								nbDockingPointsActive = i+1;
						}
						if(nbDockingPointsActive > 0)
						{
							Tutorial::getInstance()->valideStep(0);
						}
					}

					//si c'est une suppression
					if(button == SDL_BUTTON_RIGHT)
					{
						float minDist = 0.0f;
						int selected = -1;
						for(int i=0;i<NB_DOCKING_POINTS;i++)
						{
							if(_SelectedMol->_DockingPoints[i]._IsSet)
							{
								float dist = (_SelectedMol->_DockingPoints[i]._Pos - relpos).getMagnitude();
								//Log::log(Log::ENGINE_INFO,("Distance "+toString(dist) + " " + relpos.toStr()).c_str());
								if(selected == -1 || dist < minDist)
								{
									selected = i;
									minDist = dist;
								}
							}
						}

						if(selected >= 0)
						{
							_MoleculesToMatch[0]->_DockingPoints[selected]._IsSet = false;
							_MoleculesToMatch[1]->_DockingPoints[selected]._IsSet = false;
							_SoundDetach->play();
						}
					}
				}
			}
		}

		void mousePollFunction(float elapsed)
		{
			static int lastx = -1;
			static int lasty = -1;
			static float elaspedSinceLastMove = 0;

			/*POINT p;
			GetCursorPos(&p);
			BOOL res = ScreenToClient(_Renderer->_WHnd,&p);

			int x = p.x;
			int y = p.y;*/

			int x = 0;
			int y = 0;

			SDL_GetMouseState(&x,&y);

			elaspedSinceLastMove += elapsed;

			/*if(x != lastx || y != lasty)
				elaspedSinceLastMove = 0.0f;

			//Une seconde sans bouger
			if(elaspedSinceLastMove > 1.0)
			{
				elaspedSinceLastMove = 0.0f;
				NYVert3Df hitPoint;
				Molecule * mol = getPointedMol(&hitPoint);
				if(mol)
				{
					//On rend le nom de la molécule
				}
			}*/

			if(!_RoateMol)
			{
				lastx = x;
				lasty = y;
			}
			else
			{
				if(lastx == -1 && lasty == -1)
				{
					lastx = x;
					lasty = y;
				}

				if(_SelectedMol && _SelectedMol->_Body)
				{
					double posX,posY,posZ;
					_Renderer->unProjectMousePos(x,y,0.0f,&posX,&posY,&posZ);
					NYVert3Df now ((float)posX,(float)posY,(float)posZ);
					_Renderer->unProjectMousePos(lastx,lasty,0.0f,&posX,&posY,&posZ);
					NYVert3Df before ((float)posX,(float)posY,(float)posZ);

					NYVert3Df angSpeed = _SelectedMol->_Body->getAngularSpeed();
					NYVert3Df damping = (angSpeed/5.0f) * 60.0f * elapsed;
					if(damping.getMagnitude() < angSpeed.getMagnitude())
						angSpeed -= damping;
					else
						angSpeed = NYVert3Df();
					_SelectedMol->_Body->setAngularSpeed(angSpeed);
					_SelectedMol->_Body->setSpeed(NYVert3Df());

					_RotatePointWorld = _SelectedMol->_Transform.Transform(_RotatePoint);

					//On utilise thales
					float distCamRpw = (_Renderer->_Camera->_Position - _RotatePointWorld).getSize();
					float distCamLast = (_Renderer->_Camera->_Position - before).getSize();
					float rapportThales = distCamRpw / distCamLast;

					NYVert3Df vertCamNow = now - _Renderer->_Camera->_Position;
					vertCamNow *= rapportThales;
					_PosNowNearMol = _Renderer->_Camera->_Position + vertCamNow;


					NYVert3Df force = _PosNowNearMol - _RotatePointWorld;
					NYVert3Df relPos = _RotatePointWorld - _SelectedMol->_Transform._Pos;

					if(force.getMagnitude() > 0.01)
					{
						//On va vérifier si on pointe vers le barycentre, auquel cas on va tricher un peu pour aider le déplacement
						NYVert3Df relPosNorm = relPos;
						relPosNorm.normalize();
						NYVert3Df forceNorm = force;
						forceNorm.normalize();
						float scalProd = relPosNorm.scalProd(forceNorm);
						//if(abs(scalProd) > 0.9)
						if(scalProd < 0)
						{
							//on le ramène un peu vers nous
							//Pour éviter les hésitation de rotation quand on tire vers le barycentre
							vertCamNow.normalize();
							vertCamNow *= -1.0f * force.getSize() * abs(scalProd) ;
							_PosNowNearMol += vertCamNow;
							force = _PosNowNearMol - _RotatePointWorld;
							relPos = _RotatePointWorld - _SelectedMol->_Transform._Pos;
						}

						if(force.getSize() > 1)
						{
							force.normalize();
							force *= 1;
						}

						NYVert3Df torque = relPos.vecProd(force*400);
						_SelectedMol->_Body->applyTorque(torque);
					}

				}

				lastx = x;
				lasty = y;

			}
		}

		void mouseMoveFunction(int buttonsState, int x, int y)
		{
			static int lastx = -1;
			static int lasty = -1;

			if(!(buttonsState & SDL_BUTTON(3)))
			{
				lastx = x;
				lasty = y;
			}
			else
			{
				if(lastx == -1 && lasty == -1)
				{
					lastx = x;
					lasty = y;
				}

				int dx = x-lastx;
				int dy = y-lasty;

				if(_RotateCam)
				{
					_Renderer->_Camera->rotateAround((float)-dx/300.0f);
					_Renderer->_Camera->rotateUpAround((float)-dy/300.0f);
				}

				lastx = x;
				lasty = y;
			}

		}

};

#endif
