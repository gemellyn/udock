#ifndef __GAME_STATE__
#define __GAME_STATE__

#include "engine/gui/screen.h"
#include "engine/render/renderer.h"
#include "engine/physics/physic_engine.h"

class GameState
{
	public:

		NYRenderer * _Renderer;
		NYPhysicEngine * _PhysicEngine;

		bool _Active;

		virtual bool activate(bool active){_Active = active; return active;};
		virtual void preState(float elapsed){}; //Avant update physique
		virtual void postState(float elapsed){}; //Apres update physique
		virtual void addToGui(int & x, int & y,GUIScreen * screen){};
		virtual void start(void){}; //appel au lancement du jeu
		virtual void specialDownFunction(int key, int p1, int p2){};
		virtual void specialUpFunction(int key, int p1, int p2){};
		virtual void keyboardDownFunction(unsigned char key, int p1, int p2){};
		virtual void keyboardUpFunction(unsigned char key, int p1, int p2){};
		virtual void mouseWheelFunction(int wheel, int dir, int x, int y){};
		virtual void mouseFunction(int button, bool down, int x, int y){};
		virtual void mousePollFunction(float elapsed){};
		virtual void mouseMoveFunction(int buttonsState, int x, int y){};

		virtual void init(void)
		{
			 _Renderer = NYRenderer::getInstance();
			 _PhysicEngine = NYPhysicEngine::getInstance();
		}

		GameState()
		{
			_Active = false;
		}

	public:
		NYBody * getPointedBody(NYVert3Df * hitPoint)
		{
			/*POINT p;
			GetCursorPos(&p);
			BOOL res = ScreenToClient(_Renderer->_WHnd,&p);

			int x = p.x;
			int y = p.y;*/

			int x = 0;
			int y = 0;

			SDL_GetMouseState(&x,&y);

			//Selection
			NYVert3Df from = _Renderer->_Camera->_Position;

			double posX,posY,posZ;
			_Renderer->unProjectMousePos(x,y,1.0,&posX,&posY,&posZ);

			NYVert3Df to ((float)posX,(float)posY,(float)posZ);

			NYBody * body = _PhysicEngine->rayPickup(from,to,hitPoint);

			return body;
		}

		NYBody * getPointedBody(NYVert3Df pos, NYVert3Df dir, NYVert3Df * hitPoint)
		{
			NYVert3Df from = pos;
			NYVert3Df to =  pos + dir * 1000;

			NYBody * body = _PhysicEngine->rayPickup(from, to, hitPoint);

			return body;
		}
};

#endif

