#ifndef __SHIP_STATE__
#define __SHIP_STATE__

#include "game_states/game_state.h"
#include "ships/ships_manager.h"
#include "engine/gui/screen.h"
#include "engine/gui/screen_manager.h"
#include "molecules/molecules_manager.h"
#include "engine/render/camera_animated.h"
#include "log/log_actions.h"
#include "engine/sound/sound.h"
#include "tutorial/tutorial.h"

class ShipState : public GameState
{
	public:

		GUIScreenManager * _ScreenManager;
		GUIScreen * _Screen;

		ShipManager * g_ship_manager;
		MoleculesManager * _MoleculesManager;
		Ship * MyShip;
		ShipPart * g_selected_part;
		NYVert3Df g_relative_pos_local;
		float g_recul;
		bool g_rotate_cam;
		SHIP_PART_TYPE _LastUsedType;
		Ship * _HittedShip;
		bool g_will_add_part;
		bool g_add_part;
		Molecule * g_near_mol;
		float g_time_near_mol;
		GUIPBar * _PbarAcquire;
		GUIBouton * _BtnFreeze;
		Molecule * _SelectedMol;
		SoundBasic ** _SoundsAttach;
		int _NbSoundAttach;
		SoundBasic * _SoundDetach;

		ShipState() : GameState()
		{
			g_recul = 3.0f;
			g_rotate_cam = false;
			g_selected_part = NULL;
			_MoleculesManager = NULL;
			g_will_add_part = false;
			_HittedShip =  NULL;
			_LastUsedType = SHIP_PART_BASE;
			g_add_part = false;
			g_near_mol = NULL;
			g_time_near_mol = 0.0f;
			_PbarAcquire = NULL;
			g_relative_pos_local = NYVert3Df(0,-3,0.5);
			_SelectedMol = NULL;
			_SoundsAttach = NULL;
			_NbSoundAttach = 0;
			_SoundDetach = NULL;
		}

		void getHittedShip(int x, int y, Ship ** hitShip, ShipPart ** part, NYVert3Df * hitPoint)
		{
			//On lance un rayon en mode extraction de shape du body du ship
			NYVert3Df from = _Renderer->_Camera->_Position;
			double posX,posY,posZ;
			_Renderer->unProjectMousePos(x,y,1.0,&posX,&posY,&posZ);

			NYVert3Df to ((float)posX,(float)posY,(float)posZ);

			NYVert3Df hitPointTmp;
			float minDistance = 0.0;
			float distance;
			NYBody * body = NULL;
			void * shapeOwner = NULL;
			void * shapeOwnerTmp = NULL;
			NYBody * bodyTmp = _PhysicEngine->multipleRayPickup(from,to,&hitPointTmp,&distance,&shapeOwnerTmp);
			Molecule * hitMol = NULL;

			//On cherche le hit de ship le plus proche
			while(bodyTmp)
			{
				if(bodyTmp->BodyOwnerType == OWNER_SHIP)
				{
					if(distance < minDistance || minDistance == 0.0f)
					{
						*hitPoint = hitPointTmp;
						minDistance = distance,
						body = bodyTmp;
						shapeOwner = shapeOwnerTmp;
					}
				}
				bodyTmp = _PhysicEngine->getNextHit(&hitPointTmp,&distance,&shapeOwnerTmp);
			}

			//Si on le trouve
			if(body)
			{
				*hitShip = (Ship*)(body->BodyOwner);

				//On recup la position du ship part
				*part = (ShipPart*) shapeOwner;
			}
		}

		//
		void getNewPartPosFromHitPoint(Ship * hitShip, ShipPart * part, NYVert3Df hitPoint, Positionator * transform)
		{
			//On se place au bon endroit
			NYVert3Df localHitPoint = hitShip->_Transform.invTransform(hitPoint);
			NYVert3Df delta = localHitPoint - part->_Transform._Pos;

			//On cree le bon decalage
			if(abs(delta.X) > abs(delta.Y) && abs(delta.X) > abs(delta.Z))
				delta = NYVert3Df(delta.X > 0 ? SHIP_PART_SIZE : -SHIP_PART_SIZE,0,0);
			else if(abs(delta.Y) > abs(delta.X) && abs(delta.Y) > abs(delta.Z))
				delta = NYVert3Df(0,delta.Y > 0 ? SHIP_PART_SIZE : -SHIP_PART_SIZE,0);
			else if(abs(delta.Z) > abs(delta.X) && abs(delta.Z) > abs(delta.Y))
				delta = NYVert3Df(0,0,delta.Z > 0 ? SHIP_PART_SIZE : -SHIP_PART_SIZE);

			Positionator pos;
			pos.Translate(part->_Transform._Pos + delta);

			//Delta cam pour la direction
			NYVert3Df localCam = hitShip->_Transform.invTransform(_Renderer->_Camera->_Position);
			NYVert3Df deltaCam = localHitPoint - localCam;

			//Par defaut les part pointent vers la cam
			if(abs(deltaCam.X) > abs(deltaCam.Y) && abs(deltaCam.X) > abs(deltaCam.Z))
				pos._Rot.createRotateZ((float)(deltaCam.X > 0 ? M_PI/2 : -M_PI/2));
			else if(abs(deltaCam.Y) > abs(deltaCam.X) && abs(deltaCam.Y) > abs(deltaCam.Z))
				pos._Rot.createRotateZ((float)(deltaCam.Y > 0 ? M_PI : 0));
			else if(abs(deltaCam.Z) > abs(deltaCam.X) && abs(deltaCam.Z) > abs(deltaCam.Y))
				pos._Rot.createRotateX((float)(deltaCam.Z > 0 ? - M_PI/2 : M_PI/2));

			*transform = pos;
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

			//Le bon ecran
			if(active)
				_ScreenManager->setActiveScreen(_Screen);

			if(active)
				LogActions::getInstance()->logStartFlying();

			if(active)
			{
				MyShip->setPosition(_Renderer->_Camera->_Position);
				MyShip->setRotation(_Renderer->_Camera->_Direction, _Renderer->_Camera->_UpVec);
				g_relative_pos_local = NYVert3Df(0, -3, 0.5);
				g_recul = 3.0f;
				
				/*bool frozen = false;
				if(_MoleculesManager->_Molecules[0]->_Body)
					frozen = _MoleculesManager->_Molecules[0]->_Body->Frozen;
				_MoleculesManager->removeMolsFromPhysicEngine(NYPhysicEngine::getInstance());
				_MoleculesManager->addMolsToPhysicEngine(NYPhysicEngine::getInstance());
				_MoleculesManager->hideAllMols(false);
				freezeMols(frozen);*/
			}

			g_ship_manager->hideAllShips(!active);

			return active;
		}

		void preState(float elapsed)
		{
			if(MyShip)
			{
				//Si on est pas proche d'une mol
				if(!g_near_mol)
				{
					//On regarde si on est proche d'une molecule
					for(unsigned int i=0;i<_MoleculesManager->_Molecules.size();i++)
					{
 						NYVert3Df dist = _MoleculesManager->_Molecules[i]->_Transform._Pos - MyShip->_Transform._Pos;
						float distCarre = dist.getMagnitude();
						float radCarre =  (_MoleculesManager->_Molecules[i]->_Radius *  _MoleculesManager->_Molecules[i]->_Radius);
						distCarre -= radCarre;
						if(distCarre < 100.0f)
						{
							g_near_mol = _MoleculesManager->_Molecules[i];
						}
					}
				}

				//Si on est en train d'acquerir une molecule
				/*if(g_near_mol)
				{
					//On chope la distance
					NYVert3Df dist = g_near_mol->_Transform._Pos- MyShip->_Transform._Pos;
					float distCarre = dist.getMagnitude() -  (g_near_mol->_Radius * g_near_mol->_Radius);
					if(distCarre >= 100.0f)
					{
						g_near_mol = NULL;
						g_time_near_mol = 0.0f;
						_PbarAcquire->Visible = false;
					}
					else
					{
						_PbarAcquire->Visible = true;
						g_time_near_mol += elapsed;
						_PbarAcquire->setValue(g_time_near_mol/15.f);
						if(g_time_near_mol >= 15.0f)
						{
							//On ajoute la molecule
							_PbarAcquire->Visible = false;
							g_near_mol = NULL;
							g_time_near_mol = 0.0f;
						}
					}

				}*/
			}
		}

		void postState(float elapsed)
		{
			if(MyShip)
			{
				_Renderer->_Camera->setUpRef(MyShip->_Up);

				if(MyShip->_Moving && !g_rotate_cam)
					((NYCameraAnimated*)(_Renderer->_Camera))->startAnimTo(MyShip->_Transform.Transform(g_relative_pos_local),MyShip->_Up,MyShip->_Transform._Pos,0.05f);
				else
				{
					if(g_rotate_cam)
					{
						g_relative_pos_local = MyShip->_Transform._Pos - _Renderer->_Camera->_Direction * g_recul;
						g_relative_pos_local = MyShip->_Transform.invTransform(g_relative_pos_local);
					}
					((NYCameraAnimated*)(_Renderer->_Camera))->startAnimTo( MyShip->_Transform.Transform(g_relative_pos_local),MyShip->_Up,MyShip->_Transform._Pos,MyShip->_Moving ? 0.05f : 0.3f);
				}
			}
		}

		

		//INPUT
		void specialDownFunction(int key, int p1, int p2)
		{
			ShipPart * hitPart = NULL;
			Ship * hitShip = NULL;
			NYVert3Df hitPoint;

			const Uint8 *keybState = SDL_GetKeyboardState(NULL);

			if(keybState[SDL_SCANCODE_LSHIFT])
			{
				/*POINT p;
				GetCursorPos(&p);
				BOOL res = ScreenToClient(_Renderer->_WHnd,&p);

				int x = p.x;
				int y = p.y;*/

				int x = 0;
                int y = 0;

                SDL_GetMouseState(&x,&y);

				getHittedShip(x,y,&hitShip,&hitPart,&hitPoint);

				if(hitShip)
				{
					g_ship_manager->dehighlightAllParts();
					g_selected_part = hitPart;
					if(g_selected_part)
						g_selected_part->_HighLighted = true;
				}
				else
				{
					g_ship_manager->desactivateAllTempParts();
					g_ship_manager->dehighlightAllParts();
					g_selected_part = NULL;
				}
			}
		}

		void specialUpFunction(int key, int p1, int p2)
		{
			const Uint8 *keybState = SDL_GetKeyboardState(NULL);

			if(keybState[SDL_SCANCODE_LSHIFT])
			{
				g_ship_manager->desactivateAllTempParts();
				g_ship_manager->dehighlightAllParts();
				g_selected_part = NULL;
				g_add_part = false;
			}
		}

		void activateShipPartOnKey(bool activate, unsigned char key, float param)
		{
			for (unsigned int i = 0; i<g_ship_manager->Ships.size(); i++)
			{
				if (g_ship_manager->Ships[i]->_OwnedByPlayer)
				{
					for (unsigned int j = 0; j<g_ship_manager->Ships[i]->_Parts.size(); j++)
						if (g_ship_manager->Ships[i]->_Parts[j]->_Key == key)
							g_ship_manager->Ships[i]->_Parts[j]->activate(activate, param);
				}
			}
		}

		void keyboardDownFunction(unsigned char key, int p1, int p2)
		{
			if(key >= SDL_SCANCODE_A && key <= SDL_SCANCODE_Z )
			{
				if(g_selected_part && MyShip)
				{
					g_selected_part->_Key = key;
					MyShip->saveShip("myShip.bin");
					g_selected_part->_HighLighted = false;
					g_selected_part = NULL;
				}

				activateShipPartOnKey(true, key, 1.0);
			}
		}

		void keyboardUpFunction(unsigned char key, int p1, int p2)
		{
			activateShipPartOnKey(false, key, 1.0);
		}

		void mouseWheelFunction(int wheel, int dir, int x, int y)
		{
			if(g_selected_part)
			{
				SHIP_PART_TYPE type = g_selected_part->_Type;
				SHIP_PART_TYPE newType = (SHIP_PART_TYPE)(type+dir);
				if(newType < SHIP_PART_BASE)
					newType = (SHIP_PART_TYPE)(NB_SHIP_PARTS_TYPES-1);
				if(newType >= NB_SHIP_PARTS_TYPES)
					newType = SHIP_PART_BASE;
				_LastUsedType = newType;
				Positionator transform = g_selected_part->_Transform;
				Ship * ship = g_selected_part->_ShipOwner;
				ship->deleteShipPart(g_selected_part);
				g_selected_part = ship->addShipPart(newType,transform);
				ship->addToPhysicEngine(_PhysicEngine);
			}
			else
			{
				g_recul -= SHIP_PART_SIZE * 3.0f * dir;
				g_relative_pos_local = MyShip->_Transform._Pos-_Renderer->_Camera->_Direction*g_recul;
				g_relative_pos_local = MyShip->_Transform.invTransform(g_relative_pos_local);
			}
		}

		void mouseFunction(int button, bool down, int x, int y)
		{
			NYVert3Df hitPoint;
			ShipPart * hitPart = NULL;

			//Clavier
			const Uint8 *keybState = SDL_GetKeyboardState(NULL);

			//Gestion de la selection
			_HittedShip = NULL;
			getHittedShip(x,y,&_HittedShip,&hitPart,&hitPoint);

			//Gestion de la selection
			if (button == SDL_BUTTON_LEFT || button == SDL_BUTTON_RIGHT)
			{
				if (down)
				{
					_SelectedMol = NULL;
					NYBody * body = getPointedBody(&hitPoint);
					if (body)
					{
						if (body->BodyOwnerType == OWNER_MOLECULE)
							_SelectedMol = (Molecule*)(body->BodyOwner);
					}
				}
			}

			//Ajout ou suppression
			if ((button == SDL_BUTTON_LEFT || button == SDL_BUTTON_RIGHT) && down && (keybState[SDL_SCANCODE_LCTRL]))
			{
				if (_SelectedMol)
				{
					/*NYVert3Df from = _Renderer->_Camera->_Position;
					double posX, posY, posZ;
					_Renderer->unProjectMousePos(x, y, 1.0, &posX, &posY, &posZ);

					NYVert3Df to((float)posX, (float)posY, (float)posZ);*/

					NYVert3Df relpos = hitPoint - _SelectedMol->_Transform._Pos;
					NYFloatMatrix rot = _SelectedMol->_Transform._Rot;
					rot.invert();
					relpos = rot * relpos;

					//Si c'est un ajout
					if (button == SDL_BUTTON_LEFT)
					{
						bool reset = true;
						for (int i = 0; i<NB_DOCKING_POINTS; i++)
						{
							if (!_SelectedMol->_DockingPoints[i]._IsSet)
							{
								_SelectedMol->setDockingPoint(i, relpos, true);
								reset = false;
								_SoundsAttach[rand() % _NbSoundAttach]->play();
								LogActions::getInstance()->logSetHarpoonHandle(_SelectedMol->_Id, relpos);
								break;
							}
						}

						//Si on a un harpon bien placé des deux cotés, on valide l'étape du tuto
						int nbDockingPointsActive = 0;
						for (int i = 0; i<NB_DOCKING_POINTS; i++)
						{
							if (_MoleculesManager->_Molecules[0]->_DockingPoints[i]._IsSet && _MoleculesManager->_Molecules[1]->_DockingPoints[i]._IsSet)
								nbDockingPointsActive = i + 1;
						}
						if (nbDockingPointsActive > 0)
						{
							Tutorial::getInstance()->valideStep(0);
						}
					}

					//si c'est une suppression
					if (button == SDL_BUTTON_RIGHT)
					{
						float minDist = 0.0f;
						int selected = -1;
						for (int i = 0; i<NB_DOCKING_POINTS; i++)
						{
							if (_SelectedMol->_DockingPoints[i]._IsSet)
							{
								float dist = (_SelectedMol->_DockingPoints[i]._Pos - relpos).getMagnitude();
								//Log::log(Log::ENGINE_INFO,("Distance "+toString(dist) + " " + relpos.toStr()).c_str());
								if (selected == -1 || dist < minDist)
								{
									selected = i;
									minDist = dist;
								}
							}
						}

						if (selected >= 0)
						{
							_MoleculesManager->_Molecules[0]->_DockingPoints[selected]._IsSet = false;
							_MoleculesManager->_Molecules[1]->_DockingPoints[selected]._IsSet = false;
							_SoundDetach->play();
						}
					}
				}
			}


			//Rotation de la cam
			if(button == SDL_BUTTON_RIGHT)
			{
				if(!down)
					g_rotate_cam = false;
				else
					g_rotate_cam = true;
			}

			if(_HittedShip && hitPart)
			{
				if(hitPart)
				{
					Positionator transform;
					getNewPartPosFromHitPoint(_HittedShip,hitPart,hitPoint,&transform);

					if(button == SDL_BUTTON_LEFT && !down && (keybState[SDL_SCANCODE_LSHIFT]))
					{
						_HittedShip->addShipPart(_LastUsedType,transform);
						g_ship_manager->desactivateAllTempParts();
						g_add_part = false;
					}

					if(button == SDL_BUTTON_LEFT && down && (keybState[SDL_SCANCODE_LSHIFT]))
					{
						Positionator transform;
						getNewPartPosFromHitPoint(_HittedShip,hitPart,hitPoint,&transform);
						_HittedShip->activateTemporaryPart(true,1.0,transform,_LastUsedType);
						g_add_part = true;
					}

					if(button == SDL_BUTTON_RIGHT && !down && (keybState[SDL_SCANCODE_LSHIFT]))
						if(hitPart->_Type != SHIP_PART_NEXUS)
							_HittedShip->deleteShipPart(hitPart);

					if(keybState[SDL_SCANCODE_LSHIFT])
					{
						_HittedShip->addToPhysicEngine(_PhysicEngine);
						_HittedShip->saveShip("myShip.bin");
					}
				}
				else
				{
					g_ship_manager->desactivateAllTempParts();
					g_ship_manager->dehighlightAllParts();
					g_add_part = false;
				}
			}
		}


		void mouseMoveFunction(int buttonsState, int x, int y)
		{
			static int lastx = -1;
			static int lasty = -1;

			const Uint8 *keybState = SDL_GetKeyboardState(NULL);

			if(keybState[SDL_SCANCODE_LSHIFT])
			{
				ShipPart * hitPart = NULL;
				Ship * hitShip = NULL;
				NYVert3Df hitPoint;

				getHittedShip(x,y,&hitShip,&hitPart,&hitPoint);

				if(hitShip)
				{
					if(g_add_part)
					{
						Positionator transform;
						getNewPartPosFromHitPoint(hitShip,hitPart,hitPoint,&transform);
						hitShip->activateTemporaryPart(true,1.0f,transform,_LastUsedType);
					}
					else
					{
						g_ship_manager->desactivateAllTempParts();
					}

					g_ship_manager->dehighlightAllParts();
					g_selected_part = hitPart;
					if(g_selected_part)
						g_selected_part->_HighLighted = true;
				}
				else
				{
					g_ship_manager->desactivateAllTempParts();
					g_ship_manager->dehighlightAllParts();
					g_selected_part = NULL;
				}
			}

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

				if(g_rotate_cam)
				{
					_Renderer->_Camera->rotateAround((float)-dx/300.0f);
					_Renderer->_Camera->rotateUpAround((float)-dy/300.0f);
				}

				lastx = x;
				lasty = y;
			}
		}

		NYBody * getShotMol(NYVert3Df & hitPointWorld, NYVert3Df & hitPointRel)
		{
			NYVert3Df hitPointTmp;
			float distance = 0;
			float minDistance = 0;
			void * shapeOwner = NULL;
			void * shapeOwnerTmp = NULL;
			NYBody * body = NULL;
			Molecule * hitMol = NULL;


			NYBody * bodyTmp = _PhysicEngine->multipleRayPickup(MyShip->_Transform._Pos, MyShip->_Transform._Pos + MyShip->_Dir * 5000, &hitPointTmp, &distance, &shapeOwnerTmp);


			//On cherche le hit de mol le plus proche
			while (bodyTmp)
			{
				if (bodyTmp->BodyOwnerType == OWNER_MOLECULE)
				{
					if (distance < minDistance || minDistance == 0.0f)
					{
						hitPointWorld = hitPointTmp;
						minDistance = distance;
						body = bodyTmp;
						shapeOwner = shapeOwnerTmp;
					}
				}
				bodyTmp = _PhysicEngine->getNextHit(&hitPointTmp, &distance, &shapeOwnerTmp);
			}

			if (body)
			{
				Molecule* mol = (Molecule*)(body->BodyOwner);

				hitPointRel = hitPointWorld - mol->_Transform._Pos;
				NYFloatMatrix rot = mol->_Transform._Rot;
				rot.invert();
				hitPointRel = rot * hitPointRel;
			}

			return body;
		}

		void shipSetDockingPoint(bool add)
		{
			NYVert3Df hitPointWorld;
			NYVert3Df hitPointRel;
			NYBody * body = getShotMol(hitPointWorld, hitPointRel);

			if (body)
			{
				Molecule* mol = (Molecule*)(body->BodyOwner);

				if (add)
				{
					for (int i = 0; i<NB_DOCKING_POINTS; i++)
					{
						if (!mol->_DockingPoints[i]._IsSet)
						{
							mol->setDockingPoint(i, hitPointRel, true);
							_SoundsAttach[rand() % _NbSoundAttach]->play();
							LogActions::getInstance()->logSetHarpoonHandle(mol->_Id, hitPointRel);
							break;
						}
					}
				}
				else
				{
					float minDist = 0.0f;
					int selected = -1;
					for (int i = 0; i < NB_DOCKING_POINTS; i++)
					{
						if (mol->_DockingPoints[i]._IsSet)
						{
							float dist = (mol->_DockingPoints[i]._Pos - hitPointRel).getMagnitude();
							if (selected == -1 || dist < minDist)
							{
								selected = i;
								minDist = dist;
							}
						}
					}

					if (selected >= 0)
					{
						_MoleculesManager->_Molecules[0]->_DockingPoints[selected]._IsSet = false;
						_MoleculesManager->_Molecules[1]->_DockingPoints[selected]._IsSet = false;
						_SoundDetach->play();
					}
				}
			}
		}

		void gamePadFunction(NYVert3Df lj, NYVert3Df rj, float ltrig, float rtrig, bool ljoyb, bool rjoyb, bool lb, bool rb, bool a, bool b, bool x, bool y, bool xbox)
		{
			static bool ljoyb_last = false;
			static bool rjoyb_last = false;
			static bool lb_last = false;
			static bool rb_last = false;
			static bool a_last = false;
			static bool b_last = false;
			static bool x_last = false;
			static bool y_last = false;
			static bool xbox_last = false;
			static float rtrig_last = 0.0f;
			static float ltrig_last = 0.0f;
			static NYVert3Df lj_last;
			static NYVert3Df rj_last;

			float deadZone = 0.2;

			if (rtrig > deadZone )
				activateShipPartOnKey(true, SDL_SCANCODE_U, rtrig);
			if (rtrig < deadZone)
				activateShipPartOnKey(false, SDL_SCANCODE_U, rtrig);

			if (ltrig > deadZone)
				activateShipPartOnKey(true, SDL_SCANCODE_I, ltrig);
			if (ltrig < deadZone)
				activateShipPartOnKey(false, SDL_SCANCODE_I, ltrig);

			if (a && !a_last)
			{
				shipSetDockingPoint(true);
			}

			if (x && !x_last)
				activateShipPartOnKey(true, SDL_SCANCODE_O, x);
			if (!x && x_last)
				activateShipPartOnKey(false, SDL_SCANCODE_O, x);

			if (b && !b_last)
			{
				shipSetDockingPoint(false);
			}

			if (lb && !lb_last)
				activateShipPartOnKey(true, SDL_SCANCODE_Q, 0.5);
			if (!lb && lb_last)
				activateShipPartOnKey(false, SDL_SCANCODE_Q, 0.5);

			if (rb && !rb_last)
				activateShipPartOnKey(true, SDL_SCANCODE_E, 0.5);
			if (!rb && rb_last)
				activateShipPartOnKey(false, SDL_SCANCODE_E, 0.5);

			if (lj.Y > deadZone )
				activateShipPartOnKey(true, SDL_SCANCODE_S, abs(lj.Y));
			if (lj.Y < deadZone )
				activateShipPartOnKey(false, SDL_SCANCODE_S, abs(lj.Y));

			if (lj.Y < -deadZone )
				activateShipPartOnKey(true, SDL_SCANCODE_W, abs(lj.Y));
			if (lj.Y > -deadZone)
				activateShipPartOnKey(false, SDL_SCANCODE_W, abs(lj.Y));

			if (lj.X > deadZone )
				activateShipPartOnKey(true, SDL_SCANCODE_D, abs(lj.X));
			if (lj.X < deadZone )
				activateShipPartOnKey(false, SDL_SCANCODE_D, abs(lj.X));

			if (lj.X < -deadZone )
				activateShipPartOnKey(true, SDL_SCANCODE_A, abs(lj.X));
			if (lj.X > -deadZone )
				activateShipPartOnKey(false, SDL_SCANCODE_A, abs(lj.X));

			if (rj.getSize() > deadZone)
			{
				g_rotate_cam = true;
				_Renderer->_Camera->rotateAround((float)rj.X * 3 * _Renderer->_DeltaTime);
				_Renderer->_Camera->rotateUpAround((float)rj.Y * 3 * _Renderer->_DeltaTime);
			}
			else
				g_rotate_cam = false;


			/*_Force = false;
			Molecule * mol = NULL;

			if (lb)
			{
				mol = _MoleculesToMatch[0];
			}

			if (rb)
			{
				mol = _MoleculesToMatch[1];
			}

			if (lj.getSize() > 0.1)
			{
				if (mol)
				{
					_Force = true;
					NYVert3Df torque = this->_Renderer->_Camera->_Direction * 400 * lj.X;
					mol->_Body->applyTorque(torque);
				}
				else
				{
					NYVert3Df trans = _Renderer->_Camera->_Direction;
					trans *= lj.Y * 40;
					((NYCameraAnimated*)(_Renderer->_Camera))->startAnimTo(_Renderer->_Camera->_Position - trans, NYVert3Df(0, 1, 0), _Renderer->_Camera->_LookAt, 0.25f);
				}

			}

			if (rj.getSize() > 0.1)
			{

				if (mol)
				{
					_Force = true;

					NYVert3Df torqueH = this->_Renderer->_Camera->_UpVec * 400 * rj.X;
					NYVert3Df torqueV = this->_Renderer->_Camera->_NormVec * 400 * rj.Y;
					mol->_Body->applyTorque(torqueH);
					mol->_Body->applyTorque(torqueV);
				}
				else
				{
					_Renderer->_Camera->rotateAround((float)-rj.X * 3 * _Renderer->_DeltaTime);
					_Renderer->_Camera->rotateUpAround((float)-rj.Y * 3 * _Renderer->_DeltaTime);
				}
			}

			if (rtrig > 0.1)
			{
				//on rapproche
				NYVert3Df force = _MoleculesToMatch[0]->_Transform._Pos - _MoleculesToMatch[1]->_Transform._Pos;

				if (_MoleculesToMatch[0]->_Body)
					_MoleculesToMatch[0]->_Body->applyForce(force*-1, NYVert3Df(0, 0, 0));
				if (_MoleculesToMatch[1]->_Body)
					_MoleculesToMatch[1]->_Body->applyForce(force, NYVert3Df(0, 0, 0));
			}

			if (ltrig > 0.1)
			{
				//on rapproche
				NYVert3Df force = _MoleculesToMatch[0]->_Transform._Pos - _MoleculesToMatch[1]->_Transform._Pos;

				if (_MoleculesToMatch[0]->_Body)
					_MoleculesToMatch[0]->_Body->applyForce(force, NYVert3Df(0, 0, 0));
				if (_MoleculesToMatch[1]->_Body)
					_MoleculesToMatch[1]->_Body->applyForce(force*-1, NYVert3Df(0, 0, 0));
			}

			if (xbox && !xbox_last)
				startOptimisation(5000.0f);

			if (ljoyb || rjoyb)
				stopBothMols();*/

			ljoyb_last = ljoyb;
			rjoyb_last = rjoyb;
			lb_last = lb;
			rb_last = rb;
			a_last = a;
			b_last = b;
			x_last = x;
			y_last = y;
			xbox_last = xbox;
			rtrig_last = rtrig;
			ltrig_last = ltrig;
			lj_last = lj;
			rj_last = rj;

		}

};

#endif
