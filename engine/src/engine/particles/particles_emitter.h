#ifndef __PARTICLES_EMITTER__
#define __PARTICLES_EMITTER__

#include "engine/utils/types_3d.h"
#include "engine/render/renderer.h"

#define NB_MAX_PARTICLES_PER_EMITTER 300

class Particle
{
	public:
		NYVert3Df _Pos;
		NYVert3Df _Direction;
		float _Speed;
		float _Size;
		bool _Alive;
		NYColor _Color;

		Particle()
		{
			_Alive = false;
			_Size = 1.0f;
			_Color = NYColor((float)randf(),(float)randf(),(float)randf(),(float)randf());
		}
};

class ParticlesEmitter
{
	public:
		Particle _Particles[NB_MAX_PARTICLES_PER_EMITTER];
		bool _Emit;
		float _Frequency;
		float _SizeLossPerSec;
		NYVert3Df _Dir;
		Positionator _Transform;
		float _RndDir;
		float _Speed;
		float _SpeedLossPerSec;
		float _StartSize;
		NYColor _Color;
		bool _RandomColor;

	private:
		float _ElapsedEmit;

	public:

		ParticlesEmitter()
		{
			_Emit = false;
			_SizeLossPerSec = 1.0f;
			_Frequency = 5.0f;
			_Dir = NYVert3Df(0.0f,1.0f,0.0f);
			_RndDir = 1.0f;
			_Speed = 1.0f;
			_ElapsedEmit = 0.0f;
			_RandomColor = false;
			_SpeedLossPerSec = 0.0f;
		}

		void update(float elapsed)
		{
			if(_Emit)
			{
				_ElapsedEmit += elapsed;
				if (_ElapsedEmit >= 1.0f/_Frequency)
				{
					_ElapsedEmit -= 1.0f/_Frequency;
					for(int i=0;i<NB_MAX_PARTICLES_PER_EMITTER;i++)
					{
						if(!_Particles[i]._Alive)
						{
							_Particles[i]._Alive = true;
							_Particles[i]._Direction = _Transform.getWorldRot(_Dir) * (1.0f-_RndDir) + NYVert3Df((randf()*2.0f-1.0f)*_RndDir,(randf()*2.0f-1.0f)*_RndDir,(randf()*2.0f-1.0f)*_RndDir);
							_Particles[i]._Speed = _Speed+_Speed*((float)(rand()%10))/100.0f;
							_Particles[i]._Pos = this->_Transform.getWorldPos(NYVert3Df(0,0,0));
							_Particles[i]._Size = _StartSize;
							break;
						}
					}
				}
			}

			//On update les particles
			for(int i=0;i<NB_MAX_PARTICLES_PER_EMITTER;i++)
			{
				if(_Particles[i]._Alive)
				{
					_Particles[i]._Speed -= _SpeedLossPerSec * elapsed;
					if(_Particles[i]._Speed < 0)
						_Particles[i]._Speed = 0;
					_Particles[i]._Pos += _Particles[i]._Direction * _Particles[i]._Speed * elapsed;
					_Particles[i]._Size -= _SizeLossPerSec * elapsed;

					if(_Particles[i]._Size < 0.01)
						_Particles[i]._Alive = false;
				}
			}
		}

		void emit(bool emit)
		{
			if(!_Emit && emit)
				_ElapsedEmit = 1/_Frequency;
			_Emit = emit;

		}

		void explode(int nbParticules)
		{
			int nbExplode = 0;
			for(int i=0;i<NB_MAX_PARTICLES_PER_EMITTER;i++)
			{
				if(_Particles[i]._Alive == false)
				{
					_Particles[i]._Alive = true;
					_Particles[i]._Direction = NYVert3Df((float)randf()-0.5f,(float)randf()-0.5f,(float)randf()-0.5f).normalize();
					_Particles[i]._Speed = _Speed*0.7f+_Speed*(float)randf()*0.3f;
					_Particles[i]._Pos = this->_Transform.getWorldPos(NYVert3Df(0,0,0));
					_Particles[i]._Size = _StartSize;
					nbExplode++;
				}

				if(nbExplode >= nbParticules)
					break;
			}
		}

		void render(void)
		{
			glPushMatrix();
			glLoadIdentity();
			NYRenderer::getInstance()->viewFromCam();
			glDisable(GL_LIGHTING);
			glEnable(GL_COLOR_MATERIAL);

			if(!_RandomColor)
				glColor4f(_Color.R,_Color.V,_Color.B,_Color.A);

			for(int i=0;i<NB_MAX_PARTICLES_PER_EMITTER;i++)
			{

				if(_Particles[i]._Alive)
				{
					if(_RandomColor)
						glColor4f(_Particles[i]._Color.R,_Particles[i]._Color.V,_Particles[i]._Color.B,_Particles[i]._Color.A);

					glPushMatrix();
					//_cprintf("%f %f %f\n",_Particles[i]._Pos.X,_Particles[i]._Pos.Y,_Particles[i]._Pos.Z);
					glTranslatef(_Particles[i]._Pos.X,_Particles[i]._Pos.Y,_Particles[i]._Pos.Z);
					NYRenderer::getInstance()->drawBillBoard(_Particles[i]._Size);
					glPopMatrix();
					//_cprintf("render %d %0.2f\n",i,_Particles[i]._Size);
				}

			}
			glPopMatrix();
		}
};

#endif
