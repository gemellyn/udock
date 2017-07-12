#include "molecule_cubes.h"
#include "engine/timer.h"

void MoleculeCubes::releaseTempGeometry(void)
{
	Log::log(Log::ENGINE_INFO,"Releasing cubes memory");
	_MCubes->destroyCubes();
	Log::log(Log::ENGINE_INFO,"Releasing cubes temp geometry");
	_MCubes->destroyTempGeometry();
	Log::log(Log::ENGINE_INFO,"Release done");
}

void MoleculeCubes::makeGeometry(bool lowPoly, bool ses, float probe_boost, float * pcent)
{
	float minX,maxX,minY,maxY,minZ,maxZ;
	getDimensions(minX,maxX,minY,maxY,minZ,maxZ);

	//lowPoly = true;

	float taille_cube = 2.0f;
	if(!lowPoly)
		taille_cube = 0.6f;

	_MCubes->setOrigin(NYVert3Df(minX-5.0f - (probe_boost/2.0f),minY-5.0f- (probe_boost/2.0f),minZ-5.0f- (probe_boost/2.0f)));
	_MCubes->create(maxX-minX+10.0f + (probe_boost),maxY-minY + 10.0f + (probe_boost),maxZ-minZ + 10.0f + (probe_boost),taille_cube,true);


	if(!lowPoly)
	{
		Log::log(Log::USER_INFO,"Making molecule geometry for SES");

		if(probe_boost != 0)
			Log::log(Log::USER_INFO,("Using probe boost: " +
										toString(probe_boost)).c_str());

		//STEP 1
		//On crée la SAS sans interpolation, on marque juste les cubes
		#ifdef MSVC
		if(_CrtCheckMemory() == false)
			Log::log(Log::ENGINE_ERROR,"Memory corruption !");
        #endif

		_MCubes->setInterpolation(false); //Donc sans interpolation, pas besoin de recalculer les edges.
		Log::log(Log::USER_INFO,("Making SAS for " +
										toString(_NbAtomes) +
										" atomes").c_str());
		if(pcent)
			*pcent = 0;
		for(int i=0;i<_NbAtomes;i++)
		{
			if(pcent)
				*pcent += 0.33f/(float)_NbAtomes;
			_MCubes->valideSphere(_Atomes[i]._Pos,_Atomes[i]._Radius * 0.95 + (PROBE_SIZE + probe_boost),0);
		}
		Log::log(Log::ENGINE_INFO,"Cubes ok");

		#ifdef MSVC
		if(_CrtCheckMemory() == false)
			Log::log(Log::ENGINE_ERROR,"Memory corruption !");
        #endif

		//STEP 2
		//On reduit la SAS sur un diamètre PROBE_SIZE
		Log::log(Log::USER_INFO,"Contracting SAS to SES");
		NYTimer tStart;
		_MCubes->contractSurface(PROBE_SIZE + probe_boost);
		float elapsed = tStart.getElapsedSeconds(false);
		Log::log(Log::ENGINE_INFO,("Contraction done in " +
									toString(elapsed) +
									" ms").c_str());
		//STEP 3
		//On calcule tout bien la partie van der walls de la SES
		_MCubes->setInterpolation(true);
		Log::log(Log::USER_INFO,("Making van des Walls part for " +
									toString(_NbAtomes) +
									" atomes").c_str());

		for(int i=0;i<_NbAtomes;i++)
		{
			if(pcent)
				*pcent += 0.33f/(float)_NbAtomes;
			_MCubes->valideSphere(_Atomes[i]._Pos,_Atomes[i]._Radius * 0.95,0);
		}
		Log::log(Log::USER_INFO,"Van der Walls done");

		//On précalcule nous meme les normales
		_MCubes->setLissageNormales(true);

		//STEP 4
		//On calcule la charge des cubes
		Log::log(Log::USER_INFO,"Calculating Electrostatics");
		this->calculateElectrostatics();
		Log::log(Log::USER_INFO,"Electrostatics done");
	}
	else
	{
		Log::log(Log::USER_INFO,"Making molecule geometry for SAS only");

		//STEP 1
		//On crée la van der valls sans interpolation
		_MCubes->setInterpolation(false);

		Log::log(Log::USER_INFO,("Making SAS for " +
										toString(_NbAtomes) +
										" atomes").c_str());
		pcent = 0;
		for(int i=0;i<_NbAtomes;i++)
		{
			if(pcent)
				*pcent += 1.0f/(float)_NbAtomes;
			_MCubes->valideSphere(_Atomes[i]._Pos,_Atomes[i]._Radius + (PROBE_SIZE + probe_boost),_Atomes[i]._Charge);
		}

		Log::log(Log::USER_INFO,"SAS ok");
		_MCubes->setLissageNormales(false);


		/*_MCubes->Cubes[10].ColorShift = 1.0f;
		_MCubes->Cubes[10].setSommet(1,true);
		_MCubes->Cubes[11].ColorShift = 0.0f;
        _MCubes->Cubes[11].setSommet(0,true);*/
	}
}

void MoleculeCubes::calculateElectrostatics(void)
{
    int stepZ = (_MCubes->NbZ / _NbThreads);

	boost::thread **  threads = new boost::thread *[_NbThreads];
	PARAMS_THREAD_ELECTROSTATICS * params = new PARAMS_THREAD_ELECTROSTATICS[_NbThreads];

    for(int i=0;i<_NbThreads;i++)
    {
        params[i]._MCubes = _MCubes;
        params[i]._Molecule = this;
        params[i]._XStart = 0;// + (radius/(2*TailleCube)) + 1;
        params[i]._YStart = 0;
        params[i]._ZStart = i*(_MCubes->NbZ / _NbThreads);
        params[i]._NbX = _MCubes->NbX;
        params[i]._NbY = _MCubes->NbY;
        params[i]._NbZ = stepZ;
        threads[i] = new boost::thread(threadElectrostatics,params + i);
    }

    //On attend la fin des threads
    for(int i=0;i<_NbThreads;i++)
       threads[i]->join();

    for(int i=0;i<_NbThreads;i++)
        SAFEDELETE(threads[i]);

	SAFEDELETE_TAB(threads);
	SAFEDELETE_TAB(params);
}

void MoleculeCubes::threadElectrostatics(PARAMS_THREAD_ELECTROSTATICS * params)
{
    NYVert3Df pos;
    NYVert3Df normal;
    float charge;
    uint8 code;

    //Log::log(Log::USER_INFO,("Thread electrostatics from " + toString(params->ZStart) + " to " + toString(params->ZStart+params->NbZ)).c_str());

    MarchingCubes * mcubes = params->_MCubes;
    Molecule * mol = params->_Molecule;
    for (int x=params->_XStart;x<params->_NbX;x++)
    {
        for (int y=params->_YStart;y<params->_NbY;y++)
        {
            for (int z=params->_ZStart;z<params->_ZStart+params->_NbZ;z++)
            {
                code = mcubes->getCubeCode(x,y,z);
                if(code !=0 && code != 255)
                {
                    mcubes->getCubeBarycentreCoords(pos,x,y,z);
                    mcubes->getCubeFaceNormal(normal,x,y,z);
                    charge = mol->getCharge(pos + (normal * PROBE_SIZE));
                    mcubes->setCubeColorShift(charge,x,y,z);
                }
            }
        }
    }

}


void MoleculeCubes::addToOpengl(void)
{
	Log::log(Log::ENGINE_INFO,"Outputing geometry to opengl");
	//_MCubes->makeGeometryFaces(NYVert3Df()-_Barycenter);
	_MCubes->makeGeometryFacesDataOriented(NYVert3Df() - _Barycenter);
}

/**
  * On fait le convex hull avec la géom
  */
void MoleculeCubes::makeCollisionShapeFromGeom(NYPhysicEngine * engine, bool precise)
{
	this->_PreciseCollision = precise;
	if(!precise)
	{
		Log::log(Log::ENGINE_INFO,"Building hull shape from geometry");
		this->_CollideHull = NULL;
		_MCubes->destroyTempGeometry();
		_MCubes->makeVerticesOnlyBuffer(NYVert3Df()-_Barycenter);
		float * vertices = _MCubes->_Vertices;
		int nbPoints = _MCubes->_NbVertices;
		int stride = 3 * sizeof(float);
		this->_CollideHull = engine->calcHullShape(vertices,nbPoints,stride);
		Log::log(Log::ENGINE_INFO,"Hull shape done");
	}
	else
	{
		Log::log(Log::ENGINE_INFO,"Building trimesh shape from geometry");
		this->_CollideMesh = NULL;
		_MCubes->destroyTempGeometry();
		_MCubes->makeVerticesOnlyBuffer(NYVert3Df()-_Barycenter);
		float * vertices = _MCubes->_Vertices;
		int nbPoints = _MCubes->_NbVertices;
		int stride = 3 * sizeof(float);

		//for(int i=0;i<nbPoints*3;i++)
			//_cprintf("%f ",vertices[i]);

		this->_CollideMesh = engine->calcTriMeshShape(vertices,nbPoints,stride);
		Log::log(Log::ENGINE_INFO,"Trimesh shape done");
	}
}

/**
  * On fait le convex hull avec la géom (trèst len en fait)
  */
void MoleculeCubes::makeCollisionShapeFromMols(NYPhysicEngine * engine)
{
	Log::log(Log::ENGINE_INFO,"Building compound shape from mols");
	this->_CollideCompound = engine->createCompoundShape();
	for(int i=0;i<this->_NbAtomes;i++)
		engine->addSphereShapeToCompound(this->_CollideCompound,this->_Atomes[i]._Pos-_Barycenter,this->_Atomes[i]._Radius+PROBE_SIZE);
	Log::log(Log::ENGINE_INFO,"Compound shape done");
}

/**
  * A redefinir : ajoute la molecule au moteur physique : utilise la shape et crée le body
  */
void  MoleculeCubes::addToPhysicEngine(NYPhysicEngine * engine)
{
	Log::log(Log::ENGINE_INFO,("Adding molecule " +
								toString(this) +
								" with body " +
								toString(this->_Body) +
								" to physic engine").c_str());

	this->_Body = new NYBodyMolecule();
	if(_CollideMesh != NULL)
		engine->addTriMeshObject(true, _CollideMesh, _Transform._Pos, this, OWNER_MOLECULE, this->_Body);
	if(_CollideHull != NULL)
		engine->addConvexHullObject(true, _CollideHull, _Transform._Pos, this, OWNER_MOLECULE, this->_Body);
	if(_CollideCompound != NULL)
		engine->addCompoundObject(true, _CollideCompound, _Transform._Pos, this, OWNER_MOLECULE, this->_Body);

	//On applique sa rotation aussi
	setToPhysic();

}

/**
  * A redefinir : ajoute la molecule au moteur physique : utilise la shape et crée le body
  */
void  MoleculeCubes::removeFromPhysicEngine(NYPhysicEngine * engine)
{
	if(!this->_Body)
		return;

	Log::log(Log::ENGINE_INFO,("Removing molecule " +
								toString(this) +
								" with body " +
								toString(this->_Body) +
								" to physic engine").c_str());
	engine->removeObject(this->_Body);
	SAFEDELETE(this->_Body);

}

/**
  * On fait le rendu
  */
void MoleculeCubes::render(void)
{
	if(!_Visible)
		return;

	static float mat[16];
	glPushMatrix();

	//Position de la molécule
	glTranslatef(_Transform._Pos.X,_Transform._Pos.Y,_Transform._Pos.Z);

	//Rotation de la molécule autour du barycentre
	_Transform._Rot.getTansposedTab(mat);
	glMultMatrixf(mat);

	/*_Body->getOpenGlMatrix(mat);
	glMultMatrixf(mat);*/

	//Le translate du barycentre est après la rotation, on veut que la rotation se fasse autour du barycentre
	//glTranslatef(-_Barycenter.X,-_Barycenter.Y,-_Barycenter.Z);

	_MCubesToRender->renderFaces();
	/*glBegin(GL_TRIANGLES);
	float * points = _MCubes->_Vertices;
	for(int i=0;i<_MCubes->_NbVertices;i++)
		glVertex3f(points[(3*i)+0],points[(3*i)+1],points[(3*i)+2]);
	glEnd();*/

	//rendu du bouclier
	if(_CollideHull != NULL)
	{
		/*glBegin(GL_LINE_LOOP);
		const btVector3 * points = _Hull->Hull->getPoints();
		for(int i=0;i<_Hull->Hull->getNumVertices();i++)
			glVertex3f(points[i].x(),points[i].y(),points[i].z());
		glEnd();*/

		/*glBegin(GL_TRIANGLES);
		const btVector3 * points = _Hull->Shape->getVertexPointer();
		const unsigned int * indices = _Hull->Shape->getIndexPointer();
		for(int i=0;i<_Hull->Shape->numIndices();i++)
			glVertex3f(points[indices[i]].x(),points[indices[i]].y(),points[indices[i]].z());
		glEnd();*/
	}

	glPopMatrix();
}

void MoleculeCubes::renderGrappins(void)
{
	if(!_Visible)
		return;

	static float mat[16];
	glPushMatrix();

	//Position de la molécule
	glTranslatef(_Transform._Pos.X,_Transform._Pos.Y,_Transform._Pos.Z);

	//Rotation de la molécule autour du barycentre
	_Transform._Rot.getTansposedTab(mat);
	glMultMatrixf(mat);

	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);

	//Rendu des docking points
	for(int i=0;i<NB_DOCKING_POINTS;i++)
	{

		if(_DockingPoints[i]._IsSet)
		{
			glPushMatrix();
			glTranslatef(_DockingPoints[i]._Pos.X,_DockingPoints[i]._Pos.Y,_DockingPoints[i]._Pos.Z);
			glEnable(GL_BLEND);
			glColor4f(0.3f,1.0f,0.3f,0.7f);
			NYRenderer::checkGlError("drawSolidSphere before");
			NYRenderer::getInstance()->drawSolidSphere(1.0f,20,20);
			NYRenderer::checkGlError("drawSolidSphere after");
			glPopMatrix();
		}

	}

	glPopMatrix();
}

void MoleculeCubes::setAlternativeRendering(void)
{
	/*if(_MCubesToRender == _MCubes)
		_MCubesToRender = _MCubesLowDef;
	else*/
		_MCubesToRender = _MCubes;
}

/**
  * Sauvegarde dans un fichier
  */
void MoleculeCubes::saveToBinFile(const char * file)
{
	_MCubes->saveToBinFile(file);
}

/**
  * Sauvegarde dans un fichier OBJ
  */
void MoleculeCubes::saveToObjFile(const char * file)
{
	_MCubes->destroyTempGeometry();
	_MCubes->makeVerticesOnlyBuffer(NYVert3Df()-_Barycenter);
	_MCubes->saveToObjFile(file);
}

/**
  * Calcul de la surface. Attention, détruit le geometrie
  * Si on normalize, donne la surface entre 0 et 1 pour chaque atome
  */
void MoleculeCubes::calcSurfaceByAtom(bool normalize = false)
{
	float minX,maxX,minY,maxY,minZ,maxZ;
	getDimensions(minX,maxX,minY,maxY,minZ,maxZ);

	float taille_cube = 0.4f;

	_MCubes->setOrigin(NYVert3Df(minX-5.0f ,minY-5.0f,minZ-5.0f));
	_MCubes->create(maxX-minX+10.0f,maxY-minY + 10.0f,maxZ-minZ + 10.0f,taille_cube,true);


	Log::log(Log::ENGINE_INFO,"Making molecule SAS for atom surface calculus");


	//On crée la SAS sans interpolation, on marque juste les cubes
	_MCubes->setInterpolation(false); //Donc sans interpolation, pas besoin de recalculer les edges.
	Log::log(Log::ENGINE_INFO,("Making SAS for " +
		toString(_NbAtomes) +
		" atomes").c_str());

	for(int i=0;i<_NbAtomes;i++)
	{
		_MCubes->valideSphere(_Atomes[i]._Pos,_Atomes[i]._Radius + PROBE_SIZE,0);
	}
	Log::log(Log::ENGINE_INFO,"Cubes ok");

	//On compte la surface
	int maxSurface = 0;
	for(int i=0;i<_NbAtomes;i++)
	{
		_Atomes[i]._Surface = _MCubes->getNbFacettesSphere(_Atomes[i]._Pos,_Atomes[i]._Radius + PROBE_SIZE);
		if(_Atomes[i]._Surface > maxSurface)
			maxSurface = _Atomes[i]._Surface;
	}

	//On normalize
	if(normalize)
	{
		for(int i=0;i<_NbAtomes;i++)
		{
			_Atomes[i]._Surface /= (float)maxSurface;
		}
	}



	Log::log(Log::ENGINE_INFO,"Calcul de surface ok");
}

std::vector<size_t> MoleculeCubes::getAssociatedVertices(const Atome& atom)
{
	float* ptrDataVert = (_MCubes->_MolVert); //ptr to vertices positions array
	const auto offsetEnergy = _MCubes->SIZE_ENERGY; // number of float for stocking energy
	const auto offsetVertices = _MCubes->SIZE_VERTICES; // number of float for a vertex
	const unsigned int nbVertices = _MCubes->VBO_VerticesCount; // number of vertices in VBO

	std::vector<size_t> vertices; //vertices to be returned

	const float radius = atom._Radius * 1.25f;
	const float radiusSqr = radius * radius;
	NYVert3Df atom_pos_t, pos_t, vecToVec; //temp var for pos of vertex and magnitude
	atom_pos_t = atom._Pos;
	float distance_t;
	for (size_t i = 0; i < nbVertices; i++)
	{
		//retrieve pos
		pos_t.X = *(ptrDataVert);
		pos_t.Y = *(ptrDataVert + 1);
		pos_t.Z = *(ptrDataVert + 2);
		//get vec to vec vector
		vecToVec = atom_pos_t - pos_t;
		distance_t = vecToVec.getMagnitude(); // actually sqrmagnitude ??
		//do stuff
		if (distance_t <= radiusSqr)
		{
			vertices.push_back( i*offsetEnergy); // offset to energy data added to the vector
		}
		//go to nextdata
		ptrDataVert += offsetVertices;
	}

	return vertices;
}
