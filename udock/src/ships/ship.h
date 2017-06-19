#ifndef __SHIP__
#define __SHIP__

#include "engine/utils/types_3d.h"
#include "engine/utils/types.h"
#include "engine/physics/physic_engine.h"
#include "engine/particles/particles_emitter.h"
#include "projectile.h"

typedef enum {
	SHIP_PART_NEXUS = 0, //On en a qu'un, il définit notre vaisseau
	SHIP_PART_BASE,
	SHIP_PART_ENGINE,
	SHIP_PART_SHOOTER,
	NB_SHIP_PARTS_TYPES
} SHIP_PART_TYPE;

#define SHIP_PART_SIZE 0.1f

class Ship;

class ShipPart
{
	protected :
		bool _Active;
		float _ActiveParam;

	public :
		SHIP_PART_TYPE _Type;
		Positionator _Transform;
		bool _Added;
		int _NbAttach;
		char _Key;
		bool _HighLighted;
		Ship * _ShipOwner;

		ShipPart* _AttachedTo[8];

		ShipPart()
		{
			_Added = false;
			memset(_AttachedTo,0x00,sizeof(ShipPart*)*8);
			_NbAttach = 0;
			_Key = 0;
			_HighLighted = false;
			_Active = false;
			_ShipOwner = NULL;
		}

		virtual void save(FILE * fs)
		{
            int type = _Type;
			fwrite(&type,sizeof(int),1,fs);
			fwrite(&_Added,sizeof(bool),1,fs);
			fwrite(&_NbAttach,sizeof(int),1,fs);
			fwrite(&_Key,sizeof(char),1,fs);
			_Transform.writeBinary(fs);
		}

		virtual void load(FILE * fe)
		{
            bool error = true;
            int type = 0;
			if(fread(&type,sizeof(int),1,fe) == 1)
			if(fread(&_Added,sizeof(bool),1,fe) == 1)
			if(fread(&_NbAttach,sizeof(int),1,fe) == 1)
			if(fread(&_Key,sizeof(char),1,fe) == 1)
			if(_Transform.readBinary(fe))
                error = false;

            _Type = (SHIP_PART_TYPE)type;

            if(error)
                Log::log(Log::USER_ERROR,"Corrupted ship file");

			_Transform._Parent = NULL;
			transformUpdated();
		}

		void attach(ShipPart* part)
		{
			for(int i=0;i<_NbAttach;i++)
			{
				if(_AttachedTo[i] == part)
					return;
			}

			if(_NbAttach < 8)
			{
				_AttachedTo[_NbAttach] = part;
				_NbAttach++;
			}
			else
			{
				Log::log(Log::ENGINE_ERROR,"No more room to attach ship part");
			}
		}

		void detach(ShipPart* part)
		{
			for(int i=0;i<_NbAttach;i++)
			{
				if(_AttachedTo[i] == part)
				{
					_AttachedTo[i] = NULL;
					if( i < _NbAttach-1)
						_AttachedTo[i] = _AttachedTo[_NbAttach-1];
					_NbAttach--;
					return;
				}
			}
		}

		//A appeler a chaque fois qu'on modifie la position
		virtual void transformUpdated(void)
		{
		}

		virtual void addToShipBody(NYCompoundShape * shipCollider, NYPhysicEngine * engine)
		{
			engine->addCompoundObject(true,shipCollider,_Transform._Pos,this,OWNER_SHIP);
		}

		virtual void render(void)
		{
			glEnable(GL_LIGHTING);
			glEnable(GL_COLOR_MATERIAL);
			glPushMatrix();
			glTranslatef(_Transform._Pos.X,_Transform._Pos.Y,_Transform._Pos.Z);
			glMultTransposeMatrixf(_Transform._Rot.Mat.t);
			NYRenderer::getInstance()->drawSolidCube(SHIP_PART_SIZE/2.0f);
			glTranslatef(0.0f,SHIP_PART_SIZE/2.0f,0.0f);
			glColor3f(0.0f,0.0f,0.0f);
			NYRenderer::getInstance()->drawSolidCube(SHIP_PART_SIZE/5.0f);
			glPopMatrix();
		}

		virtual void update(float elapsed) = 0;

		virtual void activate(bool activate, float param) { _Active = activate; _ActiveParam = param; };

		NYVert3Df getDir(void)
		{
			NYVert3Df res(0,1,0);
			res = _Transform._Rot * res;
			return res;
		}

		bool isActive(void)
		{
			return _Active;
		}
};

#define SHIP_PART_ENGINE_POWER 2.0f

class ShipEngine : public ShipPart
{
	public:
		float _Power;

		ParticlesEmitter _Emitter;

		ShipEngine()
		{
			this->_Type = SHIP_PART_ENGINE;
			this->_Power = SHIP_PART_ENGINE_POWER;
			this->_Active = false;
			_Emitter._SizeLossPerSec = SHIP_PART_SIZE*3;
			_Emitter._Speed = 0.0f;
			_Emitter._Frequency = 30.0f;
			_Emitter._StartSize = SHIP_PART_SIZE*0.9f;
			_Emitter._Color = NYColor(1.0f,0.0f,0.0f,0.8f);
		}

		virtual void save(FILE * fs)
		{
			ShipPart::save(fs);
			fwrite(&_Power,sizeof(float),1,fs);
		}

		virtual void load(FILE * fe)
		{
			ShipPart::load(fe);
			if(fread(&_Power,sizeof(float),1,fe) != 1)
                Log::log(Log::USER_ERROR,"Corrupted ship part in ship file");
		}

		void render(void)
		{
			glColor3f(1.0f,0.5f,0.5f);
			if(_HighLighted)
				glColor3f(1.0f,0.7f,0.7f);
			ShipPart::render();
			glColor3f(1.0f,0.3f,0.3f);
			_Emitter.render();
		}

		void activate(bool active, float param)
		{
			_Active = active;
			_ActiveParam = param;
			_Emitter.emit(active);
			//_cprintf("Activate %d\n",active);
		}

		void update(float elapsed)
		{
			_Emitter.update(elapsed);
		}

		NYVert3Df getForce(void)
		{
			return getDir() * _Power * -1 * _ActiveParam;
		}

		//A appeler a chaque fois qu'on modifie la position
		void transformUpdated(void)
		{
			_Emitter._Transform._Parent = _Transform._Parent;
			_Emitter._Dir = getDir().normalize();
			_Emitter._Transform._Pos = _Transform._Pos + _Emitter._Dir * SHIP_PART_SIZE/2.0f;
			_Emitter._Speed = SHIP_PART_SIZE * _Power;
		}
};

#define SHOOT_PERIOD 0.5f

class ShipShooter : public ShipPart
{
	public:
		NYVert3Df _DirWhenShot;
		float _TimeoutShoot;

		ShipShooter()
		{
			this->_Type = SHIP_PART_SHOOTER;
			_TimeoutShoot = SHOOT_PERIOD;
		}

		void update(float elapsed)
		{
			_TimeoutShoot -= elapsed;
		}

		void render(void)
		{
			glColor3f(0.5f,0.5f,1.0f);
			if(_HighLighted)
				glColor3f(0.7f,0.7f,1.0f);
			ShipPart::render();
		}

		void activate(bool activate, float param)
		{
			_Active = activate;
			_ActiveParam = param;
			if(activate && _TimeoutShoot < 0.0f)
			{
				_TimeoutShoot = SHOOT_PERIOD;
				_DirWhenShot = getDir();
				NYVert3Df startPos = this->_Transform._Pos + _DirWhenShot;
				if(_Transform._Parent)
				{
					_DirWhenShot = _Transform._Parent->getWorldRot(_DirWhenShot);
					startPos = _Transform._Parent->getWorldPos(startPos);
				}

				startPos += _DirWhenShot * SHIP_PART_SIZE * 5.0f;

				ProjectilesManager::getInstance()->shoot(startPos, _DirWhenShot * 40.0f, this->_ShipOwner);
			}
		}

		void transformUpdated(void)
		{
		}
};

class ShipBase : public ShipPart
{
	public:
		ShipBase()
		{
			this->_Type = SHIP_PART_BASE;
		}

		void render(void)
		{
			glColor3f(1.0f,1.0f,1.0f);
			if(_HighLighted)
				glColor3f(0.7f,0.7f,0.7f);
			ShipPart::render();
		}

		void update(float elapsed)
		{

		}
};


class ShipNexus : public ShipPart
{
	public:
		ShipNexus()
		{
			this->_Type = SHIP_PART_NEXUS;
		}

		void render(void)
		{
			glColor3f(0.0f,0.0f,0.0f);
			if(_HighLighted)
				glColor3f(0.1f,0.1f,0.1f);
			ShipPart::render();
		}

		void update(float elapsed)
		{

		}
};

class ShipPartTemp : public ShipPart
{
	public:
		ShipPartTemp()
		{

		}

		void setType(SHIP_PART_TYPE type)
		{
			_Type = type;
		}

		void render(void)
		{
			switch(_Type)
			{
				case SHIP_PART_BASE: glColor4f(1.0f,1.0f,1.0f,0.5f); break;
				case SHIP_PART_ENGINE: glColor4f(1.0f,0.5f,0.5f,0.5f); break;
				case SHIP_PART_SHOOTER: glColor4f(0.5f,0.5f,1.0f,0.5f); break;
			}
			ShipPart::render();
		}

		void update(float elapsed)
		{

		}
};

class Ship
{
	public:
		Positionator _Transform;
		std::vector<ShipPart*> _Parts;
		NYCompoundShape * _Collider;
		NYBody * _Body;
		NYVert3Df _CenterOfMass;
		float _DampFactor;
		float _LinDampFactor;
		float _AngDampFactor;
		NYVert3Df _Dir;
		NYVert3Df _Up;
		bool _Moving;
		bool _OwnedByPlayer;
		ShipPartTemp _TemporaryPart;
		bool _Visible;

		Ship()
		{
			_Collider = NULL;
			_Body = NULL;
			_LinDampFactor = 1.7f;
			_AngDampFactor = 1.7f;
			_Dir = NYVert3Df(0.0f,1.0f,0.0f);
			_Moving = false;
			_OwnedByPlayer = false;
			_Visible = true;
		}

		void erase(void)
		{
			for(unsigned int i=0;i<_Parts.size();i++)
				delete _Parts[i];
			_Parts.clear();

		}

		void setPosition(NYVert3Df pos)
		{
			this->_Transform._Pos = pos;
			if(this->_Body)
				this->_Body->setBodyPosition(pos);
		}


		void setRotation(NYVert4Df axisAndAngle)
		{
			_Transform._Rot.createRotateAxe(NYVert3Df(axisAndAngle.X, axisAndAngle.Y, axisAndAngle.Z), axisAndAngle.T);
			if(this->_Body)
				this->_Body->setBodyRotation(NYVert4Df(axisAndAngle.X, axisAndAngle.Y, axisAndAngle.Z, axisAndAngle.T));
		}

		void setRotation(NYVert3Df dir, NYVert3Df up)
		{
			_Transform._Rot.createRotateLookatMatrix(dir, up);
			NYQuaternion quat;
			quat.initFromMat(_Transform._Rot);
			if (this->_Body)
				this->_Body->setBodyRotation(NYVert4Df(quat.X, quat.Y, quat.Z, quat.T));
		}

		void addToPhysicEngine(NYPhysicEngine * engine)
		{
			if(_Body)
				engine->removeObject(_Body);

			NYCompoundShape * shape = engine->createCompoundShape();

			//On garde l'ancien centre de gravite pour prendre en compte le décalage si modification de la forme
			//NYVert3Df oldCenterOfMass = _CenterOfMass;

			//On calcule le centre de gravite
			_CenterOfMass = NYVert3Df();
			for(unsigned int i=0;i<_Parts.size();i++)
			{
				_CenterOfMass += _Parts[i]->_Transform._Pos;
			}
			_CenterOfMass /= (float)(_Parts.size());

			//Mise a jour position des parties
			for(unsigned int i=0;i<_Parts.size();i++)
			{
				_Parts[i]->_Transform._Pos -= _CenterOfMass;
				_Parts[i]->transformUpdated();
			}

			for(unsigned int i=0;i<_Parts.size();i++)
			{
				NYQuaternion quat;
				quat.initFromMat(_Parts[i]->_Transform._Rot);
				engine->addBoxShapeToCompound(shape,_Parts[i]->_Transform._Pos,quat,NYVert3Df(SHIP_PART_SIZE/2,SHIP_PART_SIZE/2,SHIP_PART_SIZE/2),_Parts[i]);
			}

			//Decalage de la position en fonction de décalage des centre de masses
			_Transform._Pos += _CenterOfMass;

			_Body = engine->addCompoundObject(true,shape,_Transform._Pos,this,OWNER_SHIP);
			_Body->activateCCD();
			NYQuaternion quat;
			quat.initFromMat(this->_Transform._Rot);
			_Body->setBodyRotation(NYVert4Df(quat.X,quat.Y,quat.Z,quat.T));
			updateAfterPhysic(0);


		}

		void removeFromPhysic(NYPhysicEngine * engine)
		{
			if(_Body)
			{
				engine->removeObject(_Body);
				delete _Body;
				_Body = NULL;
			}
		}

		ShipPart * addShipPart(SHIP_PART_TYPE type, Positionator & transform)
		{
			ShipPart * part = NULL;
			switch(type)
			{
				case SHIP_PART_NEXUS :
					part = new ShipNexus();
					break;
				case SHIP_PART_BASE :
					part = new ShipBase();
					break;
				case SHIP_PART_ENGINE :
					part = new ShipEngine();
					break;
				case SHIP_PART_SHOOTER :
					part = new ShipShooter();
					break;
			}

			if(part)
			{
				part->_Transform = transform;
				part->_Transform._Parent = &(this->_Transform);
				part->transformUpdated();
				part->_ShipOwner = this;
				_Parts.push_back(part);

			}

			return part;
		}

		void deleteShipPart(ShipPart * part)
		{
			for(unsigned int i=0;i<_Parts.size();i++)
			{
				if(_Parts[i] == part)
				{
					_Parts.erase(_Parts.begin()+i);
					break;
				}
			}
		}

		void render(void)
		{
			if(!_Visible)
				return;
			glPushMatrix();
			glTranslatef(_Transform._Pos.X,_Transform._Pos.Y,_Transform._Pos.Z);
			glMultTransposeMatrixf(_Transform._Rot.Mat.t);
			for(unsigned int i=0;i<_Parts.size();i++)
				_Parts[i]->render();
			if(_TemporaryPart.isActive())
				_TemporaryPart.render();
			glPopMatrix();
		}

		void updateBeforePhysic(float elapsed)
		{
			for(unsigned int i=0;i<_Parts.size();i++)
				_Parts[i]->update(elapsed);

			//On applique les moteurs
			for(unsigned int i=0;i<_Parts.size();i++)
			{
				if(_Parts[i]->_Type == SHIP_PART_ENGINE)
				{
					ShipEngine * engine = (ShipEngine*)(_Parts[i]);

					if(engine->isActive())
					{
						NYVert3Df force = engine->getForce();
						force = this->_Transform._Rot * force;
						NYVert3Df relPos = engine->_Transform.getWorldPos(NYVert3Df()) - this->_Transform.getWorldPos(NYVert3Df());
						_Body->applyForce(force,relPos);
					}
				}
			}

			//On applique les forces contraires
			if(this->_Body)
			{
				NYVert3Df angSpeed = this->_Body->getAngularSpeed();
				NYVert3Df dampingAng = angSpeed;
				dampingAng = dampingAng * 0.2f;
				dampingAng *= -1;
				this->_Body->applyTorque(dampingAng);
				/*
				if(dampingAng.getMagnitude() < angSpeed.getMagnitude())
					angSpeed -= dampingAng;
				else
					angSpeed = NYVert3Df();

				if(angSpeed.getMagnitude() > 4.0f)
				{
					angSpeed.normalize();
					angSpeed *= 2.0f;
				}
				this->_Body->setAngularSpeed(angSpeed);*/


				NYVert3Df linSpeed = this->_Body->getSpeed();
				NYVert3Df dampingLin = linSpeed;
				dampingLin = dampingLin * 1.5;
				dampingLin *= -1;
				_Body->applyForce(dampingLin);


				/*this->_Body->setSpeed(linSpeed);*/


				_Moving = false;
				if(linSpeed.getMagnitude() > 0 || angSpeed.getMagnitude() > 0)
				{
					//_Dir = linSpeed.normalize();
					_Dir = _Transform._Rot * NYVert3Df(0, 1, 0);
					_Moving = true;
				}
			}
		}

		void updateAfterPhysic(float elapsed)
		{

			if(this->_Body)
			{
				_Transform._Pos = this->_Body->getBodyPosition();
				NYVert4Df rot = this->_Body->getBodyRotation();
				_Transform._Rot.createRotateAxe(NYVert3Df(rot.X,rot.Y,rot.Z), -rot.T);
				_Up = _Transform.getWorldRot(NYVert3Df(0,0,1));
				//_cprintf("%f %f %f\n",_Transform._Pos.X,_Transform._Pos.Y,_Transform._Pos.Z);
			}
		}

		void saveShip(const char * filename)
		{
			FILE * fs = fopen(filename,"wb");
			int size = _Parts.size();
			fwrite(&size,sizeof(int),1,fs);
			for(unsigned int i=0;i<_Parts.size();i++)
			{
                int type = _Parts[i]->_Type;
				fwrite(&type,sizeof(int),1,fs);
				_Parts[i]->save(fs);
			}
			fclose(fs);
		}

		void loadShip(const char * filename)
		{
            bool error = false;
			FILE * fe = fopen(filename,"rb");
			if(fe)
			{
				this->erase();
				int nbParts;
				if(fread(&nbParts,sizeof(int),1,fe) != 1)
                    error = true;

				for(int i=0;i<nbParts;i++)
				{
					int type;
					if(fread(&type,sizeof(int),1,fe) != 1)
                        error = true;

					Positionator pos;
					ShipPart * part = this->addShipPart((SHIP_PART_TYPE)type,pos);
					part->load(fe);
					part->_Transform._Parent = &(this->_Transform);
					part->transformUpdated();
				}
				fclose(fe);
			}

			if(error)
                Log::log(Log::USER_ERROR,"Ship loading failed, corrupted file");

		}

		void activateTemporaryPart(bool activate, float paramAct, Positionator transform, SHIP_PART_TYPE type)
		{
			_TemporaryPart.activate(activate, paramAct);
			_TemporaryPart._Transform = transform;
			_TemporaryPart.setType(type);
		}

		void dehighlightAllParts(void)
		{
			for(unsigned int i=0;i<_Parts.size();i++)
				_Parts[i]->_HighLighted = false;
		}



};


#endif
