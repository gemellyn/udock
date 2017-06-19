#ifndef __TUTORIAL_H__
#define __TUTORIAL_H__

#include "engine/render/graph/tex_manager.h"
#include "engine/render/renderer.h"

#define NB_STEPS 4

class Tutorial
{
	private:

		bool _Steps[NB_STEPS];
		NYTexFile * _Texs[NB_STEPS];
		NYVert3Df _Pos[NB_STEPS];
		float _StepDelay[NB_STEPS];
		bool _AutoActivate[NB_STEPS];
		float _StepDesactivationDelay[NB_STEPS];
		int _CurrentStep;

		static Tutorial * _Instance;
		bool _Active;
		bool _Render;

		float _DelayNextStep;


		void loadSteps(void)
		{
			for(int i=0;i<NB_STEPS;i++)
				_Steps[i] = false;
			FILE * fe = fopen("tuto.bin","rb");
			if(fe)
			{
				if(fread(_Steps,sizeof(bool),NB_STEPS,fe) != NB_STEPS)
                    Log::log(Log::USER_ERROR,"Tuto is corrupted, not enough steps.");
				fclose(fe);
			}
			_CurrentStep = findNextStep();
		}

		int findNextStep(void)
		{
			for(int i=0;i<NB_STEPS;i++)
			{
				if(_Steps[i] == false)
				{
					return i;
				}
			}
			return -1;
		}

		void saveSteps(void)
		{
			FILE * fs = fopen("tuto.bin","wb");
			if(fs)
			{
				fwrite(_Steps,sizeof(bool),NB_STEPS,fs);
				fclose(fs);
			}
		}

		Tutorial()
		{
			for(int i=0;i<NB_STEPS;i++)
			{
				string name = "textures/tuto_"+toString(i)+".png";
				_Texs[i] = NYTexManager::getInstance()->loadTexture(name);
			}
			loadSteps();
			resize();
			for(int i=0;i<NB_STEPS;i++)
			{
				_StepDelay[i] = 5.0f;
			}
			for(int i=0;i<NB_STEPS;i++)
			{
				_AutoActivate[i] = true;
			}
			_Active = false;
			_Render = false;


			//Specificités
			_AutoActivate[3] = false;
			_StepDelay[3] = 0;
		}

	public:

		static Tutorial* getInstance(void)
		{
			if(!_Instance)
				_Instance = new Tutorial();
			return _Instance;
		}

		void reset(void)
		{
			Log::log(Log::ENGINE_INFO,("Tutorial : Reset tutorial from step "+toString(_CurrentStep)).c_str());
			for(int i=0;i<NB_STEPS;i++)
				_Steps[i] = false;
			_CurrentStep = 0;
			saveSteps();
		}

		void render2D(void)
		{
			if(!_Active || !_Render || _CurrentStep < 0)
				return;
			glPushMatrix();
			NYTexManager::getInstance()->drawTex2D(_Pos[_CurrentStep].X,_Pos[_CurrentStep].Y,1,1,0,0,*(_Texs[_CurrentStep]));
			glPopMatrix();
		}

		void update(float elapsed)
		{
			bool oldRender = _Render;
			_Render = false;
			if(_Active)
			{
				_DelayNextStep -= elapsed;
				if(_DelayNextStep <= 0 && _Steps[_CurrentStep] == false)
				{
					_Render = true;
				}
			}

			if(_Render != oldRender)
				if(_Render)
					Log::log(Log::ENGINE_INFO,("Tutorial : Rendering on at step "+toString(_CurrentStep)).c_str());
				else
					Log::log(Log::ENGINE_INFO,("Tutorial : Rendering off at step "+toString(_CurrentStep)).c_str());
		}

		void valideStep(int num)
		{
			if(_Steps[num] == false)
			{
				Log::log(Log::ENGINE_INFO,("Tutorial : Validate step "+toString(num)).c_str());

				_Steps[num] = true;
				saveSteps();

				int nextStep = findNextStep();

				if(nextStep == -1)
				{
					Log::log(Log::ENGINE_INFO,"Tutorial : Deactivate tutorial, no next step");
					_Active = false;
				}
				else
					if(_AutoActivate[nextStep])
					{
						_CurrentStep = nextStep;
						Log::log(Log::ENGINE_INFO,("Tutorial : Auto activate step "+toString(_CurrentStep)).c_str());
						_DelayNextStep = _StepDelay[_CurrentStep];
						_Render = false;
						Log::log(Log::ENGINE_INFO,("Tutorial : Rendering off at activation of step "+toString(_CurrentStep)).c_str());
						_Active = true;
					}
			}

		}

		void resetStep(int num)
		{
			Log::log(Log::ENGINE_INFO,("Tutorial : Reset step "+toString(num)).c_str());
			_Steps[num] = false;
		}

		void gotoStep(int num)
		{
			if(num < NB_STEPS && _CurrentStep != num)
			{
				Log::log(Log::ENGINE_INFO,("Tutorial : Goto step "+toString(num)).c_str());
				_CurrentStep = num;
			}

		}

		void resize(void)
		{
			for(int i=0;i<NB_STEPS;i++)
			{
				_Pos[i].X = NYRenderer::getInstance()->_ScreenWidth/2 - _Texs[i]->SizeX/2;
				_Pos[i].Y = NYRenderer::getInstance()->_ScreenHeight/3 - _Texs[i]->SizeY/2;
			}

			_Pos[1].Y = NYRenderer::getInstance()->_ScreenHeight/4 - _Texs[1]->SizeY/2;
			_Pos[2].Y = 60;
		}

		void activate(bool active)
		{
			if(_Active != active)
				if(active)
					Log::log(Log::ENGINE_INFO,("Tutorial : Activate at step "+toString(_CurrentStep)).c_str());
				else
					Log::log(Log::ENGINE_INFO,("Tutorial : Deactivate at step "+toString(_CurrentStep)).c_str());

			_Active = active;

			if(_CurrentStep == -1)
				_Active = false;
			else
				if(_AutoActivate[_CurrentStep] && active)
				{
					_DelayNextStep = _StepDelay[_CurrentStep];
					_Active = true;
				}
		}

		int getCurrentStep(void)
		{
			return _CurrentStep;
		}


};

#endif
