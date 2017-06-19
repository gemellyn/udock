#ifndef __MARCHING_CUBES_H__
#define __MARCHING_CUBES_H__

#include "engine/utils/types_3d.h"
#include "engine/utils/ny_utils.h"
#include "engine/render/renderer.h"

#include "boost/atomic/atomic.hpp"
#include "boost/thread.hpp"

//#define COLOR_BASE_R 155.0f/255.0f
//#define COLOR_BASE_V 194.0f/255.0f
//#define COLOR_BASE_B 238.0f/255.0f

//#define COLOR_MAX_POS_R 0.0f/255.0f
//#define COLOR_MAX_POS_V 218.0f/255.0f
//#define COLOR_MAX_POS_B 236.0f/255.0f

//#define COLOR_MAX_NEG_R 251.0f/255.0f
//#define COLOR_MAX_NEG_V 159.0f/255.0f
//#define COLOR_MAX_NEG_B 240.0f/255.0f

/*
#define COLOR_BASE_R 155.0f/255.0f
#define COLOR_BASE_V 194.0f/255.0f
#define COLOR_BASE_B 238.0f/255.0f

#define COLOR_MAX_NEG_R 217.0/255.0f
#define COLOR_MAX_NEG_V 192.0/255.0f
#define COLOR_MAX_NEG_B 237.0/255.0f

#define COLOR_MAX_POS_R 105.0/255.0f
#define COLOR_MAX_POS_V 197.0/255.0f
#define COLOR_MAX_POS_B 240.0f/255.0f
*/

//OK
//#define COLOR_BASE_R 155.0f/255.0f
//#define COLOR_BASE_V 194.0f/255.0f
//#define COLOR_BASE_B 238.0f/255.0f

#define COLOR_BASE_R 255.0f/255.0f
#define COLOR_BASE_V 255.0f/255.0f
#define COLOR_BASE_B 255.0f/255.0f

//#define COLOR_MAX_NEG_R 166.0f/255.0f
//#define COLOR_MAX_NEG_V 255.0f/255.0f
//#define COLOR_MAX_NEG_B 231.0f/255.0f

//OK
/*#define COLOR_MAX_NEG_R 217.0f/255.0f
#define COLOR_MAX_NEG_V 192.0f/255.0f
#define COLOR_MAX_NEG_B 237.0f/255.0f

#define COLOR_MAX_POS_R 26.0f/255.0f
#define COLOR_MAX_POS_V 172.0f/255.0f
#define COLOR_MAX_POS_B 240.0f/255.0f*/


#define COLOR_MAX_NEG_R 255.0f/255.0f
#define COLOR_MAX_NEG_V 0.0f/255.0f
#define COLOR_MAX_NEG_B 0.0f/255.0f

#define COLOR_MAX_POS_R 0.0f/255.0f
#define COLOR_MAX_POS_V 0.0f/255.0f
#define COLOR_MAX_POS_B 255.0f/255.0f

/*#define COLOR_MAX_NEG_R 0.0f/255.0f
#define COLOR_MAX_NEG_V 0.0f/255.0f
#define COLOR_MAX_NEG_B 0.0f/255.0f

#define COLOR_MAX_POS_R 255.0f/255.0f
#define COLOR_MAX_POS_V 255.0f/255.0f
#define COLOR_MAX_POS_B 255.0f/255.0f*/


/**
  * Remarque générale
  * pour reflechir, le repère pour un cube et pour l'hypercube est le suivant :
  * le sommet 1 (selon LorensenCline87) est en (0,0,0)
  * Meme repère que LorensenCline87 :
  * x par vers la droite
  * y vers le haut
  * z part dans la profondeur
  * donc repere main gauche
  * on note qu'ensuite, le repère de visualisation n'est pas forcément le meme
  */

/**
  * Fonctionnement :
  * On peut générer les cubes de deux facon : en interpolant ou pas la position finale des sommets des triangles le long des cotés.
  *
  * Si on interpole pas, on utilise peu de mémoire car on calcule la position des sommet à la fin, en les placant au milieu des cotés
  * L'interpolation est plus longue : on calcule l'intersection entre l'edge et la sphère à chaque fois qu'on traite un sommet du cube,
  * et si cette intersection est un peu plus loin du sommet traité que la précédente (recouvre la précédente puisque le sommet traité
  * est forcément dans la sphère) alors on la garde.

  * Pour generer une SES (solvent excluded surface - celle que montre pymol et co)
  * - generer la surface solvent accessible : ajouter les sphère avec le rayon (r_i + PROBE_SIZE). Ne pas utiliser l'interpolation, c'est juste pour marquer les cubes
  * - supprimer tous les cubes qui sont à PROBE_SIZE distance des cubes de surface de la SAS (methode contractSurface) qui propage aux sommets
  * - lisser la SES generant la surface de van der Walls : celle avec juste les atomes, donc en ajoutant des sphères de rayon r_i mais en ne modifiant
      que les cubes qui ne sont pas a 255
  */


//Un cube de l'algo des marching cubes
class MCube
{

	boost::atomic<unsigned char> Sommets; ///< Le numero de bit correspond à l'ordre de LorensenCline1987
	uint16 EdgeVectorDone; ///< Permet de savoir si on a deja calculé l'interpolation d'un sommet (pour initialiser la première fois) - champ bit
	NYVert3Df * EdgesVectors; ///< Contient les vecteurs d'edges qu'on interpole sur la surface
	NYVert3Df * Normals; ///< Les normales au sommet, qu'on calcule si on veut les lisser
	boost::atomic<char> Flags; ///< Utilise pour flager des cubes. 0: contraction surface 1: diffusion colorshift

	public:
		float ColorShift; ///< Modifie la couleur de base (par exemple pour la charge des atomes)
		float Temp; ///< Permet de stoquer un temporaire, par exemple pour un lissage

    public:
  		static int TianglesPerCode[256*15]; ///< Donne pour chaque code les sommets à ajouter au rendu
		static float SommetsNormalises[8*3]; ///< Donne les coordonnées des huit sommets d'un cube de marching cubes. Ils sont remis a jours quand on modifie la taille du cube
		static float Taille; ///< La taille d'un cube (longueur d'un cote)
		static bool Interpolation; //si on interpole la position des sommets sur les edges pour coller à la surface, valable pour tout les cubes et choisit une fois pour toutes au début
		static int VoisinFromEdge[4*3*12]; //Ce tableau donne la position relative des voisins en fonction d'un numero d'edge : 4 entiers x y z + numero d'edge pour les 3 voisins de chacun des 12 edges

	public:
		MCube()
		{
			EdgesVectors = NULL;
			EdgeVectorDone = 0;
			Normals = NULL;
			Flags = 0;
			ColorShift = 0;
			Temp = 0;
			Sommets = 0;
		}

		~MCube()
		{
			//On alloue des EdgesVectors a chaque fois qu'on traite un cube pour la permière fois, mais il faut relacher cette mémoire a la fin
			SAFEDELETE_TAB(EdgesVectors);
			SAFEDELETE_TAB(Normals);


		}

		uint8 getCode(void)
		{
			return Sommets;
		}

		void razSommets(void)
		{
			Sommets = 0;
		}

		//Permet de calculer des valeurs par defaut pour les edges, au cas ou on en calcul pas nous meme (ce qui ne doit pas arriver normalement)
		//place le edge vector au milieu des sommets
		void calcBasicEdgesVectors(void)
		{
			EdgesVectors[0] = NYVert3Df(SommetsNormalises[0],SommetsNormalises[1],SommetsNormalises[2]) + NYVert3Df(SommetsNormalises[3],SommetsNormalises[4],SommetsNormalises[5]);
			EdgesVectors[1] = NYVert3Df(SommetsNormalises[3],SommetsNormalises[4],SommetsNormalises[5]) + NYVert3Df(SommetsNormalises[6],SommetsNormalises[7],SommetsNormalises[8]);
			EdgesVectors[2] = NYVert3Df(SommetsNormalises[6],SommetsNormalises[7],SommetsNormalises[8]) + NYVert3Df(SommetsNormalises[9],SommetsNormalises[10],SommetsNormalises[11]);
			EdgesVectors[3] = NYVert3Df(SommetsNormalises[9],SommetsNormalises[10],SommetsNormalises[11]) + NYVert3Df(SommetsNormalises[0],SommetsNormalises[1],SommetsNormalises[2]);
			EdgesVectors[4] = NYVert3Df(SommetsNormalises[12],SommetsNormalises[13],SommetsNormalises[14]) + NYVert3Df(SommetsNormalises[15],SommetsNormalises[16],SommetsNormalises[17]);
			EdgesVectors[5] = NYVert3Df(SommetsNormalises[15],SommetsNormalises[16],SommetsNormalises[17]) + NYVert3Df(SommetsNormalises[18],SommetsNormalises[19],SommetsNormalises[20]);
			EdgesVectors[6] = NYVert3Df(SommetsNormalises[18],SommetsNormalises[19],SommetsNormalises[20]) + NYVert3Df(SommetsNormalises[21],SommetsNormalises[22],SommetsNormalises[23]);
			EdgesVectors[7] = NYVert3Df(SommetsNormalises[21],SommetsNormalises[22],SommetsNormalises[23]) + NYVert3Df(SommetsNormalises[12],SommetsNormalises[13],SommetsNormalises[14]);
			EdgesVectors[8] = NYVert3Df(SommetsNormalises[12],SommetsNormalises[13],SommetsNormalises[14]) + NYVert3Df(SommetsNormalises[0],SommetsNormalises[1],SommetsNormalises[2]);
			EdgesVectors[9] = NYVert3Df(SommetsNormalises[15],SommetsNormalises[16],SommetsNormalises[17]) + NYVert3Df(SommetsNormalises[3],SommetsNormalises[4],SommetsNormalises[5]);
			EdgesVectors[10] = NYVert3Df(SommetsNormalises[9],SommetsNormalises[10],SommetsNormalises[11]) + NYVert3Df(SommetsNormalises[21],SommetsNormalises[22],SommetsNormalises[23]);
			EdgesVectors[11] = NYVert3Df(SommetsNormalises[6],SommetsNormalises[7],SommetsNormalises[8]) + NYVert3Df(SommetsNormalises[18],SommetsNormalises[19],SommetsNormalises[20]);
			for(int i=0;i<12;i++)
				EdgesVectors[i] /= 2.0f;
		}

		//A n'utiliser qu'avant de créer des cubes...
		static void updateTaille(float taille)
		{
			for(int i=0;i<8*3;i++)
			{
				SommetsNormalises[i] /= Taille;
				SommetsNormalises[i] *= taille;
			}
			Taille = taille;
		}

		//A n'utiliser qu'avant de créer des cubes...
		static void SetInterpolation(bool interpolation)
		{
			MCube::Interpolation = interpolation;
		}

		//Num entre 0 et 7
		void setSommet(int num, bool val)
		{
			uint8 tmp = 0x01;
			tmp <<= num;

			if(val)
				Sommets.fetch_or(tmp);
			else
				Sommets.fetch_and(~tmp);
		}

		//Num entre 0 et 7
		bool getSommet(int num)
		{
			uint8 tmp = 0x01;
			tmp <<= num;
			return (Sommets & tmp?true:false);
		}

		//Num entre 0 et 7
		void setFlag(int num, bool val)
		{
			uint8 tmp = 0x01;
			tmp <<= num;
			if(val)
				Flags.fetch_or(tmp);
			else
				Flags.fetch_and(~tmp);
		}

		//Num entre 0 et 7
		bool getFlag(int num)
		{
			uint8 tmp = 0x01;
			tmp <<= num;
			return (Flags & tmp?true:false);
		}


		//Num entre 0 et 11
		void setEdgeVectorDone(int num)
		{
			uint16 tmp = 0x01;
			tmp <<= num;
			EdgeVectorDone |= tmp;
		}

		bool isEdgeVectorDone(int num)
		{
			uint16 tmp = 0x01;
			tmp <<= num;
			return (EdgeVectorDone & tmp?true:false);
		}

		//Num entre 0 et 7 et recalcule les edges en fonction des params d'un sphere
		//N'appeler la fonction avec l'interpolation d'activée que si on l'a déja appelée une fois
		//pour la meme sphère sans l'interpolation (car on ne traitera les edges que pour les cubes de valeur != 255 et != 0)
		void setSommetSphere(int num, bool val, NYVert3Df centre, float rayon, float colorShift)
		{
			//On valide le bit correpondant
			uint8 tmp = 0x01;
			tmp <<= num;
			if(val)
				Sommets.fetch_or(tmp);
			else
				Sommets.fetch_and(~tmp);

			//Couleur
			if(val)
			  ColorShift = colorShift;

			//On nettoie au fur et a mesure (face recouverte)
			if(EdgesVectors != NULL && (Sommets == 255 || Sommets == 0))
				SAFEDELETE_TAB(EdgesVectors);

			//Si on replace les sommets le long des edges
			if(Interpolation && Sommets != 255 && Sommets != 0)
			{
				//Si on a pas encore créé le tableau, on le crée et on l'initialise
				//Creation de mémoire souvent, mais impossible de l'avoir pour tous les cubes, pas la place
				if(EdgesVectors == NULL)
				{
					EdgesVectors = new NYVert3Df[12];
					calcBasicEdgesVectors();
				}

				//Le centre est exprimé en fonction du coin (0,0,0) du cube. Si le sommet n'est pas le (0,0,0) on décale la sphère en conséquence
				centre += NYVert3Df(SommetsNormalises[3*num],SommetsNormalises[3*num+1],SommetsNormalises[3*num+2]);

				NYVert3Df inter;
				NYVert3Df sommet;

				//Pour le sommet qu'on traite, on va vérifier les 3 edges
				//On calcule l'intersection avec la sphère, en partant du point au bout de l'edge, vers le point qu'on traite (ordre des parames interDroiteSphere pour trouver la bonne intersection)
				//Ensuite, si intersection, on vérifie qu'elle est au dessus de l'intersection calculée précédement (que la surface recouvre celle qu'on avait dejà)
				//Si on en avait pas deja (isEdgeVectorDone) on set l'intersection de toute facon;
				switch(num)
				{
					case 0: ///012
						sommet = NYVert3Df(SommetsNormalises[0],SommetsNormalises[1],SommetsNormalises[2]);
						if(interDroiteSphere(SommetsNormalises[3 ],SommetsNormalises[4 ],SommetsNormalises[5 ],SommetsNormalises[0],SommetsNormalises[1],SommetsNormalises[2],centre.X,centre.Y,centre.Z,rayon,inter))
							if((sommet - EdgesVectors[0]).getMagnitude() < (sommet - inter).getMagnitude() || !isEdgeVectorDone(0)) {EdgesVectors[0] = inter; setEdgeVectorDone(0);}
						if(interDroiteSphere(SommetsNormalises[9 ],SommetsNormalises[10],SommetsNormalises[11],SommetsNormalises[0],SommetsNormalises[1],SommetsNormalises[2],centre.X,centre.Y,centre.Z,rayon,inter))
							if((sommet - EdgesVectors[3]).getMagnitude() < (sommet - inter).getMagnitude() || !isEdgeVectorDone(3)) {EdgesVectors[3] = inter; setEdgeVectorDone(3);}
						if(interDroiteSphere(SommetsNormalises[12],SommetsNormalises[13],SommetsNormalises[14],SommetsNormalises[0],SommetsNormalises[1],SommetsNormalises[2],centre.X,centre.Y,centre.Z,rayon,inter))
							if((sommet - EdgesVectors[8]).getMagnitude() < (sommet - inter).getMagnitude() || !isEdgeVectorDone(8)) {EdgesVectors[8] = inter; setEdgeVectorDone(8);}
						break;
					case 1: //345
						sommet = NYVert3Df(SommetsNormalises[3],SommetsNormalises[4],SommetsNormalises[5]);
						if(interDroiteSphere(SommetsNormalises[0 ],SommetsNormalises[1 ],SommetsNormalises[2 ],SommetsNormalises[3],SommetsNormalises[4],SommetsNormalises[5],centre.X,centre.Y,centre.Z,rayon,inter))
							if((sommet - EdgesVectors[0]).getMagnitude() < (sommet - inter).getMagnitude() || !isEdgeVectorDone(0)) {EdgesVectors[0] = inter; setEdgeVectorDone(0);}
						if(interDroiteSphere(SommetsNormalises[6 ],SommetsNormalises[7 ],SommetsNormalises[8 ],SommetsNormalises[3],SommetsNormalises[4],SommetsNormalises[5],centre.X,centre.Y,centre.Z,rayon,inter))
							if((sommet - EdgesVectors[1]).getMagnitude() < (sommet - inter).getMagnitude() || !isEdgeVectorDone(1)) {EdgesVectors[1] = inter; setEdgeVectorDone(1);}
						if(interDroiteSphere(SommetsNormalises[15],SommetsNormalises[16],SommetsNormalises[17],SommetsNormalises[3],SommetsNormalises[4],SommetsNormalises[5],centre.X,centre.Y,centre.Z,rayon,inter))
							if((sommet - EdgesVectors[9]).getMagnitude() < (sommet - inter).getMagnitude() || !isEdgeVectorDone(9)) {EdgesVectors[9] = inter; setEdgeVectorDone(9);}
						break;
					case 2: //678
						sommet = NYVert3Df(SommetsNormalises[6],SommetsNormalises[7],SommetsNormalises[8]);
						if(interDroiteSphere(SommetsNormalises[3 ],SommetsNormalises[4 ],SommetsNormalises[5 ],SommetsNormalises[6],SommetsNormalises[7],SommetsNormalises[8],centre.X,centre.Y,centre.Z,rayon,inter))
							if((sommet - EdgesVectors[1 ]).getMagnitude() < (sommet - inter).getMagnitude() || !isEdgeVectorDone(1 )) {EdgesVectors[1 ] = inter; setEdgeVectorDone(1 );}
						if(interDroiteSphere(SommetsNormalises[9 ],SommetsNormalises[10],SommetsNormalises[11],SommetsNormalises[6],SommetsNormalises[7],SommetsNormalises[8],centre.X,centre.Y,centre.Z,rayon,inter))
							if((sommet - EdgesVectors[2 ]).getMagnitude() < (sommet - inter).getMagnitude() || !isEdgeVectorDone(2 )) {EdgesVectors[2 ] = inter; setEdgeVectorDone(2 );}
						if(interDroiteSphere(SommetsNormalises[18],SommetsNormalises[19],SommetsNormalises[20],SommetsNormalises[6],SommetsNormalises[7],SommetsNormalises[8],centre.X,centre.Y,centre.Z,rayon,inter))
							if((sommet - EdgesVectors[11]).getMagnitude() < (sommet - inter).getMagnitude() || !isEdgeVectorDone(11)) {EdgesVectors[11] = inter; setEdgeVectorDone(11);}
						break;
					case 3: //9 10 11
						sommet = NYVert3Df(SommetsNormalises[9],SommetsNormalises[10],SommetsNormalises[11]);
						if(interDroiteSphere(SommetsNormalises[6 ],SommetsNormalises[7 ],SommetsNormalises[8 ],SommetsNormalises[9],SommetsNormalises[10],SommetsNormalises[11],centre.X,centre.Y,centre.Z,rayon,inter))
							if((sommet - EdgesVectors[2 ]).getMagnitude() < (sommet - inter).getMagnitude() || !isEdgeVectorDone(2 )) {EdgesVectors[2 ] = inter; setEdgeVectorDone(2 );}
						if(interDroiteSphere(SommetsNormalises[0 ],SommetsNormalises[1 ],SommetsNormalises[2 ],SommetsNormalises[9],SommetsNormalises[10],SommetsNormalises[11],centre.X,centre.Y,centre.Z,rayon,inter))
							if((sommet - EdgesVectors[3 ]).getMagnitude() < (sommet - inter).getMagnitude() || !isEdgeVectorDone(3 )) {EdgesVectors[3 ] = inter; setEdgeVectorDone(3 );}
						if(interDroiteSphere(SommetsNormalises[21],SommetsNormalises[22],SommetsNormalises[23],SommetsNormalises[9],SommetsNormalises[10],SommetsNormalises[11],centre.X,centre.Y,centre.Z,rayon,inter))
							if((sommet - EdgesVectors[10]).getMagnitude() < (sommet - inter).getMagnitude() || !isEdgeVectorDone(10)) {EdgesVectors[10] = inter; setEdgeVectorDone(10);}
						break;
					case 4: //12 13 14
						sommet = NYVert3Df(SommetsNormalises[12],SommetsNormalises[13],SommetsNormalises[14]);
						if(interDroiteSphere(SommetsNormalises[15],SommetsNormalises[16],SommetsNormalises[17],SommetsNormalises[12],SommetsNormalises[13],SommetsNormalises[14],centre.X,centre.Y,centre.Z,rayon,inter))
							if((sommet - EdgesVectors[4]).getMagnitude() < (sommet - inter).getMagnitude() || !isEdgeVectorDone(4)) {EdgesVectors[4] = inter; setEdgeVectorDone(4);}
						if(interDroiteSphere(SommetsNormalises[21],SommetsNormalises[22],SommetsNormalises[23],SommetsNormalises[12],SommetsNormalises[13],SommetsNormalises[14],centre.X,centre.Y,centre.Z,rayon,inter))
							if((sommet - EdgesVectors[7]).getMagnitude() < (sommet - inter).getMagnitude() || !isEdgeVectorDone(7)) {EdgesVectors[7] = inter; setEdgeVectorDone(7);}
						if(interDroiteSphere(SommetsNormalises[0 ],SommetsNormalises[1 ],SommetsNormalises[2 ],SommetsNormalises[12],SommetsNormalises[13],SommetsNormalises[14],centre.X,centre.Y,centre.Z,rayon,inter))
							if((sommet - EdgesVectors[8]).getMagnitude() < (sommet - inter).getMagnitude() || !isEdgeVectorDone(8)) {EdgesVectors[8] = inter; setEdgeVectorDone(8);}
						break;
					case 5: //15 16 17
						sommet = NYVert3Df(SommetsNormalises[15],SommetsNormalises[16],SommetsNormalises[17]);
						if(interDroiteSphere(SommetsNormalises[12],SommetsNormalises[13],SommetsNormalises[14],SommetsNormalises[15],SommetsNormalises[16],SommetsNormalises[17],centre.X,centre.Y,centre.Z,rayon,inter))
							if((sommet - EdgesVectors[4]).getMagnitude() < (sommet - inter).getMagnitude() || !isEdgeVectorDone(4)) {EdgesVectors[4] = inter; setEdgeVectorDone(4);}
						if(interDroiteSphere(SommetsNormalises[18],SommetsNormalises[19],SommetsNormalises[20],SommetsNormalises[15],SommetsNormalises[16],SommetsNormalises[17],centre.X,centre.Y,centre.Z,rayon,inter))
							if((sommet - EdgesVectors[5]).getMagnitude() < (sommet - inter).getMagnitude() || !isEdgeVectorDone(5)) {EdgesVectors[5] = inter; setEdgeVectorDone(5);}
						if(interDroiteSphere(SommetsNormalises[3 ],SommetsNormalises[4 ],SommetsNormalises[5 ],SommetsNormalises[15],SommetsNormalises[16],SommetsNormalises[17],centre.X,centre.Y,centre.Z,rayon,inter))
							if((sommet - EdgesVectors[9]).getMagnitude() < (sommet - inter).getMagnitude() || !isEdgeVectorDone(9)) {EdgesVectors[9] = inter; setEdgeVectorDone(9);}
						break;
					case 6: //18 19 20
						sommet = NYVert3Df(SommetsNormalises[18],SommetsNormalises[19],SommetsNormalises[20]);
						if(interDroiteSphere(SommetsNormalises[15],SommetsNormalises[16],SommetsNormalises[17],SommetsNormalises[18],SommetsNormalises[19],SommetsNormalises[20],centre.X,centre.Y,centre.Z,rayon,inter))
							if((sommet - EdgesVectors[5 ]).getMagnitude() < (sommet - inter).getMagnitude() || !isEdgeVectorDone(5 )) {EdgesVectors[5 ] = inter; setEdgeVectorDone(5 );}
						if(interDroiteSphere(SommetsNormalises[21],SommetsNormalises[22],SommetsNormalises[23],SommetsNormalises[18],SommetsNormalises[19],SommetsNormalises[20],centre.X,centre.Y,centre.Z,rayon,inter))
							if((sommet - EdgesVectors[6 ]).getMagnitude() < (sommet - inter).getMagnitude() || !isEdgeVectorDone(6 )) {EdgesVectors[6 ] = inter; setEdgeVectorDone(6 );}
						if(interDroiteSphere(SommetsNormalises[6 ],SommetsNormalises[7 ],SommetsNormalises[8 ],SommetsNormalises[18],SommetsNormalises[19],SommetsNormalises[20],centre.X,centre.Y,centre.Z,rayon,inter))
							if((sommet - EdgesVectors[11]).getMagnitude() < (sommet - inter).getMagnitude() || !isEdgeVectorDone(11)) {EdgesVectors[11] = inter; setEdgeVectorDone(11);}
						break;
					case 7: //21 22 23
						sommet = NYVert3Df(SommetsNormalises[21],SommetsNormalises[22],SommetsNormalises[23]);
						if(interDroiteSphere(SommetsNormalises[18],SommetsNormalises[19],SommetsNormalises[20],SommetsNormalises[21],SommetsNormalises[22],SommetsNormalises[23],centre.X,centre.Y,centre.Z,rayon,inter))
							if((sommet - EdgesVectors[6 ]).getMagnitude() < (sommet - inter).getMagnitude() || !isEdgeVectorDone(6 )) {EdgesVectors[6 ] = inter; setEdgeVectorDone(6 );}
						if(interDroiteSphere(SommetsNormalises[12],SommetsNormalises[13],SommetsNormalises[14],SommetsNormalises[21],SommetsNormalises[22],SommetsNormalises[23],centre.X,centre.Y,centre.Z,rayon,inter))
							if((sommet - EdgesVectors[7 ]).getMagnitude() < (sommet - inter).getMagnitude() || !isEdgeVectorDone(7 )) {EdgesVectors[7 ] = inter; setEdgeVectorDone(7 );}
						if(interDroiteSphere(SommetsNormalises[9 ],SommetsNormalises[10],SommetsNormalises[11],SommetsNormalises[21],SommetsNormalises[22],SommetsNormalises[23],centre.X,centre.Y,centre.Z,rayon,inter))
							if((sommet - EdgesVectors[10]).getMagnitude() < (sommet - inter).getMagnitude() || !isEdgeVectorDone(10)) {EdgesVectors[10] = inter; setEdgeVectorDone(10);}
						break;
				}
			}
		}



		//Permet de récupérer un vertex d'un triangle
		//il y'en a 12 possibles (un par edge)
		//num : 0-11
		//Le cube se considère avec son sommet 1 en 0,0,0 et de taille 1
		void getEdgeVertice(NYVert3Df & vert, int num)
		{
			//Si pas d'interpolation deja calculée, on calcule celle au milieu
			if(EdgesVectors == NULL)
			{
				//Les deux sommets à interpoler
				int interpol0=0;
				int interpol1=0;

				switch(num)
				{
					case 0:	 interpol0 = 0;	interpol1 = 1;	break;
					case 1:	 interpol0 = 1;	interpol1 = 2;	break;
					case 2:	 interpol0 = 2;	interpol1 = 3;	break;
					case 3:	 interpol0 = 3;	interpol1 = 0;	break;
					case 4:	 interpol0 = 4;	interpol1 = 5;	break;
					case 5:	 interpol0 = 5;	interpol1 = 6;	break;
					case 6:	 interpol0 = 6;	interpol1 = 7;	break;
					case 7:	 interpol0 = 7;	interpol1 = 4;	break;
					case 8:	 interpol0 = 0;	interpol1 = 4;	break;
					case 9:	 interpol0 = 1;	interpol1 = 5;	break;
					case 10: interpol0 = 3;	interpol1 = 7;	break;
					case 11: interpol0 = 2;	interpol1 = 6;	break;
				}

				//On interpole
				NYVert3Df sommet0(SommetsNormalises[interpol0*3],SommetsNormalises[interpol0*3+1],SommetsNormalises[interpol0*3+2]);
				NYVert3Df sommet1(SommetsNormalises[interpol1*3],SommetsNormalises[interpol1*3+1],SommetsNormalises[interpol1*3+2]);
				vert = sommet0 + sommet1;
				vert /= 2.0f;
			}
			else
			{
				//Sinon on va recup le deja calcule
				vert = EdgesVectors[num];
			}
		}

		//Retourne le nombre de triangles en fonction du code du cube
		int getNbTriangles(void)
		{
			int i=0;
			while(TianglesPerCode[Sommets*15 + i] != -1 && i < 15)
				i++;
			return i/3;
		}

		//A calculer une fois le cube traite !
		void calcNormals(void)
		{
			uint8 code = Sommets;

			//si c'est pas un cube qui output des faces on quitte
			if(code == 255 || code == 0)
				return;

			//Si pas deja fait, on alloue le buffer de normales
			if(Normals == NULL)
				Normals = new NYVert3Df[12];

			//Offset dans le buffer des faces
			int offset = code*15;

			//On calcule les normales de chaque face
			int i=0;

			//On fait face par face
			NYVert3Df vertice[3];
			int numVertices[3];
			while(MCube::TianglesPerCode[offset + i] != -1 && i < 15)
			{
				//On chope une face
				for(int j=0;j<3;j++)
				{
					//On chope le numéro de vertice
					int vertNum = MCube::TianglesPerCode[offset + i + j];

					///On le stoque
					numVertices[j] = vertNum;

					//On en récup le point correspondant, dans l'espace d'un cube placé à l'origine
					getEdgeVertice(vertice[j],vertNum);
				}

				//On calcule la normale
				NYVert3Df v1 = vertice[1] - vertice[0];
				NYVert3Df v2 = vertice[2] - vertice[0];
				NYVert3Df normal = v2.vecProd(v1);
				normal.normalize();

				//On attribue la normale calculée au bon edge
				for(int j=0;j<3;j++)
					Normals[numVertices[j]] = normal;

				//Face suivante
				i+=3;
			}
		}

		void getEdgeNormal(NYVert3Df & normal, int num)
		{
			if(Normals != NULL)
				normal = Normals[num];
		}

		void setEdgeNormal(NYVert3Df & normal, int num)
		{
			if(Normals != NULL)
				 Normals[num] = normal;
		}

		//Pour savoir en gros ou se trouve la partie vide de la molecule
		void getGlobalNormal(NYVert3Df & normal)
		{
			if(Normals != NULL)
			{
				normal = NYVert3Df();
				for(int i=0;i<12;i++)
					normal += Normals[i];
				normal.normalize();
			}
		}

		void getFaceCenter(NYVert3Df & barycentre)
		{
			uint8 code = Sommets;

			//si c'est pas un cube qui output des faces on quitte
			if(code == 255 || code == 0)
				return;

			//Offset dans le buffer des faces
			int offset = code*15;

			//On calcule le barycentre des faces
			int i=0;

			//On fait face par face
			int nbPoints = 0;
			NYVert3Df vertice;
			barycentre = NYVert3Df();
			while(MCube::TianglesPerCode[offset + i] != -1 && i < 15)
			{
				//On chope le numéro de vertice
				int vertNum = MCube::TianglesPerCode[offset + i];

				//On en récup le point correspondant, dans l'espace d'un cube placé à l'origine
				getEdgeVertice(vertice,vertNum);

				//Log::log(Log::ENGINE_INFO,("Vertice :" + vertice.toStr()).c_str());

				barycentre += vertice;
				nbPoints++;

				//Face suivante
				i++;
			}

			//On calcule le barycentre
			barycentre /= (float)nbPoints;

			//Log::log(Log::ENGINE_INFO,("barycentre :" + barycentre.toStr() + " nb points :" + toString(nbPoints)).c_str());


		}
};

class MarchingCubes
{
	public :
		MCube * Cubes;
		float TailleCube;
		int NbX;
		int NbY;
		int NbZ;
		int NbCubes;
		NYVert3Df Origin; ///< on décale pour le caler autour de l'objet a modéliser


		float * _Vertices; ///< Tableau temporaire pour stoquer les vertices
		int _NbVertices; ///< Nombre de vertices dans le tableau
		//unsigned int * _Indices; ///< Tableau temporaire de stoquage des indices
		//unsigned _NbIndices; ///< Nombre d'indices dans le tableau d'indices
		bool _PrecomputeNormals; ///<Si on doit calculer les normales pour qu'elles soient lissées
		bool _NormalsPrecomputed; ///< Si on a effectivement calculé les normales

		GLuint _BufGeom; ///< Buffer de rendu OGL de la molécule (geometrie)
		GLuint _VAO; ///< Vertex Array Object : pour définir le format du VBO _BufGeom

		int _VertexFloatsCountVBO; ///< Nombre de flottants par somment dans le VBO

		int _NbThreads;
		typedef struct
		{
			MarchingCubes * MCubes;
			float Radius;
			int XStart;
			int YStart;
			int ZStart;
			int NbX;
			int NbY;
			int NbZ;
		}PARAMS_THREAD_CONTRACT;

	public:
		MarchingCubes()
		{
			Cubes = NULL;
			_Vertices = NULL;
			NbX = 0;
			NbY = 0;
			NbZ = 0;
			TailleCube = 0;
			NbCubes = 0;
			Origin = NYVert3Df();
			_PrecomputeNormals = false;
			_NormalsPrecomputed = false;
			_BufGeom = 0;
			_VAO = 0;

            /*#ifdef MSVC
			SYSTEM_INFO infos;
			GetSystemInfo(&infos);
			_NbThreads = infos.dwNumberOfProcessors;
			#else*/
			_NbThreads = boost::thread::hardware_concurrency();
			//#endif

		}

		~MarchingCubes()
		{
			SAFEDELETE_TAB(_Vertices);
			if(_BufGeom != 0)
				glDeleteBuffers(1, &_BufGeom);
			if(_VAO != 0)
				glDeleteVertexArrays(1, &_VAO);
			_BufGeom = 0;
			_VAO = 0;
		}

		//Si on demande les précalcul des normales,
		//elles seront calculées et lissées (mais allocation mémoire)
		//sinon calculées à la volée au moment du stockage des facettes (mais pas lissées)
		void setLissageNormales(bool bLissage)
		{
			_PrecomputeNormals = bLissage;
		}

		void setInterpolation(bool bInterpolation)
		{
			MCube::SetInterpolation(bInterpolation);
		}

		void setOrigin(NYVert3Df origin)
		{
			Origin = origin;
		}

		void create(float sizeX, float sizeY, float sizeZ, float sizeCube, bool interpolation)
		{
			TailleCube = sizeCube;
			NbX = (int)(sizeX / sizeCube) + 1;
			NbY = (int)(sizeY / sizeCube) + 1;
			NbZ = (int)(sizeZ / sizeCube) + 1;
			NbCubes = NbX * NbY * NbZ;

			MCube::updateTaille(TailleCube);
			MCube::SetInterpolation(interpolation);

			SAFEDELETE_TAB(Cubes);
			Cubes = new MCube[NbCubes];
			if(Cubes == NULL)
			{
				Log::log(Log::ENGINE_ERROR,"Unable to create cubes, no more memory");
				return;
			}

			float taille = (float) sizeof(MCube)*NbCubes;
			taille /= 1024.0f*1024.0f;
			Log::log(Log::ENGINE_INFO,("Marching cubes : allocating " +
										toString(taille) +
										"Mo for " +
										toString(NbCubes) +
										" cubes").c_str());
		}

		//Ne detruit pas les données de géometrie, juste la structure des cubes
		void destroyCubes(void)
		{
			SAFEDELETE_TAB(Cubes);
		}

	public:
		//Permet de contracter la surface sur un rayon donné :
		//On parcourt tous les cubes de surface et on supprime tous les cubes dans un hypercube donné autour de ce cube
		//Pas très précis mais rapide, permet de passer de SAS a SES
		void contractSurface(float radius)
		{
			//On flag tous les sommets de la surface
			//en effet, on va generer de la surface au fur et a mesure, donc impossible de trouver la surface a contracter des qu'on genrere un peu de la contractée sauf par flag
			uint8 numFlag = 0;
			for(int z = 0; z < NbZ; z++)
			{
				int offsetZ = z * (NbX*NbY);
				for(int y = 0; y < NbY; y++)
				{
					int offsetY = y * NbX;
					for(int x = 0; x < NbX; x++)
					{
						int indice = x + offsetY + offsetZ;
						int code = Cubes[indice].getCode();

						//On flag les cubes de surface comme a traiter
						if(code != 255 && code != 0)
							Cubes[indice].setFlag(numFlag,true);
					}
				}
			}


			//_NbThreads = 5;
			int stepX = (NbX / _NbThreads);// - radius/TailleCube - 2;

			boost::thread ** threads = new boost::thread *[_NbThreads];
			PARAMS_THREAD_CONTRACT * params = new PARAMS_THREAD_CONTRACT[_NbThreads];

			//_cprintf("x: %d\n",stepX);

			for(int i=0;i<_NbThreads;i++)
			{
				params[i].MCubes = this;
				params[i].Radius = radius;
				params[i].XStart = (i*NbX) / _NbThreads;// + (radius/(2*TailleCube)) + 1;
				params[i].YStart = 0;
				params[i].ZStart = 0;
				params[i].NbX = stepX;
				params[i].NbY = NbY;
				params[i].NbZ = NbZ;
				threads[i] = new boost::thread(threadContractSurface,params + i);
				//_beginthreadex (NULL,0,threadContractSurface,params + i,0,NULL);
			}

			//On attend la fin des threads
			/*DWORD resWait = WaitForMultipleObjects(_NbThreads,threadHandles,TRUE,INFINITE);
			if(resWait == WAIT_FAILED)
				Log::log(Log::ENGINE_ERROR,("Error " + toString(GetLastError()) + " waiting for threads when contracting surface").c_str());*/
            for(int i=0;i<_NbThreads;i++)
                threads[i]->join();

            for(int i=0;i<_NbThreads;i++)
                SAFEDELETE(threads[i]);

			SAFEDELETE_TAB(threads);
			SAFEDELETE_TAB(params);

			Log::log(Log::ENGINE_INFO,"End of surface contration threads");

			//On finit les oubliés
			contractSurface(radius,0,NbX,0,NbY,0,NbZ);
		}

	private:

		/*
		 * Procédure de contraction de la surface, lancée en thread
		 */
		static unsigned int threadContractSurface(void * contractParams)
		{
			PARAMS_THREAD_CONTRACT * params = (PARAMS_THREAD_CONTRACT*) contractParams;
			params->MCubes->contractSurface(params->Radius,params->XStart,params->NbX,params->YStart,params->NbY,params->ZStart,params->NbZ);
			return 0;
		}

		//on contracte la surface sur un sous ensemble de l'hypercube, utile pour paralleliser
		void contractSurface(float radius,int startx, int nbx, int starty, int nby, int startz, int nbz)
		{
			uint8 numFlag = 0;

			//On va parcourir un hypercube autour de chaque cube de surface
			int rayonHyperCube = (int)(radius/TailleCube)+1;
			float magDist = radius * radius;

			NYVert3Df posCubeCentral;
			for(int z = startz; z < startz+nbz; z++)
			{
				int offsetZ = z * (NbX*NbY);
				for(int y = starty; y < starty+nby; y++)
				{
					int offsetY = y * NbX;
					for(int x = startx; x < startx+nbx; x++)
					{
						int indice = x + offsetY + offsetZ;

						//Si cube a traiter
						if(Cubes[indice].getFlag(numFlag))
						{
							//On garde la position de ce cube
							posCubeCentral.X = (float)x + 0.5f;
							posCubeCentral.Y = (float)y + 0.5f;
							posCubeCentral.Z = (float)z + 0.5f;
							posCubeCentral *= TailleCube;

							//On parcourt un hypercube autour du cube
							int xstart = x - rayonHyperCube - 2;
							int ystart = y - rayonHyperCube - 2;
							int zstart = z - rayonHyperCube - 2;
							int nbcubes = rayonHyperCube*2;
							int xfin = xstart + nbcubes + 2;
							int yfin = ystart + nbcubes + 2;
							int zfin = zstart + nbcubes + 2;

							if(xstart < 0) xstart = 0;
							if(ystart < 0) ystart = 0;
							if(zstart < 0) zstart = 0;
							if(xfin >= NbX) xfin = NbX-1;
							if(yfin >= NbY) yfin = NbY-1;
							if(zfin >= NbZ) zfin = NbZ-1;

							//On efface tout les cubes pleins de l'hyper cube qui sont a une bonne distance
							NYVert3Df posSommet;
							int indiceLocal = 0;
							int codeLocal =0;
							for(int z2 = zstart; z2 <= zfin; z2++)
							{
								for(int y2 = ystart; y2 <= yfin; y2++)
								{
									for(int x2 = xstart; x2 <= xfin; x2++)
									{
										indiceLocal = x2+y2*NbX+z2*(NbX*NbY);
										if(Cubes[indiceLocal].getFlag(numFlag) == false)
										{
											posSommet.X = (float)x2;
											posSommet.Y = (float)y2;
											posSommet.Z = (float)z2;
											posSommet *= TailleCube;
											posSommet -= posCubeCentral;
											//On efface tout ce qui se trouve a portée
											if(posSommet.getMagnitude() <= magDist)
												valideSommet(false,x2,y2,z2);
												//valideSommetSphere(false,x2,y2,z2,posCubeCentral,radius,0); //Si on veut interpoler, mais deja ca coute cher...
										}
									}
								}
							}

							//On efface les sommets du cube de surface qu'on vient de traiter
							valideSommet(false,x,y,z);
							valideSommet(false,x+1,y,z);
							valideSommet(false,x+1,y+1,z);
							valideSommet(false,x,y+1,z);
							valideSommet(false,x,y,z+1);
							valideSommet(false,x+1,y,z+1);
							valideSommet(false,x+1,y+1,z+1);
							valideSommet(false,x,y+1,z+1);

							Cubes[indice].setFlag(numFlag,false);
						}
					}
				}
			}
		}


		//On donne la valeur d'un sommet, il la répercute aux cubes correspondants (huit cubes qui partagent le meme sommet)
		void valideSommetSphere(bool value, int x, int y, int z, NYVert3Df & centre, float rayon, float colorShift)
		{
			//Chaque sommet touche potentiellement 8 cubes
			int xprev = x-1;
			int yprev = y-1;
			int zprev = z-1;

			//On fait les 8 cas
			if(z<NbZ)
			{
				if(x<NbX && y<NbY)
					Cubes[x     + y     * NbX + z * (NbX*NbY)].setSommetSphere(0,value,centre,rayon,colorShift);
				if(xprev>=0 && y<NbY)
					Cubes[xprev + y     * NbX + z * (NbX*NbY)].setSommetSphere(1,value,centre,rayon,colorShift);
				if(xprev>=0 && yprev>=0)
					Cubes[xprev + yprev * NbX + z * (NbX*NbY)].setSommetSphere(2,value,centre,rayon,colorShift);
				if(x<NbX && yprev>=0)
					Cubes[x     + yprev * NbX + z * (NbX*NbY)].setSommetSphere(3,value,centre,rayon,colorShift);
			}

			if(zprev>=0)
			{
				if(x<NbX && y<NbY)
				  Cubes[x     + y     * NbX + zprev * (NbX*NbY)].setSommetSphere(4,value,centre,rayon,colorShift);
				if(xprev>=0 && y<NbY)
				  Cubes[xprev + y     * NbX + zprev * (NbX*NbY)].setSommetSphere(5,value,centre,rayon,colorShift);
				if(xprev>=0 && yprev>=0)
				  Cubes[xprev + yprev * NbX + zprev * (NbX*NbY)].setSommetSphere(6,value,centre,rayon,colorShift);
				if(x<NbX && yprev>=0)
				  Cubes[x     + yprev * NbX + zprev * (NbX*NbY)].setSommetSphere(7,value,centre,rayon,colorShift);
			}
		}

		//On donne la valeur d'un sommet, il la répercute aux cubes correspondants (huit cubes qui partagent le meme sommet)
		void valideSommet(bool value, int x, int y, int z)
		{
			//Chaque sommet touche potentiellement 8 cubes
			int xprev = x-1;
			int yprev = y-1;
			int zprev = z-1;


			//On fait les 8 cas
			if(z<NbZ)
			{
				if(x<NbX && y<NbY)
					Cubes[x     + y     * NbX + z * (NbX*NbY)].setSommet(0,value);
				if(xprev>=0 && y<NbY)
					Cubes[xprev + y     * NbX + z * (NbX*NbY)].setSommet(1,value);
				if(xprev>=0 && yprev>=0)
					Cubes[xprev + yprev * NbX + z * (NbX*NbY)].setSommet(2,value);
				if(x<NbX && yprev>=0)
					Cubes[x     + yprev * NbX + z * (NbX*NbY)].setSommet(3,value);
			}

			if(zprev>=0)
			{
				if(x<NbX && y<NbY)
					Cubes[x     + y     * NbX + zprev * (NbX*NbY)].setSommet(4,value);
				if(xprev>=0 && y<NbY)
					Cubes[xprev + y     * NbX + zprev * (NbX*NbY)].setSommet(5,value);
				if(xprev>=0 && yprev>=0)
					Cubes[xprev + yprev * NbX + zprev * (NbX*NbY)].setSommet(6,value);
				if(x<NbX && yprev>=0)
					Cubes[x     + yprev * NbX + zprev * (NbX*NbY)].setSommet(7,value);
			}
		}

		//On verifie si le sommet a la meme valeur dans tous les cubes
		void checkSommet(int x, int y, int z)
		{
			//Chaque sommet touche potentiellement 8 cubes
			int xprev = x-1;
			int yprev = y-1;
			int zprev = z-1;

			bool values[8];

			for(int i=0;i<8;i++)
				values[i] = false;

			//On fait les 8 cas
			if(z<NbZ)
			{
				if(x<NbX && y<NbY)
					values[0] = Cubes[x     + y     * NbX + z * (NbX*NbY)].getSommet(0);
				if(xprev>=0 && y<NbY)
					values[1] = Cubes[xprev + y     * NbX + z * (NbX*NbY)].getSommet(1);
				if(xprev>=0 && yprev>=0)
					values[2] = Cubes[xprev + yprev * NbX + z * (NbX*NbY)].getSommet(2);
				if(x<NbX && yprev>=0)
					values[3] = Cubes[x     + yprev * NbX + z * (NbX*NbY)].getSommet(3);
			}

			if(zprev>=0)
			{
				if(x<NbX && y<NbY)
				  values[4] = Cubes[x     + y     * NbX + zprev * (NbX*NbY)].getSommet(4);
				if(xprev>=0 && y<NbY)
				  values[5] = Cubes[xprev + y     * NbX + zprev * (NbX*NbY)].getSommet(5);
				if(xprev>=0 && yprev>=0)
				  values[6] = Cubes[xprev + yprev * NbX + zprev * (NbX*NbY)].getSommet(6);
				if(x<NbX && yprev>=0)
				  values[7] = Cubes[x     + yprev * NbX + zprev * (NbX*NbY)].getSommet(7);
			}

			bool val = values[0];
			for(int i=1;i<8;i++)
				if(values[i] != val)
				{
					Log::log(Log::ENGINE_ERROR,("Marching cubes : vertex " +
										toString(i) +
										" has not coherent values").c_str());
				}

		}


	public:
		//Valide tous les sommets comme 'inside' dans un rayon donné
		void valideSphere(NYVert3Df centreCercle, float rayon, float colorShift)
		{
			//Changement de repere passage dans le repère des marching cubes
			centreCercle -= Origin;

			if(centreCercle.X < 0.0f || centreCercle.X > TailleCube * NbX)
				Log::log(Log::ENGINE_ERROR,"Error 1 in valideSphere");
			if(centreCercle.Y < 0.0f || centreCercle.Y > TailleCube * NbY)
				Log::log(Log::ENGINE_ERROR,"Error 2 in valideSphere");
			if(centreCercle.Z < 0.0f || centreCercle.Z > TailleCube * NbZ)
				Log::log(Log::ENGINE_ERROR,"Error 3 in valideSphere");

			//On parcourt un hypercube autour du centre
			int xstart = (int)((centreCercle.X - rayon)/TailleCube);
			int ystart = (int)((centreCercle.Y - rayon)/TailleCube);
			int zstart = (int)((centreCercle.Z - rayon)/TailleCube);
			int nbcubes = (int)(((rayon*2.0f)/TailleCube))+1;
			int xfin = xstart + nbcubes -1;
			int yfin = ystart + nbcubes -1;
			int zfin = zstart + nbcubes -1;

			if(xstart < 0) xstart = 0;
			if(ystart < 0) ystart = 0;
			if(zstart < 0) zstart = 0;
			if(xfin >= NbX) xfin = NbX-1;
			if(yfin >= NbY) yfin = NbY-1;
			if(zfin >= NbZ) zfin = NbZ-1;

			double magRayon = rayon*rayon;
			NYVert3Df posSommet;

			//On fait un premier parcours sans interpolation
			bool interpolation = MCube::Interpolation;
			MCube::SetInterpolation(false);
			for(int z = zstart; z <= zfin; z++)
			{
				for(int y = ystart; y <= yfin; y++)
				{
					for(int x = xstart; x <= xfin; x++)
					{
						//On teste si le sommet est dans la sphère
						posSommet.X = (float)x;
						posSommet.Y = (float)y;
						posSommet.Z = (float)z;
						posSommet *= TailleCube;
						posSommet = centreCercle - posSommet;
						double magDist = posSommet.getMagnitude();
						if(magDist <= magRayon)
						{
							valideSommetSphere(true,x,y,z,posSommet,rayon,colorShift);
						}
					}
				}
			}

			//On fait un second parcours avec interpolation
			if(interpolation)
			{
				MCube::SetInterpolation(interpolation);
				for(int z = zstart; z <= zfin; z++)
				{
					for(int y = ystart; y <= yfin; y++)
					{
						for(int x = xstart; x <= xfin; x++)
						{
							//On teste si le sommet est dans la sphère
							posSommet.X = (float)x;
							posSommet.Y = (float)y;
							posSommet.Z = (float)z;
							posSommet *= TailleCube;
							posSommet = centreCercle - posSommet;
							double magDist = posSommet.getMagnitude();
							if(magDist <= magRayon)
							{
								valideSommetSphere(true,x,y,z,posSommet,rayon,colorShift);
							}
						}
					}
				}
			}
		}

		//Retourne le nombre de facettes qu genere cette sphere, étant donne l'état des cubes en ce moment
		int getNbFacettesSphere(NYVert3Df centreCercle, float rayon)
		{
			//On va compter le nombres de cubes qui ont une facette
			int nbFacettes = 0;

			//Changement de repere passage dans le repère des marching cubes
			centreCercle -= Origin;

			if(centreCercle.X < 0.0f || centreCercle.X > TailleCube * NbX)
				Log::log(Log::ENGINE_ERROR,"Error 1 in getNbFacettesSphere");
			if(centreCercle.Y < 0.0f || centreCercle.Y > TailleCube * NbY)
				Log::log(Log::ENGINE_ERROR,"Error 2 in getNbFacettesSphere");
			if(centreCercle.Z < 0.0f || centreCercle.Z > TailleCube * NbZ)
				Log::log(Log::ENGINE_ERROR,"Error 3 in getNbFacettesSphere");

			//On parcourt un hypercube autour du centre
			int xstart = (int)((centreCercle.X - rayon)/TailleCube);
			int ystart = (int)((centreCercle.Y - rayon)/TailleCube);
			int zstart = (int)((centreCercle.Z - rayon)/TailleCube);
			int nbcubes = (int)(((rayon*2.0f)/TailleCube))+1;
			int xfin = xstart + nbcubes -1;
			int yfin = ystart + nbcubes -1;
			int zfin = zstart + nbcubes -1;

			if(xstart < 0) xstart = 0;
			if(ystart < 0) ystart = 0;
			if(zstart < 0) zstart = 0;
			if(xfin >= NbX) xfin = NbX-1;
			if(yfin >= NbY) yfin = NbY-1;
			if(zfin >= NbZ) zfin = NbZ-1;

			double magRayon = rayon*rayon;
			NYVert3Df posCube;

			for(int z = zstart; z <= zfin; z++)
			{
				for(int y = ystart; y <= yfin; y++)
				{
					for(int x = xstart; x <= xfin; x++)
					{
						//On teste si le cube est proche de la sphere
						posCube.X = (float)x + 0.5f;
						posCube.Y = (float)y + 0.5f;
						posCube.Z = (float)z + 0.5f;
						posCube *= TailleCube;
						posCube = centreCercle - posCube;
						posCube -= NYVert3Df(TailleCube,TailleCube,TailleCube); //On se laisse un peu de marge
						double magDist = posCube.getMagnitude();

						//Si le cube est assez proche de la sphere
						if(magDist <= magRayon)
						{
							uint8 code = Cubes[x     + y * NbX + z * (NbX*NbY)].getCode();
							if(code != 255 && code != 0)
								nbFacettes++;
						}
					}
				}
			}

			return nbFacettes;
		}

		void diffuseColorShift(void)
		{
			for(int z = 0; z < NbZ; z++)
			{
				int offsetZ = z * (NbX*NbY);
				for(int y = 0; y < NbY; y++)
				{
					int offsetY = y * NbX;
					for(int x = 0; x < NbX; x++)
					{
						int indice = x + offsetY + offsetZ;

						//if(colorShift != 0.0f)
							//continue;

						//Pour tous les cubes qui l'entourent
						int zDeb = max(z-2,0);
						int zFin = min(z+2,NbZ-2);
						int yDeb = max(y-2,0);
						int yFin = min(y+2,NbY-2);
						int xDeb = max(x-2,0);
						int xFin = min(x+2,NbX-2);

						float newColorShift = 0.0f;
						float nbAdd = 0.0f;

						for(int z2 = zDeb; z2 <= zFin; z2++)
						{
							int offsetZ2 = z2 * (NbX*NbY);
							for(int y2 = yDeb; y2 <= yFin; y2++)
							{
								int offsetY2 = y2 * NbX;
								for(int x2 = xDeb; x2 <= xFin; x2++)
								{
									int indice2 = x2 + offsetY2 + offsetZ2;

									newColorShift += Cubes[indice2].ColorShift;
									nbAdd++;

								}
							}
						}

						newColorShift /= nbAdd;
						Cubes[indice].Temp = newColorShift;
					}
				}
			}

			//On affecte les nouvelles valeurs
			float minColorShift = Cubes[0].Temp;
			float maxColorShift = Cubes[0].Temp;
			for(int i = 0; i < NbCubes; i++)
			{
				Cubes[i].ColorShift = Cubes[i].Temp;
				if(Cubes[i].ColorShift < minColorShift)
					minColorShift = Cubes[i].ColorShift;
				if(Cubes[i].ColorShift > maxColorShift)
					maxColorShift = Cubes[i].ColorShift;
			}
			Log::log(Log::USER_INFO,("Energy max : "+toString(maxColorShift)+", energy min : "+toString(minColorShift)).c_str());
		}


		//Il faut avoir calculé les normales avant de faire ca
		//CALCUL FAUX : on compte une charge par cube donc varie en fonction du nombe de cubes par atomes....
		void coulombSimplifyColorShift(float solventSize, float * pcent, float pcentWidth)
		{
			/*Log::log(Log::ENGINE_INFO,"Simplify color shift");

			//On en a besoin pour le cote de la face
			//Si on calcule les normales, on les lisse forcément (sinon a la volée si pas lissée, économise mémoire)
			setLissageNormales(true);
			computeNormals();

			int sizeConvolution = (int)(5.0f / TailleCube)+1;

			float pcentStep = pcentWidth / (float)NbCubes;

			for(int z = 0; z < NbZ; z++)
			{
				int offsetZ = z * (NbX*NbY);
				for(int y = 0; y < NbY; y++)
				{
					int offsetY = y * NbX;
					for(int x = 0; x < NbX; x++)
					{
						*pcent += pcentStep;

						int indice = x + offsetY + offsetZ;

						uint8 code = Cubes[indice].getCode();
						if(code == 255 || code == 0)
							continue;

						//On recup la normale globale du cube en cours
						NYVert3Df normaleGlobale;
						Cubes[indice].getGlobalNormal(normaleGlobale);
						normaleGlobale *= solventSize;

						//On place un point a solventSize angstrom de la surface
						NYVert3Df base = (NYVert3Df((float)x,(float)y,(float)z)*TailleCube) + normaleGlobale; //Origine du repère pas origine world, mais on travaille en relatif

						//Pour tous les cubes qui l'entourent
						int zDeb = max(z-sizeConvolution,0);
						int zFin = min(z+sizeConvolution,NbZ-1);
						int yDeb = max(y-sizeConvolution,0);
						int yFin = min(y+sizeConvolution,NbY-1);
						int xDeb = max(x-sizeConvolution,0);
						int xFin = min(x+sizeConvolution,NbX-1);

						float sumColorShift = 0.0f;
						float coeffSum = 0.0f;

						for(int z2 = zDeb; z2 <= zFin; z2++)
						{
							int offsetZ2 = z2 * (NbX*NbY);
							for(int y2 = yDeb; y2 <= yFin; y2++)
							{
								int offsetY2 = y2 * NbX;
								for(int x2 = xDeb; x2 <= xFin; x2++)
								{
									int indice2 = x2 + offsetY2 + offsetZ2;

									//Si c'est un cube de surface
									code = Cubes[indice2].getCode();
									if(code != 0)
									{
										//Distance de la référence à ce cube
										float dist = (base - (NYVert3Df((float)x2,(float)y2,(float)z2)*TailleCube)).getSize();
										coeffSum += 1.0f/dist;
										coeffSum++;
										sumColorShift += Cubes[indice2].ColorShift;//distCarre;
									}
								}
							}
						}

						sumColorShift /= coeffSum;
						Cubes[indice].Temp = sumColorShift;

					}
				}
			}


			//On affecte les nouvelles valeurs
			float minColorShift = Cubes[0].Temp;
			float maxColorShift = Cubes[0].Temp;
			for(int i = 0; i < NbCubes; i++)
			{
				Cubes[i].ColorShift = Cubes[i].Temp;
				if(Cubes[i].ColorShift < minColorShift)
					minColorShift = Cubes[i].ColorShift;
				if(Cubes[i].ColorShift > maxColorShift)
					maxColorShift = Cubes[i].ColorShift;
			}
			Log::log(Log::USER_INFO,("Energy max : "+toString(maxColorShift)+", energy min : "+toString(minColorShift)).c_str());
			*/
		}

		void getCubeBarycentreCoords(NYVert3Df & point, int x, int y, int z)
		{
			Cubes[z*(NbX*NbY)+y*NbX+x].getFaceCenter(point);
			point.X += x;
			point.Y += y;
			point.Z += z;
			point *= TailleCube;
			point += Origin;
		}

		void getCubeCenter(NYVert3Df & point, int x, int y, int z)
		{
			point.X = x*TailleCube + TailleCube/2.0f;
			point.Y = y*TailleCube + TailleCube/2.0f;
			point.Z = z*TailleCube + TailleCube/2.0f;
			point += Origin;
		}

		uint8 getCubeCode(int x, int y, int z)
		{
			return Cubes[z*(NbX*NbY)+y*NbX+x].getCode();
		}

		void setCubeColorShift(float shift, int x, int y, int z)
		{
			Cubes[z*(NbX*NbY)+y*NbX+x].ColorShift = shift;
		}

		void getCubeFaceNormal(NYVert3Df & normal,  int x, int y, int z)
		{
			Cubes[z*(NbX*NbY)+y*NbX+x].getGlobalNormal(normal);
		}


        //Permet de recup une valeur lisse du color shift, en fonction des voisins du cube pour cet edgge
		float getCubeEdgeColorShift(int x, int y, int z, int edge)
        {



            float colorShift = Cubes[x + y * NbX + z * (NbX*NbY)].ColorShift;

            /*if((edge == 0 || edge == 2 || edge == 6 || edge == 4 ||
                //edge == 8 || edge == 9 || edge == 10 || edge == 11 ||
                edge == 1 || edge == 3 || edge == 5 || edge == 7))
                return colorShift;*/


            float nbAdd = 1.f;

            //On somme les 3 voisins
            for(int j=0;j<3;j++)
            {
                int xAdd = MCube::VoisinFromEdge[edge*(3*4)+(j*4)+0]+x;
                int yAdd = MCube::VoisinFromEdge[edge*(3*4)+(j*4)+1]+y;
                int zAdd = MCube::VoisinFromEdge[edge*(3*4)+(j*4)+2]+z;

                if(xAdd<0 || xAdd>=NbX || yAdd<0 || yAdd>=NbY || zAdd<0 || zAdd >= NbZ)
                    continue;

                int indiceAdd = xAdd + yAdd * NbX + zAdd * (NbX*NbY);

                //Si c'est pas un cube de surface
                uint8 code = Cubes[indiceAdd].getCode();
                if(code == 255 || code == 0)
                    continue;


                //On somme
                colorShift += Cubes[indiceAdd].ColorShift;
                nbAdd++;
            }

            if(nbAdd > 0)
                colorShift /= nbAdd;

            return colorShift;
        }

	private:

		//Permet de précalculer des normales lissées (sinon on les calcule dans le make geometry)
		void lissageNormales(void)
		{
			Log::log(Log::ENGINE_INFO,"Lissage des normales");

			//On parcourt tous les cubes
			for(int z = 0; z < NbZ; z++)
			{
				int offsetZ = z * (NbX*NbY);
				for(int y = 0; y < NbY; y++)
				{
					int offsetY = y * NbX;
					for(int x = 0; x < NbX; x++)
					{
						int indice = x + offsetY + offsetZ;
						Cubes[indice].calcNormals();
					}
				}
			}

			//On a les normales calculées
			//On va les moyenner
			//On parcourt tous les cubes
			NYVert3Df somme; //Pour stoquer la somme
			NYVert3Df normal; //Pour requp la normale
			for(int z = 0; z < NbZ; z++)
			{
				int offsetZ = z * (NbX*NbY);
				for(int y = 0; y < NbY; y++)
				{
					int offsetY = y * NbX;
					for(int x = 0; x < NbX; x++)
					{
						int indice = x + offsetY + offsetZ;

						uint8 code = Cubes[indice].getCode();

						//Si c'est un cube de surface
						if(code != 255 && code != 0)
						{
							//On fait tous les edges : on somme avec les voisins (et on leur affecte)
							for(int i=0;i<12;i++)
							{
								somme.X = 0;
								somme.Y = 0;
								somme.Z = 0;

								//On somme les 3 voisins
								for(int j=0;j<3;j++)
								{
									int xAdd = MCube::VoisinFromEdge[i*(3*4)+(j*4)+0]+x;
									int yAdd = MCube::VoisinFromEdge[i*(3*4)+(j*4)+1]+y;
									int zAdd = MCube::VoisinFromEdge[i*(3*4)+(j*4)+2]+z;
									int vert = MCube::VoisinFromEdge[i*(3*4)+(j*4)+3];

									if(xAdd<0 || xAdd>=NbX || yAdd<0 || yAdd>=NbY || zAdd<0 || zAdd >= NbZ)
										continue;

									int indiceAdd = xAdd + yAdd * NbX + zAdd * (NbX*NbY);

									//Si c'est pas un cube de surface
									uint8 code = Cubes[indiceAdd].getCode();
                                    if(code == 255 || code == 0)
                                        continue;

									Cubes[indiceAdd].getEdgeNormal(normal,vert);

									somme += normal;
								}

                                if(somme.getMagnitude())
                                    somme.normalize();

								//On affecte aux 3 voisins
								for(int j=0;j<3;j++)
								{
									int xAdd = MCube::VoisinFromEdge[i*(3*4)+(j*4)+0]+x;
									int yAdd = MCube::VoisinFromEdge[i*(3*4)+(j*4)+1]+y;
									int zAdd = MCube::VoisinFromEdge[i*(3*4)+(j*4)+2]+z;
									int vert = MCube::VoisinFromEdge[i*(3*4)+(j*4)+3];

									if(xAdd<0 || xAdd>=NbX || yAdd<0 || yAdd>=NbY || zAdd<0 || zAdd >= NbZ)
										continue;

									int indiceAdd = xAdd + yAdd * NbX + zAdd * (NbX*NbY);

									Cubes[indiceAdd].setEdgeNormal(somme,vert);
								}
							}
						}
					}
				}
			}

			_NormalsPrecomputed = true;
		}

	public:

		/**
		  * A n'appeler que si la géométrie a été généree
		  */
		void saveToBinFile(const char * file)
		{
			FILE * fs = fopen(file,"wb");
			if(fs == NULL)
			{
				Log::log(Log::ENGINE_ERROR,("Cannot save molecule to binary file "+toString(file)).c_str());
			}
			else
			{
				unsigned char * pt = (unsigned char*)_Vertices;
				for(unsigned long i=0;i<_NbVertices*_VertexFloatsCountVBO*sizeof(float);i++)
				{
					fprintf(fs,"%c",pt[i]);
				}
				fclose(fs);
			}
		}

		/**
		  * A n'appeler que si la géométrie a été généree
		  */
		void saveToObjFile(const char * file)
		{
			FILE * fs = fopen(file,"wb");
			if(fs == NULL)
			{
				Log::log(Log::ENGINE_ERROR,("Cannot save molecule to obj file "+toString(file)).c_str());
			}
			else
			{
				//On sort les vertex
				for(unsigned long i=0;i<_NbVertices*3;i+=3)
				{
					fprintf(fs,"v %f %f %f\n",_Vertices[i],_Vertices[i+2],_Vertices[i+1]); //SENS POUR CULLING UNITY
				}

				fprintf(fs,"\n");

				//On sort les faces
				for(unsigned long i=1;i<=_NbVertices;i+=3)
				{
					fprintf(fs,"f %ld %ld %ld\n",i,i+1,i+2);
				}
				fclose(fs);

				Log::log(Log::ENGINE_INFO,("Saved to obj file "+toString(file)).c_str());
			}
		}

		void computeNormals(void)
		{
			if(_PrecomputeNormals && !_NormalsPrecomputed)
				lissageNormales();
		}

		void makeGeometryFaces(const NYVert3Df & translateOrigin)
		{
			//On se recalcule les normales si on est en mode interpolation et donc si les edges sont deja calcules
			computeNormals();

			//Init des constantes pour le rendu
			_VertexFloatsCountVBO = 4+3+3; //Taille d'un vertex en nombre de floats dans le VBO

			//On calcule le nombre de triangles qu'on va devoir stoquer
			int nbTriangles = 0;
			for(int i=0;i<NbCubes;i++)
				nbTriangles += Cubes[i].getNbTriangles();

			//D'ou le nombre de sommets
			_NbVertices = nbTriangles * 3;

			//On fera un tableau type C4F_V3F
			if(_Vertices != NULL)
				SAFEDELETE_TAB(_Vertices);
			_Vertices = new float[_VertexFloatsCountVBO * _NbVertices];
			if(_Vertices == NULL)
			{
				Log::log(Log::ENGINE_ERROR,"Memory allocation failed for _Vertices of MarchingCubes");
				return;
			}

			//On construit le tableau de vertices
			float * ptVertices = _Vertices;

			//On prepare les couleurs
			NYColor colorMaxPos(COLOR_MAX_POS_R,COLOR_MAX_POS_V,COLOR_MAX_POS_B,1.0f);
			NYColor colorMaxNeg(COLOR_MAX_NEG_R,COLOR_MAX_NEG_V,COLOR_MAX_NEG_B,1.0f);
			NYColor colorBase(COLOR_BASE_R,COLOR_BASE_V,COLOR_BASE_B,1.0f);

			//On parcourt tous les cubes
			for(int z = 0; z < NbZ; z++)
			{
				int offsetZ = z * (NbX*NbY);
				for(int y = 0; y < NbY; y++)
				{
					int offsetY = y * NbX;
					for(int x = 0; x < NbX; x++)
					{
						int indice = x + offsetY + offsetZ;

						uint8 code = Cubes[indice].getCode();
						int offset = code*15;

                        //Si pas cube de surface
                        if(code == 0 || code == 255)
                            continue;

						//On ajoute les triangles
						int i=0;

						//On fait face par face
						NYVert3Df vertice[3];
						NYVert3Df normals[3];
						float colorShifts[3];
						while(MCube::TianglesPerCode[offset + i] != -1 && i < 15)
						{
							//On chope une face
							for(int j=0;j<3;j++)
							{
								//On chope le numéro de vertice
								int vertNum = MCube::TianglesPerCode[offset + i + j];

								//On en récup le point correspondant, dans l'espace d'un cube placé à l'origine
								Cubes[indice].getEdgeVertice(vertice[j],vertNum);

								//On place le vertice dans le bon espace (on le translate au niveau du cube concerne)
								vertice[j].X += x * TailleCube;
								vertice[j].Y += y * TailleCube;
								vertice[j].Z += z * TailleCube;

								vertice[j] += translateOrigin;
								vertice[j] += Origin;

								colorShifts[j] = getCubeEdgeColorShift(x,y,z,vertNum);


								//Si normale précalculée on la chope
								if(_NormalsPrecomputed)
								{
									Cubes[indice].getEdgeNormal(normals[j],vertNum);
									normals[j].normalize(); //On les a pas re normalisées lors du lissage
								}
							}


							//Si pas calculées, on la calcule a la volee
							if(!_NormalsPrecomputed)
							{
								//On calcule la normale
								NYVert3Df v1 = vertice[1] - vertice[0];
								NYVert3Df v2 = vertice[2] - vertice[0];
								NYVert3Df normal = v2.vecProd(v1);
								normal.normalize();

								//Ca sera la meme pour les trois
								for(int j=0;j<3;j++)
									normals[j] = normal;
							}


							//On ajoute la face
							for(int j=0;j<3;j++)
							{
                                float colorShift = colorShifts[2-j];
                                //colorShift = Cubes[indice].ColorShift ;
								if(colorShift > 0)
								{
									/**ptVertices = COLOR_BASE_R-(colorShift*2.0f) ; ptVertices++;
									*ptVertices = COLOR_BASE_V-(colorShift*2.0f) ; ptVertices++;
									*ptVertices = COLOR_BASE_B; ptVertices++;*/
									NYColor res = colorBase.interpolate(colorMaxPos,colorShift);
									*ptVertices = res.R ; ptVertices++;
									*ptVertices = res.V ; ptVertices++;
									*ptVertices = res.B ; ptVertices++;
								}

								if(colorShift < 0)
								{
									NYColor res = colorBase.interpolate(colorMaxNeg,-colorShift);
									*ptVertices = res.R ; ptVertices++;
									*ptVertices = res.V ; ptVertices++;
									*ptVertices = res.B ; ptVertices++;
								}

								if(colorShift == 0)
								{
									*ptVertices = COLOR_BASE_R; ptVertices++;
									*ptVertices = COLOR_BASE_V; ptVertices++;
									*ptVertices = COLOR_BASE_B; ptVertices++;
								}

								*ptVertices = 1.0f; ptVertices++;

								//Le bon sens pour GL_CULLFACES

								*ptVertices = normals[2-j].X; ptVertices++;
								*ptVertices = normals[2-j].Y; ptVertices++;
								*ptVertices = normals[2-j].Z; ptVertices++;

								*ptVertices = vertice[2-j].X; ptVertices++;
								*ptVertices = vertice[2-j].Y; ptVertices++;
								*ptVertices = vertice[2-j].Z; ptVertices++;
							}

							i+=3;
						}
					}
				}
			}


			//On cree le VAO

			//détruit si existe
			if(_VAO != 0)
				glDeleteBuffers(1, &_VAO);
			_VAO = 0;

			//Generation du VAO
			glGenVertexArrays(1, &_VAO);

			//On bind
			glBindVertexArray(_VAO);

			//On cree les VBO
			//On détruit si existe
			if(_BufGeom != 0)
				glDeleteBuffers(1, &_BufGeom);
			_BufGeom = 0;

			//Generation des buffers
			glGenBuffers(1, &_BufGeom);

			NYRenderer::checkGlError("glGenBuffers(1, &_BufGeom);");

			//On met les vertices
			glBindBuffer(GL_ARRAY_BUFFER, _BufGeom);

			NYRenderer::checkGlError("glBindBuffer(GL_ARRAY_BUFFER, _BufGeom);");

			glBufferData(GL_ARRAY_BUFFER, _NbVertices*_VertexFloatsCountVBO*sizeof(float), _Vertices, GL_STREAM_DRAW);

			NYRenderer::checkGlError("glBufferData(GL_ARRAY_BUFFER, _NbAtomes*_VertexFloatsCountVBO*sizeof(float), _Vertices, GL_STREAM_DRAW);");

			//On debind
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

			//On detruit pas pour pouvoir ensuite sauver dans un fichier si on veut
			//On detruit les buffers
			//SAFEDELETE(_Vertices);
		}

		void makeVerticesOnlyBuffer(const NYVert3Df & translateOrigin)
		{
			//On calcule le nombre de triangles qu'on va devoir stoquer
			int nbTriangles = 0;
			for(int i=0;i<NbCubes;i++)
				nbTriangles += Cubes[i].getNbTriangles();

			//D'ou le nombre de sommets
			_NbVertices = nbTriangles * 3;

			//On fait un tableau type V3F seulement
			_Vertices = new float[3 * _NbVertices];
			if(_Vertices == NULL)
			{
				Log::log(Log::ENGINE_ERROR,"Memory allocation failed for _Vertices of MarchingCubes");
				return;
			}

			//On construit le tableau de vertices
			float * ptVertices = _Vertices;

			//On parcourt tous les cubes
			for(int z = 0; z < NbZ; z++)
			{
				int offsetZ = z * (NbX*NbY);
				for(int y = 0; y < NbY; y++)
				{
					int offsetY = y * NbX;
					for(int x = 0; x < NbX; x++)
					{
						int indice = x + offsetY + offsetZ;

						uint8 code = Cubes[indice].getCode();
						int offset = code*15;

						//On ajoute les triangles
						int i=0;

						//On fait face par face
						NYVert3Df vertice[3];
						while(MCube::TianglesPerCode[offset + i] != -1 && i < 15)
						{
							//On chope une face
							for(int j=0;j<3;j++)
							{
								//On chope le numéro de vertice
								int vertNum = MCube::TianglesPerCode[offset + i + j];

								//On en récup le point correspondant, dans l'espace d'un cube placé à l'origine
								Cubes[indice].getEdgeVertice(vertice[j],vertNum);

								//On place le vertice dans le bon espace (on le translate au niveau du cube concerne)
								vertice[j].X += (x * TailleCube);
								vertice[j].Y += (y * TailleCube);
								vertice[j].Z += (z * TailleCube);
							}

							//On ajoute la face
							for(int j=0;j<3;j++)
							{
								*ptVertices = vertice[j].X + Origin.X + translateOrigin.X; ptVertices++;
								*ptVertices = vertice[j].Y + Origin.Y + translateOrigin.Y; ptVertices++;
								*ptVertices = vertice[j].Z + Origin.Z + translateOrigin.Z; ptVertices++;
							}

							i+=3;
						}
					}
				}
			}

			//On detruit pas pour pouvoir ensuite sauver dans un fichier si on veut
			//On detruit les buffers
			//SAFEDELETE(_Vertices);
		}

		void destroyTempGeometry(void)
		{
			SAFEDELETE_TAB(_Vertices);
		}


		//Fait une geom de debug, avec plein de points
		void makeGeometry(void)
		{
			//Init des constantes pour le rendu
			_VertexFloatsCountVBO = 4+3; //Taille d'un vertex en nombre de floats dans le VBO

			//On convertit les points en géometrie affichable
			int nbPoints = NbCubes * 8;

			//On fera un tableau type C4F_V3F
			_Vertices = new float[_VertexFloatsCountVBO * nbPoints];
			_NbVertices = nbPoints;
			if(_Vertices == NULL)
			{
				Log::log(Log::ENGINE_ERROR,"Memory allocation failed for _Vertices of MarchingCubes");
				return;
			}

			//On construit le tableau de vertices
			float * ptVertices = _Vertices;

			//On parcourt tous les cubes
			for(int z = 0; z < NbZ; z++)
			{
				int offsetZ = z * (NbX*NbY);
				for(int y = 0; y < NbY; y++)
				{
					int offsetY = y * NbX;
					for(int x = 0; x < NbX; x++)
					{
						int indice = x + offsetY + offsetZ;

						//point 1
						*ptVertices = Cubes[indice].getSommet(0) ? 0.0f : 1.0f; ptVertices++;
						*ptVertices = Cubes[indice].getSommet(0) ? 1.0f : 0.0f; ptVertices++;
						*ptVertices = Cubes[indice].getSommet(0) ? 0.0f : 0.0f; ptVertices++;
						*ptVertices = 1.0f; ptVertices++;

						*ptVertices = x*TailleCube; ptVertices++;
						*ptVertices = y*TailleCube; ptVertices++;
						*ptVertices = z*TailleCube; ptVertices++;

						//point 2
						*ptVertices = Cubes[indice].getSommet(1) ? 0.0f : 1.0f; ptVertices++;
						*ptVertices = Cubes[indice].getSommet(1) ? 1.0f : 0.0f; ptVertices++;
						*ptVertices = Cubes[indice].getSommet(1) ? 0.0f : 0.0f; ptVertices++;
						*ptVertices = 1.0f; ptVertices++;

						*ptVertices = (x+0.9f)*TailleCube; ptVertices++;
						*ptVertices = y*TailleCube; ptVertices++;
						*ptVertices = z*TailleCube; ptVertices++;

						//point 3
						*ptVertices = Cubes[indice].getSommet(2) ? 0.0f : 1.0f; ptVertices++;
						*ptVertices = Cubes[indice].getSommet(2) ? 1.0f : 0.0f; ptVertices++;
						*ptVertices = Cubes[indice].getSommet(2) ? 0.0f : 0.0f; ptVertices++;
						*ptVertices = 1.0f; ptVertices++;

						*ptVertices = (x+0.9f)*TailleCube; ptVertices++;
						*ptVertices = (y+0.9f)*TailleCube; ptVertices++;
						*ptVertices = z*TailleCube; ptVertices++;

						//point 4
						*ptVertices = Cubes[indice].getSommet(3) ? 0.0f : 1.0f; ptVertices++;
						*ptVertices = Cubes[indice].getSommet(3) ? 1.0f : 0.0f; ptVertices++;
						*ptVertices = Cubes[indice].getSommet(3) ? 0.0f : 0.0f; ptVertices++;
						*ptVertices = 1.0f; ptVertices++;

						*ptVertices = x*TailleCube; ptVertices++;
						*ptVertices = (y+0.9f)*TailleCube; ptVertices++;
						*ptVertices = z*TailleCube; ptVertices++;

						//point 5
						*ptVertices = Cubes[indice].getSommet(4) ? 0.0f : 1.0f; ptVertices++;
						*ptVertices = Cubes[indice].getSommet(4) ? 1.0f : 0.0f; ptVertices++;
						*ptVertices = Cubes[indice].getSommet(4) ? 0.0f : 0.0f; ptVertices++;
						*ptVertices = 1.0f; ptVertices++;

						*ptVertices = x*TailleCube; ptVertices++;
						*ptVertices = y*TailleCube; ptVertices++;
						*ptVertices = (z+0.9f)*TailleCube; ptVertices++;

						//point 6
						*ptVertices = Cubes[indice].getSommet(5) ? 0.0f : 1.0f; ptVertices++;
						*ptVertices = Cubes[indice].getSommet(5) ? 1.0f : 0.0f; ptVertices++;
						*ptVertices = Cubes[indice].getSommet(5) ? 0.0f : 0.0f; ptVertices++;
						*ptVertices = 1.0f; ptVertices++;

						*ptVertices = (x+0.9f)*TailleCube; ptVertices++;
						*ptVertices = y*TailleCube; ptVertices++;
						*ptVertices = (z+0.9f)*TailleCube; ptVertices++;

						//point 7
						*ptVertices = Cubes[indice].getSommet(6) ? 0.0f : 1.0f; ptVertices++;
						*ptVertices = Cubes[indice].getSommet(6) ? 1.0f : 0.0f; ptVertices++;
						*ptVertices = Cubes[indice].getSommet(6) ? 0.0f : 0.0f; ptVertices++;
						*ptVertices = 1.0f; ptVertices++;

						*ptVertices = (x+0.9f)*TailleCube; ptVertices++;
						*ptVertices = (y+0.9f)*TailleCube; ptVertices++;
						*ptVertices = (z+0.9f)*TailleCube; ptVertices++;

						//point 8
						*ptVertices = Cubes[indice].getSommet(7) ? 0.0f : 1.0f; ptVertices++;
						*ptVertices = Cubes[indice].getSommet(7) ? 1.0f : 0.0f; ptVertices++;
						*ptVertices = Cubes[indice].getSommet(7) ? 0.0f : 0.0f; ptVertices++;
						*ptVertices = 1.0f; ptVertices++;

						*ptVertices = x*TailleCube; ptVertices++;
						*ptVertices = (y+0.9f)*TailleCube; ptVertices++;
						*ptVertices = (z+0.9f)*TailleCube; ptVertices++;
					}
				}
			}

			//On cree le VAO

			//détruit si existe
			if(_VAO != 0)
				glDeleteBuffers(1, &_VAO);
			_VAO = 0;

			//Generation du VAO
			glGenVertexArrays(1, &_VAO);

			//On bind
			glBindVertexArray(_VAO);

			//On cree les VBO
			//On détruit si existe
			if(_BufGeom != 0)
				glDeleteBuffers(1, &_BufGeom);
			_BufGeom = 0;

			//Generation des buffers
			glGenBuffers(1, &_BufGeom);

			//On met les vertices
			glBindBuffer(GL_ARRAY_BUFFER, _BufGeom);

			glBufferData(GL_ARRAY_BUFFER, nbPoints*_VertexFloatsCountVBO*sizeof(float), _Vertices, GL_STREAM_DRAW);

			NYRenderer::checkGlError("glBufferData(GL_ARRAY_BUFFER, _NbAtomes*_AtomVertexArraySize*sizeof(float), _Vertices, GL_STREAM_DRAW);");

			//On debind
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

			//On detruit les buffers
			//SAFEDELETE(_Vertices);
		}

		void render(void)
		{
			/*glPushMatrix();
			glTranslatef(Origin.X,Origin.Y,Origin.Z);

			glEnable(GL_COLOR_MATERIAL);
			glEnable(GL_LIGHTING);

			glBindBuffer(GL_ARRAY_BUFFER, _BufGeom);

			// activation des tableaux de sommets
			glEnableClientState(GL_COLOR_ARRAY);
			glEnableClientState(GL_VERTEX_ARRAY);

			glColorPointer(4, GL_FLOAT,_VertexFloatsCountVBO * sizeof(float), BUFFER_OFFSET(0));

			NYRenderer::checkGlError("glColorPointer(4, GL_FLOAT,_VertexFloatsCountVBO*sizeof(float), BUFFER_OFFSET(0));");

			glVertexPointer(3, GL_FLOAT,_VertexFloatsCountVBO * sizeof(float),BUFFER_OFFSET(4*sizeof(float)));

			NYRenderer::checkGlError("glVertexPointer(3, GL_FLOAT,_VertexFloatsCountVBO*sizeof(float),BUFFER_OFFSET(7*sizeof(float))); ");

			glDrawArrays(GL_POINTS,0,_NbVertices);
			NYRenderer::checkGlError("glDrawArrays(GL_POINTS,0,_NbVertices);");

			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glDisableClientState(GL_COLOR_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY);

			glDisable(GL_COLOR_MATERIAL);
			glDisable(GL_LIGHTING);
			glPopMatrix();*/
		}

		void renderFaces(void)
		{
			glPushMatrix();
			//glTranslatef(Origin.X,Origin.Y,Origin.Z);

			glEnable(GL_COLOR_MATERIAL);
			glEnable(GL_LIGHTING);

			glBindBuffer(GL_ARRAY_BUFFER, _BufGeom);
			glBindVertexArray(_VAO);

			// activation des tableaux de sommets
			/*glEnableClientState(GL_COLOR_ARRAY);
			glEnableClientState(GL_NORMAL_ARRAY);
			glEnableClientState(GL_VERTEX_ARRAY);

			glColorPointer(4, GL_FLOAT,_VertexFloatsCountVBO * sizeof(float), BUFFER_OFFSET(0));
			NYRenderer::checkGlError("glColorPointer(4, GL_FLOAT,_VertexFloatsCountVBO*sizeof(float), BUFFER_OFFSET(0));");

			glNormalPointer( GL_FLOAT,_VertexFloatsCountVBO * sizeof(float),BUFFER_OFFSET(4*sizeof(float)));
			NYRenderer::checkGlError("glNormalPointer( GL_FLOAT,_VertexFloatsCountVBO * sizeof(float),BUFFER_OFFSET(4*sizeof(float)));");

			glVertexPointer(3, GL_FLOAT,_VertexFloatsCountVBO * sizeof(float),BUFFER_OFFSET(7*sizeof(float)));
			NYRenderer::checkGlError("glVertexPointer(3, GL_FLOAT,_VertexFloatsCountVBO*sizeof(float),BUFFER_OFFSET(7*sizeof(float))); ");*/

			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 10*sizeof(float), BUFFER_OFFSET(0));
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 10*sizeof(float), BUFFER_OFFSET(4*sizeof(float)));
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 10*sizeof(float), BUFFER_OFFSET(7*sizeof(float)));

			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);

			NYRenderer::getInstance()->sendOglMatToShader();

			glDrawArrays(GL_TRIANGLES,0,_NbVertices);
			NYRenderer::checkGlError("glDrawArrays(GL_TRIANGLES,0,_NbVertices);");

			glDisable(GL_COLOR_MATERIAL);
			glDisable(GL_LIGHTING);
			glPopMatrix();
		}

};

#endif
