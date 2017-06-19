#ifndef __LOAD_STATE__
#define __LOAD_STATE__

#include "game_states/ship_state.h"
#include "game_states/mol_state.h"
#include "molecules/molecules_manager.h"
#include "molecules/marching_cubes/molecule_cubes.h"
#include "ships/ship.h"
#include "engine/gui/screen_manager.h"


class LoadState : public GameState
{
	private:
		Molecule * _Mols[2];

	public:

		GUIScreenManager * _ScreenManager;
		GUIScreen * _Screen;
		GUIPBar * _LoadBar;
		GUILabel * _LoadLabel;
		MoleculesManager * _MoleculesManager;
		bool _PreLoaded[2];
		bool _Loaded[2];

		int _ToLoadId[2];
		std::wstring _ToLoad[2];
		unsigned long _MemoryBefore ;
		float _LoadPcent;
		Ship * MyShip;

		ShipState * _ShipState;
		MolState * _MolState;
		bool _LowPoly;

		LoadState() : GameState()
		{
			_LowPoly = false;
			#ifdef _DEBUG
			_LowPoly = true;
			#endif
			_Screen = NULL;
			_ScreenManager = NULL;
			_LoadLabel = NULL;
		}

		void setToLoad(int id1, int id2, wstring & filename1, wstring & filename2)
		{
			_ToLoad[0] = filename1;
			_ToLoad[1] = filename2;
			_ToLoadId[0] = id1;
			_ToLoadId[1] = id2;
		}

		bool activate(bool active)
		{
			GameState::activate(active);

			_LoadBar->Visible = active;
			_LoadBar->setValue(0);

			_MemoryBefore = getMemoryUsage();

			#ifdef MSVC
			if(_CrtCheckMemory() == false)
				Log::log(Log::ENGINE_ERROR,"Memory corruption !");
            #endif

			if(active)
			{
				_ScreenManager->setActiveScreen(_Screen);

				LogScreen::setLabel(_LoadLabel);

				Log::log(Log::ENGINE_INFO, "Activation of load state");

				MyShip->setPosition(NYVert3Df(0,-160,0));
				MyShip->setRotation(NYVert4Df(0,1,0,0));

				Log::log(Log::ENGINE_INFO, "Load state : removing molecules");
				_MoleculesManager->deleteMolecules(_PhysicEngine);

				_PreLoaded[0] = false;
				_Loaded[0] = false;
				_PreLoaded[1] = false;
				_Loaded[1] = false;
				_LoadPcent = 0;

				//On lance le chargement de la mol 1
				Log::log(Log::ENGINE_INFO, ("Load state : loading first mol from "+wstring_to_utf8(_ToLoad[0])).c_str());
				_Mols[0] = new MoleculeCubes();
				_Mols[0]->_Id = _ToLoadId[0];
				_MoleculesManager->assyncLoadMol(_Mols[0],_ToLoad[0],_LowPoly,true,true,_PreLoaded+0,&_LoadPcent);
			}

			return active;
		}

		void preState(float elapsed)
		{
			if(!_PreLoaded[0])
				_LoadBar->setValue(_LoadPcent/2.0f);
			if(_PreLoaded[0] && !_Loaded[0])
			{
				Log::log(Log::ENGINE_INFO, "Load state : first mol loaded");

				_LoadBar->setValue(0.5);
				_Mols[0]->addToOpengl();
				_Mols[0]->saveToObjFile("mol1.obj"); ///SAUVEGARDE OBJ
				_Mols[0]->makeCollisionShapeFromGeom(_PhysicEngine, true);
				_Mols[0]->_Transform._Pos = NYVert3Df(1000.0f,0,0);
				_Mols[0]->addToPhysicEngine(_PhysicEngine);
				_Mols[0]->_Transform._Pos = NYVert3Df((_Mols[0]->_Body->getWidth()/2.0f+20.0f),0,0);
				_Mols[0]->_Body->setBodyPosition(_Mols[0]->_Transform._Pos);
				_Mols[0]->releaseTempGeometry();
				_Loaded[0] = true;
				_Mols[0]->_Visible = false;
				_MoleculesManager->addMolecule(_Mols[0]);

				//On lance le chargement de la mol 2
				_Mols[1] = new MoleculeCubes();
				_LoadPcent = 0;
				Log::log(Log::ENGINE_INFO, ("Load state : loading second mol from "+wstring_to_utf8(_ToLoad[1])).c_str());
				_Mols[1]->_Id = _ToLoadId[1];
				_MoleculesManager->assyncLoadMol(_Mols[1],_ToLoad[1],_LowPoly,true,true,_PreLoaded+1,&_LoadPcent);

			}

			if(!_PreLoaded[1] && _PreLoaded[0])
				_LoadBar->setValue(0.5+_LoadPcent/2.0f);

			if(_PreLoaded[1] && !_Loaded[1])
			{
				Log::log(Log::ENGINE_INFO, "Load state : second mol loaded");

				_LoadBar->setValue(1.0);
				_Mols[1]->addToOpengl();
				_Mols[1]->saveToObjFile("mol2.obj"); ///SAUVEGARDE OBJ
				_Mols[1]->makeCollisionShapeFromGeom(_PhysicEngine, true);
				_Mols[1]->_Transform._Pos = NYVert3Df(-1000.0f,0,0);
				_Mols[1]->addToPhysicEngine(_PhysicEngine);
				_Mols[1]->_Transform._Pos = NYVert3Df(-(_Mols[1]->_Body->getWidth()/2.0f+20.0f),0,0);
				_Mols[1]->_Body->setBodyPosition(_Mols[1]->_Transform._Pos);
				_Mols[1]->releaseTempGeometry();
				_Loaded[1] = true;
				_Mols[1]->_Visible = false;
				_MoleculesManager->addMolecule(_Mols[1]);

				Log::log(Log::ENGINE_INFO,"Molecules loaded ok");
				unsigned long memoryAfter = getMemoryUsage();
				Log::log(Log::ENGINE_INFO,("We lost " + toString((memoryAfter - _MemoryBefore)/1024) + "ko during molecules loading process").c_str());

				this->activate(false);
				_MolState->activate(true);
				_MolState->resetPositions();
			}
		}


};

#endif
