#ifndef __LOG_ACTIONS_READER_H__
#define __LOG_ACTIONS_READER_H__

#include <string>
#include <stdio.h>
#include "engine/utils/types_3d.h"
#include "engine/network/http_manager.h"
#include "global.h"
#include "log/log_actions.h"
#include "molecules/marching_cubes/molecule_cubes.h"

#define NB_PROTS 8

class LogActionsReader
{
	private:
		class Action
		{
			public :
				int EvtNum;
				uint32 Time;
				int IdUser;

				void readEntete(FILE * fe)
				{
					EvtNum = 0;
					Time = 0;
					IdUser = 0;
					fread(&EvtNum,sizeof(int),1,fe);
					fread(&Time,sizeof(uint32),1,fe);
					fread(&IdUser,sizeof(int),1,fe);
				}

				Action & operator=(Action & action)
				{
					EvtNum = action.EvtNum;
					Time = action.Time;
					IdUser = action.IdUser;
					return *this;
				}

				virtual void readData(FILE * fe){};
		};

		class ActionStartDocking : public Action
		{
			public :
				int Id1;
				int Id2;

				void readData(FILE * fe)
				{
                    bool error = true;
					if(fread(&Id1,sizeof(int),1,fe) == 1)
					if(fread(&Id2,sizeof(int),1,fe) == 1)
                        error = false;
                    if(!error)
                        Log::log(Log::ENGINE_INFO,("Start log " + toString(Id1) + " " + toString(Id2)).c_str());
                    else
                        Log::log(Log::ENGINE_INFO,"Unable to read ActionStartDocking from log");
				}
		};

		class ActionComplex : public Action
		{
			public :
				int Id1;
				int Id2;
				Positionator Pos1;
				Positionator Pos2;

				void readData(FILE * fe)
				{
                    float x,y,z;
                    bool error = true;
					if(fread(&Id1,sizeof(int),1,fe) == 1)
					if(fread(&Id2,sizeof(int),1,fe) == 1)
					if(fread(&(Pos1._Pos.X),sizeof(float),1,fe) == 1)
					if(fread(&(Pos1._Pos.Y),sizeof(float),1,fe) == 1)
					if(fread(&(Pos1._Pos.Z),sizeof(float),1,fe) == 1)
					if(fread(&x,sizeof(float),1,fe) == 1)
					if(fread(&y,sizeof(float),1,fe) == 1)
					if(fread(&z,sizeof(float),1,fe) == 1)
                        error = false;

                    if(error)
                        Log::log(Log::USER_ERROR, "Unable to load ActionComplex pos1 from log");

					Pos1._Rot.createRotateXYZ(x,y,z);

                    error = true;
					if(fread(&(Pos2._Pos.X),sizeof(float),1,fe) == 1)
					if(fread(&(Pos2._Pos.Y),sizeof(float),1,fe) == 1)
					if(fread(&(Pos2._Pos.Z),sizeof(float),1,fe) == 1)
					if(fread(&x,sizeof(float),1,fe) == 1)
					if(fread(&y,sizeof(float),1,fe) == 1)
					if(fread(&z,sizeof(float),1,fe) == 1)
                        error = false;

					if(error)
                        Log::log(Log::USER_ERROR, "Unable to load ActionComplex pos2 from log");

					Pos2._Rot.createRotateXYZ(x,y,z);

					Log::log(Log::ENGINE_INFO,("Complex pos " + toString(Id1) + " " + toString(Id2)).c_str());
				}
		};

		class ActionSetHarpoonHandle : public Action
		{
			public :
				int Id1;
				NYVert3Df Pos;

				void readData(FILE * fe)
				{
                    bool error = true;
					if(fread(&Id1,sizeof(int),1,fe) == 1)
					if(fread(&(Pos.X),sizeof(float),1,fe) == 1)
					if(fread(&(Pos.Y),sizeof(float),1,fe) == 1)
					if(fread(&(Pos.Z),sizeof(float),1,fe) == 1)
                        error = false;

                    if(error)
                        Log::log(Log::USER_ERROR, "Unable to load ActionSetHarpoonHandle from log");

					Log::log(Log::ENGINE_INFO,("Harpoon on " + toString(Id1)).c_str());
				}
		};

		class ActionScore : public Action
		{
			public :
				float score;

				void readData(FILE * fe)
				{
                    bool error = true;
					if(fread(&score,sizeof(float),1,fe) == 1)
                        error = false;

                    if(error)
                        Log::log(Log::USER_ERROR, "Unable to load ActionScore from log");

					Log::log(Log::ENGINE_INFO,("Score " + toString(score)).c_str());
				}
		};

		class ActionUseOptim : public Action
		{
			public :
				int Id1;
				int Id2;
				Positionator Pos1;
				Positionator Pos2;

				void readData(FILE * fe)
				{
                    float x,y,z;
                    bool error = true;
					if(fread(&Id1,sizeof(int),1,fe) == 1)
					if(fread(&Id2,sizeof(int),1,fe) == 1)
					if(fread(&(Pos1._Pos.X),sizeof(float),1,fe) == 1)
					if(fread(&(Pos1._Pos.Y),sizeof(float),1,fe) == 1)
					if(fread(&(Pos1._Pos.Z),sizeof(float),1,fe) == 1)
					if(fread(&x,sizeof(float),1,fe) == 1)
					if(fread(&y,sizeof(float),1,fe) == 1)
					if(fread(&z,sizeof(float),1,fe) == 1)
                        error = false;

                    if(error)
                        Log::log(Log::USER_ERROR, "Unable to load ActionUseOptim pos1 from log");

					Pos1._Rot.createRotateXYZ(x,y,z);

                    error = true;
					if(fread(&(Pos2._Pos.X),sizeof(float),1,fe) == 1)
					if(fread(&(Pos2._Pos.Y),sizeof(float),1,fe) == 1)
					if(fread(&(Pos2._Pos.Z),sizeof(float),1,fe) == 1)
					if(fread(&x,sizeof(float),1,fe) == 1)
					if(fread(&y,sizeof(float),1,fe) == 1)
					if(fread(&z,sizeof(float),1,fe) == 1)
                        error = false;

					if(error)
                        Log::log(Log::USER_ERROR, "Unable to load ActionUseOptim pos2 from log");

                    Pos2._Rot.createRotateXYZ(x,y,z);

					Log::log(Log::ENGINE_INFO,("Optim " + toString(Id1) + " " + toString(Id2)).c_str());
				}
		};

		class ActionVersion : public Action
		{
			public :
				int Overall;
				int Major;
				int Minor;

				void readData(FILE * fe)
				{
                    bool error = true;
					if(fread(&Overall,sizeof(int),1,fe) == 1)
					if(fread(&Major,sizeof(int),1,fe) == 1)
					if(fread(&Minor,sizeof(int),1,fe) == 1)
                        error = false;

                    if(error)
                        Log::log(Log::USER_ERROR, "Unable to load ActionVersion pos2 from log");

					Log::log(Log::ENGINE_INFO,("Version " + toString(Overall) + "." + toString(Major)+ "." + toString(Minor)).c_str());
				}
		};

		FILE * _Fe;
		fpos_t _Pos;
		std::vector<Action*> _Actions;

		long _TimeByCouples[NB_PROTS][NB_PROTS];


		/*void closeFile(void)
		{
			if(_Fe != NULL)
			{
				fclose(_Fe);
				_Fe = NULL;
			}
		}*/

		/*void getNextEntete(int * evtNum, DWORD * time, int * idUser)
		{
			fread(evtNum,sizeof(int),1,_Fe);
			fread(time,sizeof(DWORD),1,_Fe);
			fread(idUser,sizeof(int),1,_Fe);
		}

		void saveCurrentPosition(void)
		{
			fgetpos(_Fe,&_Pos);
		}

		void restoreLastPosition(void)
		{
			fsetpos(_Fe,&_Pos);
		}

		void jumpAction(int evtNum)
		{
			char buffer[1000];
			switch(evtNum)
			{
				case 1: fread(buffer,sizeof(int)   * 2 ,1,_Fe); break;
				case 2: fread(buffer,sizeof(int)   * 2 +
									 sizeof(float) * 12 ,1,_Fe); break;
				case 3: fread(buffer,sizeof(int)   * 1 +
									 sizeof(float) * 3 ,1,_Fe); break;
				case 4: fread(buffer,sizeof(float),1,_Fe); break;
				case 5: fread(buffer,sizeof(int)   * 2 +
									 sizeof(float) * 12 ,1,_Fe); break;
				case 6: fread(buffer,sizeof(int)   * 3  ,1,_Fe); break;
			}
		}*/



	public:

		LogActionsReader()
		{
			_Fe = NULL;
		}

		bool addFile(std::string & file)
		{
			_Fe = fopen(file.c_str(),"rb");
			if(_Fe == NULL)
			{
				Log::log(Log::ENGINE_ERROR,"Unable to open log file");
				return true;
			}

			Action * tester = new Action();

			while(!feof(_Fe))
			{
				//On lit l'entete
				tester->readEntete(_Fe);
				if(feof(_Fe))
					break;

				Action * a = NULL;
				switch(tester->EvtNum)
				{
					case 1: a = new ActionStartDocking(); *a=*tester; a->readData(_Fe); break;
					case 2: a = new ActionComplex(); *a=*tester; a->readData(_Fe); break;
					case 3: a = new ActionSetHarpoonHandle(); *a=*tester; a->readData(_Fe); break;
					case 4: a = new ActionScore(); *a=*tester; a->readData(_Fe); break;
					case 5: a = new ActionUseOptim(); *a=*tester; a->readData(_Fe); break;
					case 6: a = new ActionVersion(); *a=*tester; a->readData(_Fe); break;
					default: Log::log(Log::ENGINE_ERROR,"Erreur de lecture du log, evt inconnu.");
				}

				if(a != NULL)
				{
					_Actions.push_back(a);
				}

			}

			fclose(_Fe);
			return false;
		}

		void analyze(void)
		{
			//Temps par couples
			int time= -1;
			int id1 = 0;
			int id2 = 0;
			int idUser = 0;
			memset(_TimeByCouples,0x00,NB_PROTS*NB_PROTS*sizeof(long));
			for(int i=0;i<_Actions.size();i++)
			{
				if(_Actions[i]->EvtNum == 1)
				{
					ActionStartDocking * as = (ActionStartDocking *)(_Actions[i]);
					time = as->Time;
					id1 = as->Id1;
					id2 = as->Id2;
					idUser = as->IdUser;
				}

				if(_Actions[i]->EvtNum != 6 && _Actions[i]->EvtNum != 1)
				{
					if(time > 0 && idUser == _Actions[i]->IdUser)
					{
						_TimeByCouples[id1-1][id2-1] += _Actions[i]->Time - time;
						_TimeByCouples[id2-1][id1-1] += _Actions[i]->Time - time;
						//_cprintf("adding %d time to %d and %d for user %d for event %d\n",_Actions[i]->Time - time,id1,id2,idUser,_Actions[i]->EvtNum);
						time = _Actions[i]->Time;

					}
					else
					{
						time = -1;
					}
				}
			}
			FILE * fs = fopen("logs/time.csv","w");
			if(fs)
			{
				//On sort les temps
				for(int i=0;i<NB_PROTS;i++)
				{
					for(int j=0;j<NB_PROTS;j++)
					{
						fprintf(fs,"%ld",_TimeByCouples[i][j]);
						if(j<NB_PROTS-1)
							fprintf(fs,",");
						else
							fprintf(fs,"\n");
					}
				}
				fclose(fs);
			}



			//On efface le csv
			fs = fopen("logs\\res.csv","w");
			if(fs)
			{
				fprintf(fs,"id action,mol 1,mol 2,recouvrement,energie\n");
				fclose(fs);
			}

			//On efface le csv
			fs = fopen("logs\\top.csv","w");
			if(fs)
			{
				fprintf(fs,"id mol,energie top\n");
				fclose(fs);
			}


			studyMol(1,2);
			studyMol(2,1);
			studyMol(3,4);
			studyMol(4,3);
			studyMol(5,6);
			studyMol(6,5);
			studyMol(7,8);
			studyMol(8,7);


			Log::log(Log::ENGINE_INFO,"Done");
		}

		void studyMol(int id, int idPartenaireVrai)
		{
			MoleculeCubes * mols[2];
			mols[0] = new MoleculeCubes();
			mols[1] = new MoleculeCubes();

			mols[0]->loadFromMOL2((L"molecules/"+toWString(id)+L".mol2").c_str());
			mols[0]->calcSurfaceByAtom(true);

			//L'image sur laquelle on va projetter nos valeurs
			int size = 64;
			float * image = new float[size*size*3];
			memset(image,0x00,size*size*3*sizeof(float));

			mols[1]->loadFromMOL2((L"molecules/"+toWString(idPartenaireVrai)+L".mol2").c_str());

			//On projette sur l'image l'interface avec le vrai partenaire
			double energyTop = projette(mols[0],mols[1],image,size,true);

			//On sort l'energie Top de la bonne interface
			FILE * fs = fopen("logs\\top.csv","a");
			if(fs)
			{
				fprintf(fs,"%d,%f\n",id,energyTop);
				fclose(fs);
			}

			int lostOptim = 0;

			//On regarde les logs
			for(int i=0;i<_Actions.size();i++)
			{
				//Si c'est une optim
				if(_Actions[i]->EvtNum == 5)
				{
					ActionUseOptim * a = (ActionUseOptim*)_Actions[i];

					//Si c'est une optim de moi sur barnase ou barstar, alors on oublie... car je connais
					if(a->IdUser == 1 && (a->Id1 == 1 || a->Id2 == 1 || a->Id1 == 2 || a->Id2 == 2)){
						lostOptim++;
						continue;
					}

					//Si c'est le joueur test on oublie il fait nimp
					if(a->IdUser == 3){
						continue;
					}

					//Si c'est une optim avec la molecule que l'on étudie
					if(a->Id1 == id || a->Id2 == id)
					{
						int idCalc1 = 0;
						int idCalc2 = 0;

						//On charge l'autre mol
						if(a->Id1 == id)
						{
							//mols[1]->releaseTempGeometry();
							mols[1]->loadFromMOL2((L"molecules/"+toWString(a->Id2)+L".mol2").c_str());
							mols[0]->_Transform = a->Pos1;
							mols[1]->_Transform = a->Pos2;
							idCalc1 = a->Id1;
							idCalc2 = a->Id2;
						}
						else
						{
							//mols[1]->releaseTempGeometry();
							mols[1]->loadFromMOL2((L"molecules/"+toWString(a->Id1)+L".mol2").c_str());
							mols[0]->_Transform = a->Pos2;
							mols[1]->_Transform = a->Pos1;
							idCalc1 = a->Id2;
							idCalc2 = a->Id1;
						}

						//On projette l'interface lors de l'optim dans l'image courante
						double recouvrement = 0;
						double energy = projette(mols[0],mols[1],image,size,false, &recouvrement);

						//on balance dans le fichier csv
						FILE * fs = fopen("logs\\res.csv","a");
						if(fs)
						{
							fprintf(fs,"%d,%d,%d,%f,%f\n",i,idCalc1,idCalc2,recouvrement,energy);
							fclose(fs);
						}
					}
				}
			}

			//On sort dans un fichier la valeur de l'accumulateur pour tous les atomes de la mol
			std::string filename = "logs\\" + toString(id) +".csv";
			fs = fopen(filename.c_str(),"w");
			if(fs)
			{
				fprintf(fs,"NbExplo, Surface\n");
				for(int i=0;i<mols[0]->_NbAtomes;i++)
					fprintf(fs,"%f,%f\n",mols[0]->_Atomes[i]._Accumulateur,mols[0]->_Atomes[i]._Surface);
				fclose(fs);
			}

			normalize(image,size,0);
			normalize(image,size,1);
			goodColors(image,size);
			NYTexManager::writeImage(("logs\\res_" + toString(id) + ".png").c_str(),size,size,image,"res");

			Log::log(Log::ENGINE_INFO,("Mol id " + toString(id) + " done (lost "+toString(lostOptim)+" optims cause guigui)").c_str());

			mols[0]->releaseTempGeometry();
			mols[1]->releaseTempGeometry();
			SAFEDELETE(mols[0]);
			SAFEDELETE(mols[1]);
			SAFEDELETE_TAB(image);
		}

		//component : 0 pour rouge, 1 pour vert,2 pour bleu
		void normalize(float * image, int width, int component)
		{
			float max = 0;
			for(int i=0;i<width*width;i++)
			{
				if(image[3*i + component] > max)
					max = image[3*i + component];
			}

			if(max != 0)
			{
				for(int i=0;i<width*width;i++)
				{
					image[3*i +  component] /= max;
				}
			}
		}

		//Permet de partir du blanc plutot que du noir
		void goodColors(float * image, int width)
		{
			for(int i=0;i<width*width;i++)
			{
				//Si c'est pas un atome : blanc
				if(image[3*i + 2] <= 0)
				{
					image[3*i + 0] = 1.0f;
					image[3*i + 1] = 1.0f;
					image[3*i + 2] = 1.0f;
				}
				else
				{
					//Sinon si c'est que un atome pas explore de la bonne interface
					if(image[3*i + 0] == 0 && image[3*i + 1] > 0)
					{
						//bleu
						image[3*i + 0] = 0.0f;
						image[3*i + 1] = 0.0f;
						image[3*i + 2] = 0.8f;
					}
					else
					{
						//Sinon si c'est un atome explore hors de la bonne interface
						if(image[3*i + 0] > 0 && image[3*i + 1] == 0)
						{
							//rouge qui tent vers le gris
							float val = image[3*i + 0];
							image[3*i + 0] = 0.8f;
							image[3*i + 1] = 0.6f - (val*0.6f); //on saute direct
							image[3*i + 2] = 0.6f - (val*0.6f);
						}
						else
						{
							//Sinon il est explore et dans la bonne interface
							if(image[3*i + 0] > 0 && image[3*i + 1] > 0)
							{
								//vert qui tent vers le gris
								float val = image[3*i + 0];
								image[3*i + 0] = 0.6f - (val*0.6f);
								image[3*i + 1] = 0.8f;
								image[3*i + 2] = 0.6f - (val*0.6f);
							}
							else
							{
								//Sinon c'est juste un atome degrade vers le blanc en fonction de sa surface (la valeur)
								float val = image[3*i + 2];
								image[3*i + 0] = 0.7f + (val*0.2f);
								image[3*i + 1] = 0.7f + (val*0.2f);
								image[3*i + 2] = 0.7f + (val*0.2f);
							}
						}
					}
				}
			}
		}


		//bonPartenaire : les atomes sont chargés dans le buffer de rendu et dans le moteur physique
		//avec l'offset du barycentre. Donc si on utilise une potision donnée par l'appli, il faut
		//tenir compte de cet offset. Par contre, si on utilise un position de base (par exemple les
		//atomes dans leur position originale dans le fichier) alors il ne faut plus tenir compte
		//de l'offset du barycentre.
		//incremente : si on ajoute chaque atome à son pixel ou si on met simplement à 1
		//retourne le niveau d'energie
		//RECOUVREMENT
		//Ne fonctionne que si on a deja appele la fonciton avec bonPartenaire, sur la meme image
		//la fonction calcule la part d'atomes de l'interface correcte qui se trouve dans l'interface actuelle
		//COULEURS
		//Rouge = exploration
		//Vert = bonne interface
		//bleu = presence d'un atome
		//ACCUMULATEUR
		//A chaque fois qu'un atome est dans l'interface, on ajoute 1 a l'accumulateur
		double projette(Molecule * mol1, Molecule * mol2, float * image, int width, bool bonPartenaire, double * recouvrement = NULL)
		{
			static Atome * AtomesTransformesMol1 = NULL;
			static Atome * AtomesTransformesMol2 = NULL;
			static int NbAtomesTransformesMol1 = 0;
			static int NbAtomesTransformesMol2 = 0;
			static Hasher3D * Hasher = NULL;

			if(AtomesTransformesMol1 == NULL || NbAtomesTransformesMol1 < mol1->_NbAtomes)
			{
				SAFEDELETE_TAB(AtomesTransformesMol1);
				AtomesTransformesMol1 = new Atome[mol1->_NbAtomes];
				NbAtomesTransformesMol1 = mol1->_NbAtomes;
			}
			if(AtomesTransformesMol2 == NULL || NbAtomesTransformesMol2 < mol2->_NbAtomes)
			{
				SAFEDELETE_TAB(AtomesTransformesMol2);
				AtomesTransformesMol2 = new Atome[mol2->_NbAtomes];
				NbAtomesTransformesMol2 = mol2->_NbAtomes;
			}

			if(Hasher == NULL)
				Hasher = new Hasher3D();


			//On transforme les molécules (passage des atomes en repère global)
			NYVert3Df pos;
			for(int i=0;i<mol1->_NbAtomes;i++)
			{
				pos = mol1->_Atomes[i]._Pos;
				if(!bonPartenaire)
				{
					pos -= mol1->_Barycenter;
					pos = mol1->_Transform._Rot * pos;
					pos += mol1->_Transform._Pos;
				}
				AtomesTransformesMol1[i]._Pos = pos;
				AtomesTransformesMol1[i]._Radius = mol1->_Atomes[i]._Radius;
				AtomesTransformesMol1[i]._Charge = mol1->_Atomes[i]._Charge;
				AtomesTransformesMol1[i]._UdockId = mol1->_Atomes[i]._UdockId;
			}


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
				if(!bonPartenaire)
				{
					pos -= mol2->_Barycenter;
					pos = mol2->_Transform._Rot * pos;
					pos += mol2->_Transform._Pos;
				}
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
			Hasher->createBuckets(NYVert3Df(minX,minY,minZ),sizeMax,30.0f); //avec un cutoff a 16 on trouve pas la meme chose si on inverse le couples...

			//On ajoute les atomes aux buckets
			for(int i=0;i<mol2->_NbAtomes;i++)
			{
				Hasher->putInBucket(AtomesTransformesMol2[i]._Pos,AtomesTransformesMol2 + i);
			}

			BucketRequest request;
			NYVert3Df pos1,pos2,posr;
			double r;
			double rmSurR;
			double rmSurR6;
			double rmSurR8;
			double epsilon;
			double contact;
			double charge;
			int udockId1 = 0;

			double energy = 0;
			double energyContact = 0;
			double energyCharge = 0;
			float radius1 = 0;
			double charge1 = 0;

			float maxTheta= 0;
			float minTheta = 1000;
			float maxPhi = 0;
			float minPhi = 1000;

			//On détermine les coordonnées polaires de l'atome
			float dist,theta,phi;

			int nbInInterface = 0;
			int nbInGoodInterface = 0;
			int sizeGoodInterface = 0;

			//Reset les flags si bon partenaire
			if(bonPartenaire)
				for(int i=0;i<mol1->_NbAtomes;i++)
						mol1->_Atomes[i]._Flags = 0;

			for(int i=0;i<mol1->_NbAtomes;i++)
			{
				radius1 = AtomesTransformesMol1[i]._Radius;
				pos1 = AtomesTransformesMol1[i]._Pos;
				charge1 = AtomesTransformesMol1[i]._Charge;
				udockId1 = AtomesTransformesMol1[i]._UdockId;

				//On se recentre en 0 (l'origine de la mol est son barycentre si j'ai bon, car c'est notre centre de rotation)
				posr = mol1->_Atomes[i]._Pos - mol1->_Barycenter;
				//On détermine les coordonnées polaires de l'atome
				posr.toSphericalCoordinates(&dist,&theta,&phi);
				if(theta < 0)
					theta += 2*M_PI;
				if(phi < 0)
					phi += 2*M_PI;

				/*if(theta > maxTheta)
					maxTheta = theta;
				if(phi > maxPhi)
					maxPhi = phi;
				if(theta < minTheta)
					minTheta = theta;
				if(phi < minPhi)
					minPhi = phi;

				Log::log(Log::ENGINE_INFO,(toString(posr.X)+" "+toString(posr.Y)+" "+toString(posr.Z)).c_str());*/

				Hasher->getBucketContentWithNeighbours(pos1,&request);

				bool inInterface = false;

				for(int k=0;k<27;k++)
				{
					Atome * atomek = (Atome*) (request._Buckets[k]);
					while(atomek)
					{
						pos2 = atomek->_Pos;
						pos2 -= pos1;
						r = pos2.getSize();

						//Si a moins de n angstroms, alors on dit que l'atome est sur l'interface
						if(r- (radius1 + atomek->_Radius) < 4.0f)
						{
							inInterface = true;
						}

						//On calcule l'energie
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
					}
				}

				int x = (int) ((theta * width)/(M_PI));
				int y = (int) ((phi * width)/(2*M_PI));
				if(inInterface)
				{
					//Si c'est l'interface de base
					if(bonPartenaire)
						image[(y*width*3) + (x*3)+1] ++ ; //Dans le vert celle de base	(pas ce quon affiche, voir goodColors() )
					else
						image[(y*width*3) + (x*3)+0] ++ ; //Dans le rouge l'exploration (pas ce quon affiche, voir goodColors() )
					nbInInterface++;

					//On marque que l'atome est explore
					mol1->_Atomes[i]._Accumulateur++;

					if(bonPartenaire)
						mol1->_Atomes[i]._Flags = 1;

					//Calcul du recouvrement
					if(!bonPartenaire && mol1->_Atomes[i]._Flags)
						nbInGoodInterface++;
				}
				//On marque l'atome en bleu de toute facon on met la surface
				image[(y*width*3) + (x*3)+2] = min(1.0f,image[(y*width*3) + (x*3)+2] + mol1->_Atomes[i]._Surface); //Dans le bleu c'est un atome (pas ce quon affiche, voir goodColors() )

				//Qu'il soit en interaction ou pas,on compte l'atome si il est un atome d'interface
				if(!bonPartenaire && mol1->_Atomes[i]._Flags)
					sizeGoodInterface++;
			}

			if(recouvrement)
				*recouvrement = (double)nbInGoodInterface/(double)sizeGoodInterface;

			return energy;

			//Log::log(Log::ENGINE_INFO,("Found "+toString(nbInInterface)+"/"+toString(mol1->_NbAtomes)+" atomes in interface").c_str());
		}

		//useBasePosition : les atomes sont chargés dans le buffer de rendu et dans le moteur physique
		//avec l'offset du barycentre. Donc is on utilise une potision donnée par l'appli, il faut
		//tenir compte de cet offset. Par contre, si on utilise un position de base (par exemple les
		//atomes dans leur position originale dans le fichier) alors il ne faut plus tenir compte
		//de l'offset du barycentre.
		void projette2(Molecule * mol1, Molecule * mol2, float * image, int width, bool useBasePosition)
		{
			static Atome * AtomesTransformesMol1 = NULL;
			static Atome * AtomesTransformesMol2 = NULL;
			static Hasher3D * Hasher = NULL;

			if(AtomesTransformesMol1 == NULL || sizeof(AtomesTransformesMol1) < mol1->_NbAtomes)
				AtomesTransformesMol1 = new Atome[mol1->_NbAtomes];
			if(AtomesTransformesMol2 == NULL || sizeof(AtomesTransformesMol2) < mol2->_NbAtomes)
				AtomesTransformesMol2 = new Atome[mol2->_NbAtomes];

			if(Hasher == NULL)
				Hasher = new Hasher3D();


			//On transforme les molécules (passage des atomes en repère global)
			NYVert3Df pos;
			for(int i=0;i<mol1->_NbAtomes;i++)
			{
				pos = mol1->_Atomes[i]._Pos;
				if(!useBasePosition)
				{
					pos -= mol1->_Barycenter;
					pos = mol1->_Transform._Rot * pos;
					pos += mol1->_Transform._Pos;
				}
				AtomesTransformesMol1[i]._Pos = pos;
				AtomesTransformesMol1[i]._Radius = mol1->_Atomes[i]._Radius;
				AtomesTransformesMol1[i]._Charge = mol1->_Atomes[i]._Charge;
				AtomesTransformesMol1[i]._UdockId = mol1->_Atomes[i]._UdockId;
			}


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
				if(!useBasePosition)
				{
					pos -= mol2->_Barycenter;
					pos = mol2->_Transform._Rot * pos;
					pos += mol2->_Transform._Pos;
				}
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
			Hasher->createBuckets(NYVert3Df(minX,minY,minZ),sizeMax,5.0f);

			//On ajoute les atomes aux buckets
			for(int i=0;i<mol2->_NbAtomes;i++)
			{
				Hasher->putInBucket(AtomesTransformesMol2[i]._Pos,AtomesTransformesMol2 + i);
			}

			BucketRequest request;
			NYVert3Df pos1,pos2,posr;
			double r;

			float maxTheta= 0;
			float minTheta = 1000;
			float maxPhi = 0;
			float minPhi = 1000;

			//On détermine les coordonnées polaires de l'atome
			float dist,theta,phi;

			int nbInInterface = 0;

			for(int i=0;i<mol1->_NbAtomes;i++)
			{
				pos1 = AtomesTransformesMol1[i]._Pos;
				//On se recentre en 0 (l'origine de la mol est son barycentre si j'ai bon, car c'est notre centre de rotation)
				posr = mol1->_Atomes[i]._Pos - mol1->_Barycenter;
				//On détermine les coordonnées polaires de l'atome
				posr.toSphericalCoordinates(&dist,&theta,&phi);
				if(theta < 0)
					theta += 2*M_PI;
				if(phi < 0)
					phi += 2*M_PI;

				/*if(theta > maxTheta)
					maxTheta = theta;
				if(phi > maxPhi)
					maxPhi = phi;
				if(theta < minTheta)
					minTheta = theta;
				if(phi < minPhi)
					minPhi = phi;

				Log::log(Log::ENGINE_INFO,(toString(posr.X)+" "+toString(posr.Y)+" "+toString(posr.Z)).c_str());*/

				Hasher->getBucketContentWithNeighbours(pos1,&request);

				bool inInterface = false;

				for(int k=0;k<27;k++)
				{
					Atome * atomek = (Atome*) (request._Buckets[k]);
					while(atomek)
					{
						pos2 = atomek->_Pos;
						pos2 -= pos1;
						r = pos2.getSize();

						//Si a moins de n angstroms, alors on dit que l'atome est sur l'interface
						if(r < 5.0f)
						{
							//On normalise par rapport a l'image
							int x = pos1.X + width/2;
							int y = pos1.Y + width/2;

							image[(y*width*3) + (x*3)] = 1.0f;//++

							x = atomek->_Pos.X + width/2;
							y = atomek->_Pos.Y + width/2;

							image[(y*width*3) + (x*3)+1] = 1.0f;//++

							inInterface = true;
						}
						//Suivant
						atomek = (Atome*) (atomek->_NextInBucket);
					}
				}

				if(inInterface)
					nbInInterface++;
			}

			Log::log(Log::ENGINE_INFO,("Found "+toString(nbInInterface)+"/"+toString(mol1->_NbAtomes)+" atomes in interface").c_str());
		}



};

#endif
