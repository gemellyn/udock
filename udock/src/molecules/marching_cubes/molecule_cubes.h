#ifndef __MOLECULE_CUBES_H__
#define __MOLECULE_CUBES_H__

#include "molecules/molecule.h"
#include "marching_cubes.h"


class MoleculeCubes : public Molecule
{
	public :
		MarchingCubes * _MCubes;
		//MarchingCubes * _MCubesLowDef;
		MarchingCubes * _MCubesToRender;

	public :

		MoleculeCubes() : Molecule()
		{
			_MCubes  = new MarchingCubes();
			//_MCubesLowDef  = new MarchingCubes();
			_MCubesToRender = _MCubes;
			/*#ifdef MSVC
			SYSTEM_INFO infos;
			GetSystemInfo(&infos);
			_NbThreads = infos.dwNumberOfProcessors;
			#else*/
			_NbThreads =  boost::thread::hardware_concurrency();;
			//#endif
		}

		~MoleculeCubes()
		{
			releaseTempGeometry();
			_MCubes->destroyCubes();
			SAFEDELETE(_MCubes);
		}

		/**
		* Une fois qu'on a generé ce qu'on souhaitait, rendl a mémoire
		*/
		void releaseTempGeometry(void);

		/**
		  * On fabrique la géometrie en utilisant les marching cubes
		  */
		void makeGeometry(bool lowPoly, bool ses, float probe_boost = 0.0f, float * pcent = NULL);

		/**
		  * On output la géometrie dans un buffer opengl
		  */
		void addToOpengl(void);

		/**
		  * On fait le convex hull avec la géom
		  */
		void makeCollisionShapeFromGeom(NYPhysicEngine * engine, bool precise);

		/**
		  * On fait un compound shape de sphères
		  */
		void makeCollisionShapeFromMols(NYPhysicEngine * engine);

		/**
		  * A redefinir : ajoute la molecule au moteur physique : utilise la shape et crée le body
		  */
		void addToPhysicEngine(NYPhysicEngine * engine);

		/**
		  * A redefinir : enleve la molecule au moteur physique (conserve la shape mais remove le body)
		  */
		void removeFromPhysicEngine(NYPhysicEngine * engine);

		/**
		  * Rendu de la molécule
		  */
		void render(void);

		/**
		  * Rendu des grappins
		  */
		void renderGrappins(void);

		/**
		  * Pour tester d'autres rendus
		  */
		void setAlternativeRendering(void);

		/**
		  * Permet de sauvegarder dans un fichier binaire
		  */
		void saveToBinFile(const char * file);

		/**
		  * Permet de sauvegarder dans un fichier OBJ
		  */
		void saveToObjFile(const char * file);

		/**
		  * Permet de calculer, pour chaque atome, sa surface exposée (SAS)
		  */
		void calcSurfaceByAtom(bool normalize);

		/**
		* Permet de récupérer un tableau de vertex associé à un atome
		*/
		std::vector<size_t> getAssociatedVertices(const Atome& atom);

    private:

        int _NbThreads;
		typedef struct
		{
			MarchingCubes * _MCubes;
			Molecule * _Molecule;
			int _XStart;
			int _YStart;
			int _ZStart;
			int _NbX;
			int _NbY;
			int _NbZ;
		}PARAMS_THREAD_ELECTROSTATICS;



		/**
		  * Permet de calculer l'electrostatique de la molécule, dans chaque cube du marching cube
		  */
		void calculateElectrostatics(void);

        /**
		  * Uitilise par calculateElectrostatics pour calcul parallele.
		  */
		static void threadElectrostatics(PARAMS_THREAD_ELECTROSTATICS * params);
};


#endif
