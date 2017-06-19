#ifndef __PROJECTILE__
#define __PROJECTILE__

#include "engine/utils/types_3d.h"
#include "engine/utils/types.h"
#include "engine/physics/physic_engine.h"
#include "engine/render/renderer.h"
#include "engine/particles/particles_emitter.h"

#include <vector>

class NYBodyProjectile : public NYBody{
	public:
		void collision(NYBody * other, btManifoldPoint& cp)
		{
		
			
		}
};

class Projectile
{
	public:
		NYVert3Df _Position;
		NYBodyProjectile * _Body;
		ParticlesEmitter * Emitter;
		float _Life;

		Projectile()
		{
			_Body = NULL;
			Emitter = new ParticlesEmitter();
			Emitter->_Color = NYColor(0.5f,0.5f,0.8f,0.5f);
			Emitter->_SizeLossPerSec = 0.1f*3;
			Emitter->_Speed = 0.0f;
			Emitter->_Frequency = 10.0f;
			Emitter->_StartSize = 0.1f*0.9f;
			_Life = 0.0f;
		}

		void launch(NYVert3Df pos, NYVert3Df speed)
		{
			NYPhysicEngine * engine = NYPhysicEngine::getInstance();
			_Position = pos;
			this->_Body = new NYBodyProjectile();
			engine->addSphereObject(true, 0.04f, _Position, this, OWNER_PROJECTILE, this->_Body);
			this->_Body->setSpeed(speed);
			this->_Body->activateCCD();
			this->_Body->activateCollisionReport(true);
			this->_Body->setMass(0.00001);
			Emitter->emit(true);
			_Life = 5.0f;
		}

		void destroy(void)
		{
			if(this->_Body)
				NYPhysicEngine::getInstance()->removeObject(this->_Body);
			this->_Body = NULL;
			Emitter->emit(false);
		}

		void render(void)
		{
			glPushMatrix();
			glTranslatef(_Position.X,_Position.Y,_Position.Z);
			glColor4f(0.2f,0.2f,0.7f,0.5f);
			NYRenderer::getInstance()->drawSolidSphere(0.05,5,5);
			glPopMatrix();
			Emitter->render();
		}

		void updateAfterPhysic(float elapsed)
		{
			if(this->_Body)
			{
				this->_Position = this->_Body->getBodyPosition();
				Emitter->_Transform._Pos = this->_Position;
			}

			Emitter->update(elapsed);
			_Life -= elapsed;
			if(_Life < 0)
			{
				destroy();
			}
		}
};

class ProjectilesManager
{
	private :
		ProjectilesManager()
		{
			//_Projectiles.reserve(1000);
		}

		static ProjectilesManager * _Instance;

		std::vector<Projectile*> _Projectiles;

	public:

		static ProjectilesManager * getInstance(void)
		{
			if(_Instance == NULL)
				_Instance = new ProjectilesManager();
			return _Instance;
		}

		void shoot(NYVert3Df start, NYVert3Df  speed, void * owner)
		{
			Projectile * projectile = NULL;
			for(unsigned int i=0;i<_Projectiles.size();i++)
			{
				if(_Projectiles[i]->_Life < 0)
				{
					projectile = _Projectiles[i];
					break;
				}
			}

			if(projectile == NULL)
			{
				projectile = new Projectile();
				_Projectiles.push_back(projectile);
			}

			projectile->launch(start,speed);



		}

		void render(void)
		{
			for(unsigned int i=0;i<_Projectiles.size();i++)
				if(_Projectiles[i]->_Life >= 0)
					_Projectiles[i]->render();
		}

		void updateAfterPhysic(float elapsed)
		{
			std::vector<Projectile*>::iterator it = _Projectiles.begin();
			while(it != _Projectiles.end())
			{
				if((*it)->_Life >= 0)
					(*it)->updateAfterPhysic(elapsed);
				/*if ((*it)->_Life < 0)
				{
					//delete(*it);
					//it = _Projectiles.erase(it);
				}
				else*/
					it++;
			}
		}
};

#endif
