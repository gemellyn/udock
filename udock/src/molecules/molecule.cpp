#include "molecule.h"
#include "global.h"

unsigned int Molecule::genUID = 0;

/**
  * Chargement de la molecule a partir d'un MOL2
  */
int Molecule::loadFromMOL2(const wchar_t * path)
{
	//On savue le path
	this->_Path = path;

	//Ouverture et check du fichier
	std::string spath = wstring_to_utf8(path);
	FILE * fe = fopen(spath.c_str(),"r");
	if(!fe)
	{
		Log::log(Log::ENGINE_ERROR,("Unable to open mol2 file "+wstring_to_utf8(toWString(path))).c_str());
		return -1;
	}

	//On parse l'entete
	if(Mol2Loader::parseEntete(fe,_MolName,_MolType,_ChargeType,&_NbBonds) != 0)
	{
		Log::log(Log::ENGINE_ERROR,("Error while parsing entete mol2 file "+wstring_to_utf8(toWString(path))).c_str());
		return -1;
	}

	//On fait une permière passe pour calculer le nombre d'atomes
	_NbAtomes = 0;
	Mol2Loader::findAtomsFromStart(fe);

	int ret = 0;
	_NbAtomes = 0;
	ret = Mol2Loader::readLine(fe);
	while(ret == 0)
	{
		_NbAtomes++;
		ret = Mol2Loader::readLine(fe);
	};

	//On revient au début du fichier
	fseek(fe,0,SEEK_SET);

	//Au debut des atomes
	Mol2Loader::findAtomsFromStart(fe);

	//On crée un tableau d'atomes à la bonne taille
	SAFEDELETE_TAB(_Atomes);
	_Atomes = new Atome[_NbAtomes];

	//On charge les atomes du fichier
	ret =  Mol2Loader::readLine(fe);
	int iAtom = 0;
	Mol2Atom * atom = new Mol2Atom();
	while(ret == 0)
	{
		//On ajoute un atome
		atom->buildFromLine(Mol2Loader::g_current_mol2_line);
		_Atomes[iAtom].buildFromMol2Atom(atom);
		ret = Mol2Loader::readLine(fe);
		iAtom++;
	}

	//Rend memoire
	SAFEDELETE(atom);

	//Fermeture du fichier
	fclose (fe);

	//Reset position
	this->_Transform = Positionator();

	//Calcul du barycentre
	calcBarycenter();

	//Calcul du radius
	computeRadius();

	//Lissage des charges
	//lisseCharges();

	return 0;
}

/**
  * On sauve dans un mol2, en tenant compte de la position actuelle
  */
int Molecule::saveToMOL2(const char * path)
{
	FILE * fs = fopen(path,"w");
	if(!fs)
	{
		Log::log(Log::ENGINE_ERROR,("Unable to open mol2 file for writing "+toString(path)).c_str());
		return -1;
	}

	//On va ecrire des atomes
	Mol2Loader::writeEntete(fs,_MolName.c_str(),_NbAtomes,_NbBonds,_MolType.c_str(),_ChargeType.c_str());

	//On les ecrit
	Mol2Atom * atom = new Mol2Atom();
	for(int i=0;i<_NbAtomes;i++)
	{
		atom->setSerial(i+1);
		atom->setName(_Atomes[i]._Name);
		NYVert3Df position;
		toWorldPos(_Atomes[i]._Pos,&position);
		atom->setPosition(position);
		atom->setType(_Atomes[i]._Type);
		atom->setSubstName(_Atomes[i]._SubstName);
		atom->setSubstId(_Atomes[i]._SubstId);
		atom->setCharge(_Atomes[i]._Charge);
		atom->saveToFile(fs);
	}

	//On output les bonds
	std::string spath = wstring_to_utf8(this->_Path);
	FILE * fe = fopen(spath.c_str(),"r");
	if(fe == NULL)
	{
		Log::log(Log::ENGINE_ERROR,("Unable to open mol2 file for bonds copy "+wstring_to_utf8(_Path)).c_str());
		return -1;
	}
	else
	{
		Mol2Loader::writeBonds(fs,fe);
		fclose (fe);
	}

	//Rend memoire
	SAFEDELETE(atom);

	//Fermeture du fichier
	fclose (fs);

	return 0;
}

/**
  * On sauve dans un mpdb, en tenant compte de la position actuelle
  */
int Molecule::saveToPDB(const char * path, bool useOriginalPos)
{
	FILE * fs = NULL;
	fs = fopen(path,"w");
	if(!fs)
	{
		Log::log(Log::ENGINE_ERROR,("Unable to open pdb file for writing "+toString(path)).c_str());
		return -1;
	}

	PdbLoader::writeEntete(fs,_MolName.c_str(),_NbAtomes,_NbBonds,_MolType.c_str(),_ChargeType.c_str());

	//On les ecrit
	PdbAtom * atom = new PdbAtom();
	for(int i=0;i<_NbAtomes;i++)
	{
		atom->setSerial(i+1);
		atom->setName(_Atomes[i]._Name);
		NYVert3Df position;
		if(!useOriginalPos)
			toWorldPos(_Atomes[i]._Pos,&position);
		else
			position = _Atomes[i]._Pos;
		atom->setPosition(position);
		atom->setType(_Atomes[i]._Type);
		atom->setSubstName(_Atomes[i]._SubstName);
		atom->setSubstId(_Atomes[i]._SubstId);
		atom->setCharge(_Atomes[i]._Charge);
   		atom->saveToFile(fs,0);
	}

	//Rend memoire
	SAFEDELETE(atom);

	//Fermeture du fichier
	fclose (fs);


	return 0;
}

int Molecule::saveComplexToPDB(const char * path, Molecule * otherMol, float score, bool useOriginalPos)
{
	FILE * fs = NULL;
	fs = fopen(path,"w");
	if(!fs)
	{
		Log::log(Log::ENGINE_ERROR,("Unable to open pdb file for writing "+toString(path)).c_str());
		return -1;
	}

	PdbLoader::writeEntete(fs,_MolName.c_str(),_NbAtomes,_NbBonds,_MolType.c_str(),_ChargeType.c_str());

	PdbLoader::writeRemark(fs, 300, "");
	PdbLoader::writeRemark(fs, 300, ("Mol A : " + this->_MolName).c_str());
	PdbLoader::writeRemark(fs, 300, ("Mol B : " + otherMol->_MolName).c_str());
	PdbLoader::writeRemark(fs, 300, ("UDock Score : " + toString(-score)).c_str());
	PdbLoader::writeRemark(fs, 300, "");

	//On ecrit la mol 1
	PdbAtom * atom = new PdbAtom();
	for(int i=0;i<_NbAtomes;i++)
	{
		atom->setSerial(i+1);
		atom->setName(_Atomes[i]._Name);
		NYVert3Df position;
		if(!useOriginalPos)
			toWorldPos(_Atomes[i]._Pos,&position);
		else
			position = _Atomes[i]._Pos;
		atom->setPosition(position);
		atom->setType(_Atomes[i]._Type);
		atom->setSubstName(_Atomes[i]._SubstName);
		atom->setSubstId(_Atomes[i]._SubstId);
		atom->setCharge(_Atomes[i]._Charge);
		atom->saveToFile(fs,0);
	}

	PdbLoader::writeJunction(fs,*atom,0);

	//On ecrit la mol 2
	for(int i=0;i<otherMol->_NbAtomes;i++)
	{
		atom->setSerial(i+1+_NbAtomes+1);
		atom->setName(otherMol->_Atomes[i]._Name);
		NYVert3Df position;
		if(!useOriginalPos)
			otherMol->toWorldPos(otherMol->_Atomes[i]._Pos,&position);
		else
			position = otherMol->_Atomes[i]._Pos;
		atom->setPosition(position);
		atom->setType(otherMol->_Atomes[i]._Type);
		atom->setSubstName(otherMol->_Atomes[i]._SubstName);
		atom->setSubstId(otherMol->_Atomes[i]._SubstId);
		atom->setCharge(otherMol->_Atomes[i]._Charge);
		atom->saveToFile(fs,1);
	}

	//Rend memoire
	SAFEDELETE(atom);

	//Fermeture du fichier
	fclose (fs);


	return 0;
}

/**
  * On récupère le plus gros atome. Permet par exemple de hasher l'espace de la molécule à la meilleure taille
  */
float Molecule::getBiggestAtomRadius(void)
{
	float maxRadius = _Atomes[0]._Radius;
	for(int i=1;i<_NbAtomes;i++)
	{
		if(_Atomes[i]._Radius > maxRadius)
			maxRadius = _Atomes[i]._Radius;
	}
	return maxRadius;
};

void Molecule::getDimensions(float & minXres, float & maxXres, float & minYres, float & maxYres, float &minZres, float &maxZres)
{
	float minX = _Atomes[0]._Pos.X;
	float minY = _Atomes[0]._Pos.Y;
	float minZ = _Atomes[0]._Pos.Z;
	float maxX = _Atomes[0]._Pos.X;
	float maxY = _Atomes[0]._Pos.Y;
	float maxZ = _Atomes[0]._Pos.Z;

	for(int i=0;i<_NbAtomes;i++)
	{
		if(_Atomes[i]._Pos.X < minX)
			minX = _Atomes[i]._Pos.X;
		if(_Atomes[i]._Pos.Y < minY)
			minY = _Atomes[i]._Pos.Y;
		if(_Atomes[i]._Pos.Z < minZ)
			minZ = _Atomes[i]._Pos.Z;

		if(_Atomes[i]._Pos.X > maxX)
			maxX = _Atomes[i]._Pos.X;
		if(_Atomes[i]._Pos.Y > maxY)
			maxY = _Atomes[i]._Pos.Y;
		if(_Atomes[i]._Pos.Z > maxZ)
			maxZ = _Atomes[i]._Pos.Z;
	}

	minXres = minX;
	minYres = minY;
	minZres = minZ;
	maxXres = maxX;
	maxYres = maxY;
	maxZres = maxZ;
}

void Molecule::getMaxDim(float & maxDim)
{
	if(!_MaxDimDone)
	{
		float minX = 0;
		float minY = 0;
		float minZ = 0;
		float maxX = 0;
		float maxY = 0;
		float maxZ = 0;

		getDimensions(minX,minY,minZ,maxX,maxY,maxZ);

		_MaxDim = max(max(maxX-minX,maxY-minY),maxZ-minZ);
		_MaxDimDone = true;
	}

	maxDim = _MaxDim;
}


void  Molecule::calcBarycenter(void)
{
	_Barycenter = NYVert3Df();
	for(int i=0;i<_NbAtomes;i++)
	{
		_Barycenter += _Atomes[i]._Pos;
	}
	_Barycenter /= (float)_NbAtomes;
}

void Molecule::savePosition(const char * path)
{
	FILE * fs = fopen(path,"w");
	if(fs)
	{
		for(int y=0;y<3;y++)
			for(int x=0;x<3;x++)
				fprintf(fs,"%f ",this->_Transform._Rot.Mat.m[x][y]);
		fprintf(fs,"%f %f %f",this->_Transform._Pos.X,this->_Transform._Pos.Y,this->_Transform._Pos.Z);
		fclose(fs);
	}

}

void Molecule::loadPosition(const char * path)
{
    bool error = false;
	FILE * fe = fopen(path,"r");
	if(fe)
	{
		for(int y=0;y<3;y++)
			for(int x=0;x<3;x++)
                if(fscanf(fe,"%f ",&(this->_Transform._Rot.Mat.m[x][y])) != 1)
                    error = true;
		if(fscanf(fe,"%f %f %f",&(this->_Transform._Pos.X),&(this->_Transform._Pos.Y),&(this->_Transform._Pos.Z)) != 1)
            error = true;
		fclose(fe);

		if(error)
            Log::log(Log::USER_ERROR,"Unable to load molecule position");
	}

}


