#ifndef __SHIP_MANAGER__
#define __SHIP_MANAGER__

#include "ships/ship.h"
#include "engine/utils/ny_utils.h"
#define _USE_MATH_DEFINES 
#include <math.h>

class ShipManager
{
	public:
		std::vector<Ship*> Ships;

		ShipManager()
		{
		}

		Ship * getShipByBody(NYBody * body)
		{
			for(unsigned int i=0;i<Ships.size();i++)
				if(Ships[i] == body->BodyOwner)
					return Ships[i];
			return NULL;
		}

		void buildRandomShip(NYPhysicEngine * engine)
		{
			Ship * ship = new Ship();
			Positionator pos;
			//pos.Translate(NYVert3Df(10,10,10));
			//pos.Rotate(NYVert3Df(0,1,0),0.6);

			/*for(int i=0;i<5000;i++)
			{
				int composante = rand()%3;
				float sign = (rand()%2 == 0) ? -1 : 1;
				SHIP_PART_TYPE type = (rand()%100 < 3) ? SHIP_PART_ENGINE : SHIP_PART_BASE;

				
				switch(composante)
				{
					case 0: pos.Translate(NYVert3Df(0,0,SHIP_PART_SIZE*sign)); break;
					case 1: pos.Translate(NYVert3Df(0,SHIP_PART_SIZE*sign,0)); break;
					case 2: pos.Translate(NYVert3Df(SHIP_PART_SIZE*sign,0,0)); break;	
				}

				ship->addShipPart(type,pos);
			}*/



			Ships.push_back(ship);

			ship->addToPhysicEngine(engine);
		}

		void buildBaseShip(NYPhysicEngine * engine)
		{
			Ship * ship = new Ship();
			Positionator pos;
			pos.Rotate(NYVert3Df(0.0f,0.0f,1.0f),(float)M_PI);
			

			ShipEngine * shipEngine = (ShipEngine*)ship->addShipPart(SHIP_PART_NEXUS,pos);
			//shipEngine->activate(true);
			/*pos.Translate(NYVert3Df(SHIP_PART_SIZE,0,0));
			ship->addShipPart(SHIP_PART_BASE,pos);
			pos.Translate(NYVert3Df(SHIP_PART_SIZE,0,0));
			ship->addShipPart(SHIP_PART_BASE,pos);
			pos.Translate(NYVert3Df(SHIP_PART_SIZE,0,0));
			ship->addShipPart(SHIP_PART_BASE,pos);
			pos.Translate(NYVert3Df(0,0,SHIP_PART_SIZE));
			ship->addShipPart(SHIP_PART_ENGINE,pos);
			pos.Translate(NYVert3Df(0,0,-2*SHIP_PART_SIZE));
			ship->addShipPart(SHIP_PART_ENGINE,pos);
			pos.Translate(NYVert3Df(0,0,SHIP_PART_SIZE));
			pos.Translate(NYVert3Df(SHIP_PART_SIZE,0,0));
			shipEngine = (ShipEngine*)ship->addShipPart(SHIP_PART_ENGINE,pos);
			//shipEngine->activate(true);
			pos.Translate(NYVert3Df(SHIP_PART_SIZE,0,0));
			shipEngine = (ShipEngine*)ship->addShipPart(SHIP_PART_ENGINE,pos);
			shipEngine->_Power /= 2.0f;
			///shipEngine->activate(true);*/
		

			
			Ships.push_back(ship);
			ship->addToPhysicEngine(engine);
		}

		void render(void)
		{
			for(unsigned int i=0;i<Ships.size();i++)
			{
				Ships[i]->render();
			}
		}

		void updateBeforePhysic(float elapsed)
		{
			for(unsigned int i=0;i<Ships.size();i++)
			{
				Ships[i]->updateBeforePhysic(elapsed);
			}
		}

		void updateAfterPhysic(float elapsed)
		{
			for(unsigned int i=0;i<Ships.size();i++)
			{
				Ships[i]->updateAfterPhysic(elapsed);
			}
			
		}

		void desactivateAllTempParts(void)
		{
			for(unsigned int i=0;i<Ships.size();i++)
			{
				Ships[i]->activateTemporaryPart(false,1.0,Positionator(),SHIP_PART_BASE);
			}
		}

		void dehighlightAllParts(void)
		{
			for(unsigned int i=0;i<Ships.size();i++)
			{
				Ships[i]->dehighlightAllParts();
			}
		}

		void hideAllShips(bool hide)
		{
			for(unsigned int i=0; i<Ships.size(); i++)
			{

				if(hide)
					Ships[i]->removeFromPhysic(NYPhysicEngine::getInstance());
				else
					Ships[i]->addToPhysicEngine(NYPhysicEngine::getInstance());

				Ships[i]->_Visible = !hide;
			}
		}
};

#endif