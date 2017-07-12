#ifndef __MOLECULE_H__
#define __MOLECULE_H__

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>

#include <cstdio>

#include "engine/utils/types_3d.h"
#include "molecules/loaders/mol2_loader.h"
#include "molecules/loaders/pdb_loader.h"
#include "engine/hasher_3d.h"
#include "atom.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "engine/physics/physic_engine.h"
#include "engine/sound/sound.h"

#define THREAD_LIMIT 255
#define PROBE_SIZE 1.4f

#define NB_DOCKING_POINTS 20

class DockingPoint
{
	public:
		NYVert3Df _Pos;
		bool _IsSet;

		DockingPoint()
		{
			_IsSet = false;
		}
};

class NYBodyMolecule : public NYBody{
public:
	void collision(NYBody * other, btManifoldPoint& cp)
	{
		//if (other->BodyOwnerType == OWNER_MOLECULE)
		{
			//float force = (float)cp.getAppliedImpulse();
			//Log::log(Log::USER_INFO, ("Impact sur molecule, force "+toString(force)).c_str());

		}

	}
};

class Molecule
{
	public :
		Atome * _Atomes; ///< Table des atomes composant la molécule
		int _NbAtomes; ///< Nombre d'atomes pour cette molécule
		int _NbBonds; ///< pour le moment juste lu dans le fichier et utilisé pour genrer un fichier correct
		NYVert3Df _Barycenter; ///< Le barycentre de la molecule
		Positionator _Transform; ///< Position et rotation de la molecule
		NYHullShape * _CollideHull; ///< Convex hull pour la physique
		NYTriShape * _CollideMesh; ///< Tri mesh pour la physique
		NYCompoundShape * _CollideCompound; ///< Composition de colliders pour la physique
		bool _PreciseCollision; ///< Quel type de collision on utilise en ce moment
		NYBodyMolecule * _Body; ///< Lien avec le moteur physique
		float _Radius; ///< Distance max d'un atome au centre
		bool _Visible; ///< Si on fait le rendu
		int _Id; ///< L'id unique de la mol sur le serveur

		//Pour le docking
		DockingPoint _DockingPoints[NB_DOCKING_POINTS];

		//Infos sur la molécule
		std::string _MolName;
	    std::string _MolType;
	    std::string _ChargeType;
		std::wstring _Path; ///< fichier source de la molécule

		const unsigned int UID;

	private :
		bool _MaxDimDone; ///Si on a deja calculé la dimension max de la molécule
		float _MaxDim; ///La dimension max de la molécule

		static unsigned int genUID;

	public :

		Molecule(): UID(++genUID)
		{
			_Atomes = NULL;
			_NbAtomes = 0;
			_CollideHull = NULL;
			_CollideMesh = NULL;
			_CollideCompound = NULL;
			_Body = NULL;
			_NbBonds = 0;
			_Visible = true;
			_MaxDimDone = false;
			_MaxDim = 0;
			resetDockingPoints();
		}

		virtual ~Molecule()
		{
			SAFEDELETE_TAB(_Atomes);
			SAFEDELETE(_CollideHull);
			SAFEDELETE(_CollideMesh);
			SAFEDELETE(_CollideCompound);
		}

		/**
		  * Permet de savoir quel est l'atome le plus gros de la molécule
		  */
		float getBiggestAtomRadius(void);

		/**
		  * On récup le barycentre de la molécule
		  */
		void calcBarycenter(void);


		/**
		  * Permet de connaitre l'espace occupé par la molécule
		  */
		void getDimensions(float & minXres, float & maxXres, float & minYres, float & maxYres, float &minZres, float &maxZres);


		/**
		  * Permet de connaitre la dimension max de la molecule
		  */
		void getMaxDim(float & maxDim);


		/**
		  * Chargement de la molecule a partir d'un PDB
		  */
		int loadFromPDB(char * path);


		/**
		  * Chargement de la molecule a partir d'un MOL2
		  */
		int loadFromMOL2(const wchar_t * path);


		/**
		  * Sauvegarde la molecule dans un MOL2 en tenant compte de sa position actuelle
		  */
		int saveToMOL2(const char * path);

		/**
		  * Sauvegarde la molecule dans un PDB en tenant compte de sa position actuelle
		  */
		int saveToPDB(const char * path, bool useOriginalPos=false);

		/**
		  * Sauvegarde la molecule dans un PDB et ajoute l'autre directement a la suite
		  */
		int saveComplexToPDB(const char * path, Molecule * otherMol, float score, bool useOriginalPos=false);

		/**
		  * A redefinir : permet de sauvegarder dans un fichier
		  */
		virtual void saveToBinFile(const char * file) {};

		/**
		  * A redefinir : permet de sauvegarder dans un fichier OBJ
		  */
		virtual void saveToObjFile(const char * file) {};

		/**
		  * Sauvegarde de la position
		  */
		void savePosition(const char * path);

		/**
		  * chargement de la position
		  */
		void loadPosition(const char * path);

		/**
		* On libère la mémoire qu'on utilisera plus, une fois la génération terminée
		*/
		virtual void releaseTempGeometry(void) {};

		/**
		  * A redefinir : creation de la géometrie (peut etre threadé)
		  */
		virtual void makeGeometry(bool lowPoly, bool ses, float probe_boost = 0.0f, float * pcent = NULL) = 0;

		/**
		  * On output la géometrie dans un buffer opengl (a ne pas faire dans le rendu)
		  */
		virtual void addToOpengl(void) = 0;

		/**
		  * A redefinir : definit le convex hull a partir de la géometrie, en utilisant le moteur physique
		  */
		virtual void makeCollisionShapeFromGeom(NYPhysicEngine * engine, bool precise) = 0;

		/**
		  * A redefinir : definit un modèle physique a partir des molécules, composition de sphères
		  */
		virtual void makeCollisionShapeFromMols(NYPhysicEngine * engine) = 0;

		/**
		  * A redefinir : ajoute la molecule au moteur physique : utilise la shape et crée le body
		  */
		virtual void addToPhysicEngine(NYPhysicEngine * engine) = 0;

		/**
		  * A redefinir : enleve la molecule au moteur physique (conserve la shape mais remove le body)
		  */
		virtual void  removeFromPhysicEngine(NYPhysicEngine * engine) = 0;

		/**
		  * Met a jour la position de la molecule a partir de sa position dans le moteur physique
		  */
		virtual void updateAfterPhysic(float elapsed)
		{
			if(!this->_Body)
				return;
			this->_Transform._Pos = this->_Body->getBodyPosition();
			NYVert4Df rot = this->_Body->getBodyRotation();
			//Axe dans l'autre sens visiblemeent donc angle négatif
			this->_Transform._Rot.createRotateAxe(NYVert3Df(rot.X,rot.Y,rot.Z), -rot.T);
		}

		/**
		  * Met a jour la physique a partir de la position de la mol (danger)
		  */
		virtual void setToPhysic(void)
		{
			if(!this->_Body)
				return;
			this->_Body->setBodyPosition(this->_Transform._Pos);
			NYQuaternion quat;
			quat.initFromMat(this->_Transform._Rot);
			this->_Body->setBodyRotation(NYVert4Df(quat.X,quat.Y,quat.Z,-quat.T));

		}

		/**
		  * A redefinir :rendu de la molecule
		  */
		virtual void render(void) = 0;

		/**
		  * Rendu des grappins
		  */
		virtual void renderGrappins(void) = 0;

		/**
		  * A redefinir : permet d'avoir plusieurs rendus par type de molecule
		  */
		virtual void setAlternativeRendering(void) {};

		//Pour le docking
		/**
		  * On définit un point de docking
		  **/
		void setDockingPoint(int num, NYVert3Df pos, bool activate)
		{
			_DockingPoints[num]._Pos = pos;
			_DockingPoints[num]._IsSet = activate;
		}

		/**
		  * On reset tous les points de docking
		  **/
		void resetDockingPoints(void)
		{
			for(int i=0;i<NB_DOCKING_POINTS;i++)
				_DockingPoints[i]._IsSet = false;
		}

		/**
		* Position d'un point local en WorldPos
		*/
		void toWorldPos(NYVert3Df from, NYVert3Df * to)
		{
			*to = from;
			*to -= _Barycenter;
			*to = _Transform._Rot * (*to);
			*to += _Transform._Pos;
		}

		/**
		* Taille du radius
		*/
		void computeRadius(void)
		{
			_Radius = 0;
			for(int i=0;i<_NbAtomes; i++)
			{
				float dist = (_Atomes[i]._Pos - _Barycenter).getMagnitude();
				if(dist > _Radius)
				{
					_Radius = dist;
				}
			}
			_Radius = sqrt(_Radius);
		}

		/**
		  * Lissage convolutif quasi coulomb
		  * mais dans l'espace des atomes sans savoir la direction de la surface... bof
		  */
		void lisseCharges(void)
		{
			for(int i=0;i<_NbAtomes; i++)
			{
				_Atomes[i]._ChargeLissee = _Atomes[i]._Charge;
				float sommeDist = 0;
				for(int j=0;j<_NbAtomes; j++)
				{
					if(i != j)
					{
						float dist = (_Atomes[i]._Pos - _Atomes[j]._Pos).getSize();
						dist *= dist;
						_Atomes[i]._ChargeLissee += _Atomes[j]._Charge / dist;
						sommeDist += 1.0f/dist;
					}
				}
				_Atomes[i]._ChargeLissee /= sommeDist;
				_Atomes[i]._ChargeLissee *= 30;
			}
		}

		/**
		  * Donne la charge a n'importe quel point de l'espace autour de la molecule
		  */
		float getCharge(NYVert3Df position)
		{
			float charge = 0.0f;

			for(int i=0;i<_NbAtomes; i++)
			{
				float dist = (_Atomes[i]._Pos - position).getSize();
				//if (dist < 10.0f)
					charge += _Atomes[i]._Charge / dist;
			}
			return charge;
		}

		/**
		  * Permet de calculer, pour chaque atome, sa surface exposée (SAS)
		  */
		void calcSurfaceByAtom(bool normalize);

		/**
		  * Calcule le hashcode d'une molecule
		  */
		uint32 hashCode(void)
		{
			uint32 hash = 0;
			for(int i=0;i<_NbAtomes; i++)
			{
				hash ^= (uint32)_Atomes[i]._Pos.X;
				hash <<= 1;
			}

			return hash;
		}
};


#endif
