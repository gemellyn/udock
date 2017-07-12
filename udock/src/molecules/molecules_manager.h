#ifndef __MOLECULES_MANAGER__
#define __MOLECULES_MANAGER__

#include "molecule.h"
#include "molecules/marching_cubes/molecule_cubes.h"
#include "engine/hasher_3d.h"
#include <vector>
#include <utility> 
#include <algorithm>
#include <unordered_map>
#include <boost/thread.hpp>

#define CUTOFF_DISTANCE 16.0f //Taille des cubes du hasher 3D

typedef std::unordered_map<unsigned int, std::pair<float ,std::vector<size_t>>> AtomeHashmap;
//uid : pair --> energy, vertex offset

class MoleculesManager
{
	private :
		int _NbThreads;
		int _NbAtomesMaxBuffer;
		Atome * AtomesTransformesMol1;
		Atome * AtomesTransformesMol2;
		Hasher3D * _Hasher;
		Molecule * _MoleculesEvaluees[2];

		AtomeHashmap* atomeMap;
		unsigned int atomeCount;

	public :
		std::vector<Molecule*> _Molecules;

		typedef struct
		{
			std::wstring _File; ///< Fichier a charger
			Molecule * _Mol; ///< Molecule de cours de création
			bool _LowPoly;
			bool _Ses;
			bool _KeepTempGeom;
			bool * _Loaded;
			float * _Pcent;
		}PARAMS_THREAD_LOAD_MOL;

	    MoleculesManager()
		{
			//On lance autant de threads qu'on a de cores, en attente du calcul d'energie

			//On récup le nombre de processeurs sur le systeme
			/*#ifdef MSVC
			SYSTEM_INFO infos;
			GetSystemInfo(&infos);
			_NbThreads = infos.dwNumberOfProcessors;
			#else*/
			_NbThreads = boost::thread::hardware_concurrency();;
			//#endif
			//fprintf(stdout,"Found %d procs. Using %d threads for energy calculation.\n",_NbThreads,_NbThreads);
			_NbAtomesMaxBuffer = 0;
			AtomesTransformesMol1 = NULL;
			AtomesTransformesMol2 = NULL;
			_Hasher =  new Hasher3D();
			_MoleculesEvaluees[0] = NULL;
			_MoleculesEvaluees[1] = NULL;
			atomeMap = nullptr;
		}

		void setMoleculeEvaluees(Molecule * mol1, Molecule * mol2)
		{
			atomeCount = mol1->_NbAtomes + mol2->_NbAtomes;
			atomeMap = new AtomeHashmap(atomeCount);

			_MoleculesEvaluees[0] = mol1;
			_MoleculesEvaluees[1] = mol2;
		}

		void assyncLoadMol(Molecule * mol, std::wstring & file, bool lowPoly, bool ses, bool keepTempGeom, bool * loaded, float * pcent)
		{
			//HANDLE threadH;

			PARAMS_THREAD_LOAD_MOL * params = new PARAMS_THREAD_LOAD_MOL();
			*loaded = false;

			params->_File = file;
			params->_Mol = mol;
			params->_KeepTempGeom = keepTempGeom;
			params->_Loaded = loaded;
			params->_LowPoly = lowPoly;
			params->_Ses = ses;
			params->_Pcent = pcent;

			boost::thread t(&threadLoadMol,params);
            //threadH = (HANDLE)_beginthreadex (NULL,0,threadLoadMol,params,0,NULL);

		}

		void syncLoadMol(Molecule * mol, std::wstring & file, bool lowPoly, bool ses, bool keepTempGeom, bool * loaded, float * pcent)
		{
			PARAMS_THREAD_LOAD_MOL * params = new PARAMS_THREAD_LOAD_MOL();
			*loaded = false;

			params->_File = file;
			params->_Mol = mol;
			params->_KeepTempGeom = keepTempGeom;
			params->_Loaded = loaded;
			params->_LowPoly = lowPoly;
			params->_Ses = ses;
			params->_Pcent = pcent;
			threadLoadMol(params);
		}


		static  unsigned int threadLoadMol(void * loadMolParams)
		{
			PARAMS_THREAD_LOAD_MOL * params = (PARAMS_THREAD_LOAD_MOL*) loadMolParams;

			#ifdef MSVC
			if(_CrtCheckMemory() == false)
				Log::log(Log::ENGINE_ERROR,"Memory corruption !");
            #endif

			Log::log(Log::ENGINE_INFO, ("Loading mol "+wstring_to_utf8(params->_File)).c_str());
			params->_Mol->loadFromMOL2(params->_File.c_str());
			Log::log(Log::ENGINE_INFO, "Making geometry");
			params->_Mol->makeGeometry(params->_LowPoly,params->_Ses,0.0f,params->_Pcent);

			if(!params->_KeepTempGeom)
			{
				Log::log(Log::ENGINE_INFO, "Releasing temp geometry");
				params->_Mol->releaseTempGeometry();
			}

            #ifdef MSVC
			if(_CrtCheckMemory() == false)
				Log::log(Log::ENGINE_ERROR,"Memory corruption !");
            #endif

			Log::log(Log::ENGINE_INFO, "Loading thread ok");

			*(params->_Loaded) = true;

			return 0;
		}


		void addMolecule(Molecule * mol)
		{
			_Molecules.push_back(mol);
			if(mol->_NbAtomes > _NbAtomesMaxBuffer)
			{
				SAFEDELETE_TAB(AtomesTransformesMol1);
				SAFEDELETE_TAB(AtomesTransformesMol2);
				_NbAtomesMaxBuffer = mol->_NbAtomes;

				AtomesTransformesMol1 = new Atome[_NbAtomesMaxBuffer];
				AtomesTransformesMol2 = new Atome[_NbAtomesMaxBuffer];
			}

			initializeMolScore((MoleculeCubes*)mol);
		}

		void deleteMolecules(NYPhysicEngine * engine)
		{
			if (atomeMap != nullptr)
			{
				for (auto it = atomeMap->begin(); it != atomeMap->end(); ++it)
					it->second.second.clear();
				atomeMap->clear();
				SAFEDELETE(atomeMap);
			}
			//reset UID for atomeMap
			Atome::resetgenUID();

			removeMolsFromPhysicEngine(engine);
			for (unsigned int i = 0; i < _Molecules.size(); i++)
				SAFEDELETE(_Molecules[i]);
			_Molecules.clear();
			SAFEDELETE_TAB(AtomesTransformesMol1);
			SAFEDELETE_TAB(AtomesTransformesMol2);
			_NbAtomesMaxBuffer = 0;
		}

		void render(void)
		{
			for(unsigned int i=0;i<_Molecules.size();i++)
			{
				if(_Molecules[i]->_Visible)
					_Molecules[i]->render();
			}
		}

		void renderGrappins(void)
		{
			for(unsigned int i=0;i<_Molecules.size();i++)
			{
				if(_Molecules[i]->_Visible)
					_Molecules[i]->renderGrappins();
			}
		}

		void renderTransparent(void)
		{
			if(_Molecules.size() >= 2)
			{
				for(int i=0;i<NB_DOCKING_POINTS;i++)
				{

					if(_Molecules[0]->_DockingPoints[i]._IsSet && _Molecules[1]->_DockingPoints[i]._IsSet &&
					   _Molecules[0]->_Visible && _Molecules[1]->_Visible)
					{
						NYVert3Df point0WorldPos = _Molecules[0]->_Transform.Transform(_Molecules[0]->_DockingPoints[i]._Pos);
						NYVert3Df point1WorldPos = _Molecules[1]->_Transform.Transform(_Molecules[1]->_DockingPoints[i]._Pos);
						glColor3d(0,0,0);
						glBegin(GL_LINES);
						glVertex3f(point0WorldPos.X,point0WorldPos.Y,point0WorldPos.Z);
						glVertex3f(point1WorldPos.X,point1WorldPos.Y,point1WorldPos.Z);
						glEnd();
					}
				}
			}
		}

		void renderHiddenObjects(void)
		{
			/*for(unsigned int i=0;i<_Molecules.size();i++)
			{
				if(!_Molecules[i]->_Visible)
					_Molecules[i]->render();
			}*/
		}

		/**
		calcMol2Pos : on se laisse la possibilite de ne pas recalculer la position de la mol2. C'est utile
		quand on fait de l'optimisation, dans ce cas on ne modifie que la position de la mol1, et on ne
		calcule qu'une seule fois la position de la mol2 et son hashage.
		*/
		double calcEnergie(Molecule * mol1, Molecule * mol2, bool calcMol2Pos = true, int * nbCouplesCalc = NULL, double * pEnergyContact = NULL, double * pEnergyCharge = NULL)
		{
			//LARGE_INTEGER t1;
			//QueryPerformanceCounter(&t1);

			//On transforme les molécules (passage des atomes en repère global)
			NYVert3Df pos;

			for(int i=0;i<mol1->_NbAtomes;i++)
			{
				pos = mol1->_Atomes[i]._Pos;
				pos -= mol1->_Barycenter;
				pos = mol1->_Transform._Rot * pos;
				pos += mol1->_Transform._Pos;
				AtomesTransformesMol1[i]._Pos = pos;
				AtomesTransformesMol1[i]._Radius = mol1->_Atomes[i]._Radius;
				AtomesTransformesMol1[i]._Charge = mol1->_Atomes[i]._Charge;
				AtomesTransformesMol1[i]._UdockId = mol1->_Atomes[i]._UdockId;
			}

			if(calcMol2Pos)
			{
				//On va placer la seconde molecule dans le hasher pour trouver rapidement les voisins
				//Donc on en profite pour calculer sa taille max
				float minX =  mol2->_Atomes[0]._Pos.X;
				float minY =  mol2->_Atomes[0]._Pos.Y;
				float minZ =  mol2->_Atomes[0]._Pos.Z;
				float maxX =  mol2->_Atomes[0]._Pos.X;
				float maxY =  mol2->_Atomes[0]._Pos.Y;
				float maxZ =  mol2->_Atomes[0]._Pos.Z;

				for(int i=0;i<mol2->_NbAtomes;i++)
				{
					pos = mol2->_Atomes[i]._Pos;
					pos -= mol2->_Barycenter;
					pos = mol2->_Transform._Rot * pos;
					pos += mol2->_Transform._Pos;
					AtomesTransformesMol2[i]._Pos = pos;
					AtomesTransformesMol2[i]._Radius = mol2->_Atomes[i]._Radius;
					AtomesTransformesMol2[i]._Charge = mol2->_Atomes[i]._Charge;
					AtomesTransformesMol2[i]._UdockId = mol2->_Atomes[i]._UdockId;

					if(pos.X < minX)
						minX = pos.X;
					if(pos.Y < minY)
						minY = pos.Y;
					if(pos.Z < minZ)
						minZ = pos.Z;
					if(pos.X > maxX)
						maxX = pos.X;
					if(pos.Y > maxY)
						maxY = pos.Y;
					if(pos.Z > maxZ)
						maxZ = pos.Z;
				}

				float sizeMax = max(max((maxX-minX),(maxY-minY)),(maxZ-minZ));

				//On crée le hasher
				_Hasher->createBuckets(NYVert3Df(minX,minY,minZ),sizeMax,CUTOFF_DISTANCE);

				//On ajoute les atomes aux buckets
				for(int i=0;i<mol2->_NbAtomes;i++)
				{
					_Hasher->putInBucket(AtomesTransformesMol2[i]._Pos,AtomesTransformesMol2 + i);
				}
			}

			BucketRequest request;
			double energy = 0;
			double energyContact = 0;
			double energyCharge = 0;
			float radius1 = 0;
			double charge1 = 0;
			int udockId1 = 0;
			NYVert3Df pos1,pos2;

			double r;
			double rmSurR;
			double rmSurR6;
			double rmSurR8;
			double epsilon;
			double contact;
			double charge;

			for(int i=0;i<mol1->_NbAtomes;i++)
			{
				pos1 = AtomesTransformesMol1[i]._Pos;
				radius1 = AtomesTransformesMol1[i]._Radius;
				charge1 = AtomesTransformesMol1[i]._Charge;
				udockId1 = AtomesTransformesMol1[i]._UdockId;

				_Hasher->getBucketContentWithNeighbours(pos1,&request);

				for(int k=0;k<27;k++)
				{
					Atome * atomek = (Atome*) (request._Buckets[k]);
					while(atomek)
					{
						pos2 = atomek->_Pos;
						pos2 -= pos1;
						r = pos2.getSize();

						//Lennard Jones
						rmSurR = (radius1 + atomek->_Radius)/r;
						rmSurR6 = rmSurR * rmSurR * rmSurR;
						rmSurR6 = rmSurR6 * rmSurR6;

						//double rmSurR12 = rmSurR6 * rmSurR6;
						rmSurR8 = rmSurR6 * rmSurR * rmSurR;

						//On prend en compte cornell 1995 pour mixer les deux parties
        				epsilon = Atome::_AtomsEpsilonsSquared[atomek->_UdockId][udockId1];

						contact = epsilon * rmSurR8-(2*epsilon*rmSurR6);
						energyContact += contact;

						//Coulombic
						charge = (332.0761f/20.0f)* ((atomek->_Charge * charge1)/r);
						energyCharge += charge;

						//Total
						energy += contact + charge;

						//Suivant
						atomek = (Atome*) (atomek->_NextInBucket);

						if(nbCouplesCalc)
							(*nbCouplesCalc)++;
					}
				}
				//update score pour un atome;
				updateEnergyScoreForAtome(&AtomesTransformesMol1[i], energy);
			}

			applyScoreChanges((MoleculeCubes*)mol1, energy);

			if(pEnergyContact)
				*pEnergyContact = energyContact;
			if(pEnergyCharge)
				*pEnergyCharge = energyCharge;

			/*LARGE_INTEGER t2;
			QueryPerformanceCounter(&t2);
			LONGLONG elapsedLong = t2.QuadPart-t1.QuadPart;
			LARGE_INTEGER li_freq;
			QueryPerformanceFrequency(&li_freq);
			LONGLONG freq = li_freq.QuadPart;
			freq /= 1000;
			float elapsed = (float) ((float)elapsedLong/(float)freq);
			_cprintf("%ld ms for energy calc\n",elapsedLong);*/

			return energy;
		}

		void updateAfterPhysic(float elapsed)
		{
			for(unsigned int i=0;i<_Molecules.size();i++)
				_Molecules[i]->updateAfterPhysic(elapsed);
		}

		void removeMolsFromPhysicEngine(NYPhysicEngine * engine)
		{
			for(unsigned int i=0;i<_Molecules.size();i++)
				_Molecules[i]->removeFromPhysicEngine(engine);
		}

		void addMolsToPhysicEngine(NYPhysicEngine * engine)
		{
			for(unsigned int i=0;i<_Molecules.size();i++)
				_Molecules[i]->addToPhysicEngine(engine);
		}

		void freezeAllMols(bool freeze)
		{
			for(unsigned int i=0;i<_Molecules.size();i++)
				if(_Molecules[i]->_Body)
					_Molecules[i]->_Body->setFreeze(freeze);
		}

		void hideAllMols(bool hide)
		{
			for(unsigned int i=0;i<_Molecules.size();i++)
				_Molecules[i]->_Visible = !hide;
		}

		//update the score of the associated vertices of a atom
		void updateEnergyScoreForAtome(Atome* atome, const float& energy)
		{
			if (atome)
			{
				std::pair<float, std::vector<size_t>>* entry = nullptr;
				try {
					entry = &atomeMap->at(atome->UID);
				}
				catch (const std::out_of_range& oor) {
					std::cerr << "Out of Range error: " << oor.what() << " UID : " << atome->UID << '\n';
					return;
				}
				//update score (will be proportionated on apply)
				if(!entry->second.empty())
					entry->first = energy;
			}
		}

		void applyScoreChanges(MoleculeCubes* mol, const float& energyTotale)
		{
			AtomeHashmap::iterator it = atomeMap->begin();
			while (it != atomeMap->end())
			{
				//update score to have proportion based energy score
				it->second.first /= energyTotale;
				//fill the score for all vertex associated with current atome
				std::fill(it->second.second.begin(), it->second.second.end(), it->second.first);
				++it;
			}
			//opengl buffer update (1call)
		}

		//initialize the hashmap for a given molecule
		void initializeMolScore(MoleculeCubes* mol)
		{
			if (atomeMap == nullptr)
				atomeMap = new AtomeHashmap();

			size_t count = mol->_NbAtomes;
			atomeMap->reserve((size_t)(count + atomeMap->size()));

			for (size_t i = 0; i < count; i++)
			{
				Atome* atom = nullptr;
				atom = (mol->_Atomes + i);
				if (atom)
				{
					//ajoute l'atome à la hashmap
					atomeMap->emplace(
						atom->UID,
						std::pair<float, std::vector<size_t>>(
							0.0f,
							mol->getAssociatedVertices(*(atom))
							)
					);
				}
			}
		}
};

#endif
