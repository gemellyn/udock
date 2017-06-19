/**
  * Ce fichier regroupe des classes d'abstraction de la physique
  * C'est un adapteur qui permet de limiter au maximum les dépendances
  * entre l'api du moteur physique utilisée (PhysX ou Bulet par ex)
  * et le moteur lui même.
  **/
#ifndef __PHYSIC_ENGINE__
#define __PHYSIC_ENGINE__

//#define NY_BULLET_MULTI_THREAD 1
#ifdef NY_BULLET_MULTI_THREAD
#include "BulletMultiThreaded/SpuGatheringCollisionDispatcher.h"
#include "BulletMultiThreaded/Win32ThreadSupport.h"
#include "BulletMultiThreaded/SpuNarrowPhaseCollisionTask/SpuGatheringCollisionTask.h"
#endif

#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionShapes/btShapeHull.h"
#include "BulletCollision/Gimpact/btGImpactShape.h"
#include "BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h"
#include "LinearMath/btConvexHull.h"

#include "engine/utils/types_3d.h"
#include "engine/utils/ny_utils.h"
#include "engine/log/log.h"


/////POUR CHOPER LES SUBSHAPES d'une COMPOUND SHAPE
struct	AllCompoundRayResultCallback : public btCollisionWorld::AllHitsRayResultCallback
{
	AllCompoundRayResultCallback(const btVector3&	rayFromWorld,const btVector3&	rayToWorld)
	: AllHitsRayResultCallback(rayFromWorld,rayToWorld)
	{
	}

	btAlignedObjectArray<btCollisionShape*> m_childShapes;

	virtual	btScalar	addSingleResult(btCollisionWorld::LocalRayResult& rayResult,bool normalInWorldSpace)
	{

		btScalar res = AllHitsRayResultCallback::addSingleResult(rayResult,normalInWorldSpace);

		//Pendant cet appel, la collision shape est la child shape (si c'est une compound shape)
		//Après elle est remplacée par la coll shape mère et donc on perd le child
		//Donc ici on la stoque pour la retrouver
		m_childShapes.push_back(rayResult.m_collisionObject->getCollisionShape());

		return res;
	}
};



enum
{
	OWNER_MOLECULE,
	OWNER_SHIP,
	OWNER_PROJECTILE
};

///Pour gérer les collisisons avec un bouclier convexe
class NYHullShape
{
	public:
		btConvexHullShape * Hull;
		btShapeHull * Shape; //Pour le debug

	public:
		NYHullShape()
		{
			Hull = NULL;
			Shape = NULL;
		}
};

///Pour gérer les collisions directement sur la géométrie, même concave
class NYTriShape
{
	public:
		btGImpactMeshShape * MeshShape;

	public:
		NYTriShape()
		{
			MeshShape = NULL;
		}

};

///Pour gérer les collisions avec une composition de primitives
class NYCompoundShape
{
	public:
		btCompoundShape * CompoundShape;

	public:
		NYCompoundShape()
		{
			CompoundShape = NULL;
		}

};

///Pour gérer la physique d'un objet, on utilise un corps
class NYBody
{
	public:
		btRigidBody * Body;
		void * BodyOwner; ///<Celui qui possède ce corps
		int BodyOwnerType;
		bool Frozen;
		float Mass;
		btCollisionShape * CollShape;
		bool Dynamic;

		NYBody() : Body(NULL), BodyOwner(NULL), Frozen(false), Mass(0.0f), BodyOwnerType(0), Dynamic(false), CollShape(NULL)
		{

		}

		//Active la detection continue de collisions
		//pour les objegts très rapides (qui vont plus vite en un step que leur diametre)
		void activateCCD(void)
		{
			float width = getSize();
			Body->setCcdMotionThreshold(width/2.0f);
			Body->setCcdSweptSphereRadius(width/4.0f);
		}

		void setFriction (float friction)
		{
			Body->setFriction(friction);
		}


		///Recupere la position du body
		NYVert3Df getBodyPosition(void)
		{
			//btScalar m[16];
			//On récup la matrice du modèle phy
			btDefaultMotionState* myMotionState = (btDefaultMotionState*)Body->getMotionState();
			btTransform trans;
			myMotionState->getWorldTransform(trans);
			btVector3 pos = trans.getOrigin();
			//myMotionState->m_graphicsWorldTrans.getOpenGLMatrix(m);
			//return NYVert3Df(m[12],m[13],m[14]);
			return NYVert3Df(pos.x(),pos.y(), pos.z());
		}

		void setBodyPosition(NYVert3Df position)
		{
			Body->setActivationState(ACTIVE_TAG);
			btDefaultMotionState* myMotionState = (btDefaultMotionState*)Body->getMotionState();
			btTransform trans;
			myMotionState->getWorldTransform(trans);
			trans.setOrigin(btVector3(position.X,position.Y,position.Z));
			myMotionState->setWorldTransform(trans);
			Body->setWorldTransform(trans);
			Body->setMotionState(myMotionState);
		}

		///La rotation sous forme d'un quaternion
		NYVert4Df getBodyRotation(void)
		{
			btDefaultMotionState* myMotionState = (btDefaultMotionState*)Body->getMotionState();
			btTransform trans;
			myMotionState->getWorldTransform(trans);
			btQuaternion q = trans.getRotation();
			return NYVert4Df(q.getAxis().x(),q.getAxis().y(),q.getAxis().z(),q.getAngle());
		}

		void setBodyRotation(NYVert4Df rotation)
		{
			Body->setActivationState(ACTIVE_TAG);
			btDefaultMotionState* myMotionState = (btDefaultMotionState*)Body->getMotionState();
			btTransform trans;
			myMotionState->getWorldTransform(trans);
			trans.setRotation(btQuaternion(rotation.X,rotation.Y,rotation.Z,rotation.T));
			myMotionState->setWorldTransform(trans);
			Body->setWorldTransform(trans);
			Body->setMotionState(myMotionState);
		}

		///Position et orientation sous forme d'une matrice
		void getOpenGlMatrix(float * matrix)
		{
			btDefaultMotionState* myMotionState = (btDefaultMotionState*)Body->getMotionState();
			btTransform trans;
			myMotionState->getWorldTransform(trans);
			trans.getOpenGLMatrix(matrix);
			//myMotionState->m_graphicsWorldTrans.getOpenGLMatrix(matrix);
		}

		///La hauteur de la bbox du body
		float getHeight(void)
		{
			btVector3 top, bottom;
			Body->getAabb(top,bottom);
			return nyabs(bottom.z() - top.z());
		}

		float getWidth(void)
		{
			btVector3 top, bottom;
			Body->getAabb(top,bottom);
			return nyabs(bottom.x() - top.x());
		}

		//Donne la dimension max de l'objet
		float getSize(void)
		{
			btVector3 top, bottom;
			Body->getAabb(top,bottom);
			return max(nyabs(bottom.z() - top.z()),max(nyabs(bottom.y() - top.y()),nyabs(bottom.x() - top.x())));
		}


		///Applique une force sur le centre de gravite
		void applyForce(const NYVert3Df & force)
		{
			Body->setActivationState(ACTIVE_TAG);
			Body->applyCentralForce(btVector3(force.X,force.Y,force.Z));
		}

		///Applique une force en un point donné
		///Attention, la position relative est la position world - la position du body
		void applyForce(const NYVert3Df & force, const NYVert3Df & relPos)
		{
			Body->setActivationState(ACTIVE_TAG);
			Body->applyForce(btVector3(force.X,force.Y,force.Z),btVector3(relPos.X,relPos.Y,relPos.Z));
		}


		///Applique une force sur le centre de gravite
		void applyImpulse(const NYVert3Df & force)
		{
			Body->setActivationState(ACTIVE_TAG);
			Body->applyCentralImpulse(btVector3(force.X,force.Y,force.Z));
		}

		///Applique une force en un point donné
		void applyImpulse(NYVert3Df & force,NYVert3Df & pos)
		{
			Body->setActivationState(ACTIVE_TAG);
			Body->applyImpulse(btVector3(force.X,force.Y,force.Z),btVector3(pos.X,pos.Y,pos.Z));
		}

		void applyTorque(NYVert3Df force)
		{
			Body->setActivationState(ACTIVE_TAG);
			Body->applyTorque(btVector3(force.X,force.Y,force.Z));
		}

		void applyTorqueImpulse(NYVert3Df force)
		{
			Body->setActivationState(ACTIVE_TAG);
			Body->applyTorqueImpulse(btVector3(force.X,force.Y,force.Z));
		}

		///Donne le vecteur vitesse du body
		NYVert3Df getSpeed(void)
		{
			btVector3 actSpeed = Body->getLinearVelocity();
			return NYVert3Df(actSpeed.m_floats[0],actSpeed.m_floats[1],actSpeed.m_floats[2]);
		}

		void setSpeed(NYVert3Df speed)
		{
			Body->setLinearVelocity(btVector3(speed.X,speed.Y,speed.Z));
		}

		NYVert3Df getAngularSpeed(void)
		{
			btVector3 angSpeed = Body->getAngularVelocity();
			return NYVert3Df(angSpeed.m_floats[0],angSpeed.m_floats[1],angSpeed.m_floats[2]);
		}

		void setAngularSpeed(NYVert3Df speed)
		{
			Body->setAngularVelocity(btVector3(speed.X,speed.Y,speed.Z));
		}

		///Fait qu'un objet ne bouge plus
		///Attention !!!  ecrase les linear et angular factors
		void setFreeze(bool freeze)
		{
			Frozen = freeze;
			if(freeze)
			{
				Body->setLinearFactor(btVector3(0.0f,0.0f,0.0f));
				Body->setAngularFactor(btVector3(0.0f,0.0f,0.0f));
			}
			else
			{
				Body->setLinearFactor(btVector3(1.0f,1.0f,1.0f));
				Body->setAngularFactor(btVector3(1.0f,1.0f,1.0f));
			}
		}

		///Fait qu'un objet est limité sur certains axes en rotation et translation
		///1.0 ne limte pas du tout
		///0.0 limite totalement
		void setTransRotFactors(NYVert3Df translation, NYVert3Df rotation)
		{
			Body->setLinearFactor(btVector3(translation.X,translation.Y,translation.Z));
			Body->setAngularFactor(btVector3(rotation.X,rotation.Y,rotation.Z));
		}

		/**
			Appelee en cas de collision (si on a activé le collision report)
			A REDEFINIR
		*/
		virtual void collision(NYBody * other, btManifoldPoint& cp)
		{

		}

		/**
		* Malheureusement cette fonction n'a aucune impact en ce qui concerne les concave shapes, surement
		* a cause du fix nécessaire pour avoir le report de collision sur les mesh concaves
		*/
		void activateCollisionReport(bool report)
		{
			if (report)
				Body->setCollisionFlags(Body->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
			else
				Body->setCollisionFlags(Body->getCollisionFlags() & ~btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
		}

		void setMass(float mass)
		{
			btVector3 localInertia(0, 0, 0);
			if (this->Dynamic)
				CollShape->calculateLocalInertia(mass, localInertia);

			Body->setMassProps(mass, localInertia);
		}

};


class NYPhysicEngine
{
	public:
		btDefaultCollisionConfiguration * CollisionConfiguration;
		btCollisionDispatcher * Dispatcher;
		btDbvtBroadphase * Broadphase;
		btSequentialImpulseConstraintSolver * Solver;
		btDiscreteDynamicsWorld * DynamicsWorld;
		btAlignedObjectArray<btCollisionShape*>	CollisionShapes;

	private:
		AllCompoundRayResultCallback * _HitCallBack;
		int _LastGivenHitId;
		static NYPhysicEngine * _Instance;

		NYPhysicEngine()
		{
			CollisionConfiguration = NULL;
			Dispatcher = NULL;
			Broadphase = NULL;
			Solver = NULL;
			DynamicsWorld = NULL;
			_HitCallBack = NULL;
			setCollisionReportFunction(NYPhysicEngine::collisionReport);
		}

	public:

		static NYPhysicEngine * getInstance()
		{
			if(!_Instance)
				_Instance = new NYPhysicEngine();
			return _Instance;
		}

		///Initialisation du moteur physique
		void initialisation(NYVert3Df gravity)
		{
			CollisionConfiguration = new btDefaultCollisionConfiguration();

			#ifdef NY_BULLET_MULTI_THREAD
				int maxNumOutstandingTasks = 4;//number of maximum outstanding tasks
				Win32ThreadSupport * threadSupport = new Win32ThreadSupport(Win32ThreadSupport::Win32ThreadConstructionInfo(
					"collision",
					processCollisionTask,
					createCollisionLocalStoreMemory,
					maxNumOutstandingTasks));

				Dispatcher = new SpuGatheringCollisionDispatcher(threadSupport,maxNumOutstandingTasks,CollisionConfiguration);
			#else
				Dispatcher = new btCollisionDispatcher(CollisionConfiguration);
			#endif

			Broadphase = new btDbvtBroadphase();

			btSequentialImpulseConstraintSolver* sol = new btSequentialImpulseConstraintSolver();
			Solver = sol;

			DynamicsWorld = new btDiscreteDynamicsWorld(Dispatcher,Broadphase,Solver,CollisionConfiguration);

			btCollisionDispatcher * dispatcher = static_cast<btCollisionDispatcher *>(DynamicsWorld->getDispatcher());
			btGImpactCollisionAlgorithm::registerAlgorithm(dispatcher);

			DynamicsWorld->setGravity(btVector3(gravity.X,gravity.Y,gravity.Z));

			#ifdef NY_BULLET_MULTI_THREAD
				DynamicsWorld->getDispatchInfo().m_enableSPU = true;
			#endif

			_HitCallBack = NULL;
			_LastGivenHitId = 0;
		}

		///Jette un rayon et recuper la forme en collision
		NYBody * rayPickup(NYVert3Df & rayFrom, NYVert3Df  & rayTo, NYVert3Df * hitPoint)
		{
			btVector3 bRayFrom(rayFrom.X,rayFrom.Y,rayFrom.Z);
			btVector3 bRayTo(rayTo.X,rayTo.Y,rayTo.Z);

			btCollisionWorld::ClosestRayResultCallback rayCallback(bRayFrom,bRayTo);
			DynamicsWorld->rayTest(bRayFrom,bRayTo,rayCallback);
			if (rayCallback.hasHit())
			{
				btRigidBody* body = (btRigidBody*)btRigidBody::upcast(rayCallback.m_collisionObject);
				if (body)
				{
					//if (!(body->isStaticObject() || body->isKinematicObject()))
					{
						if(hitPoint)
							*hitPoint = NYVert3Df(rayCallback.m_hitPointWorld.x(),rayCallback.m_hitPointWorld.y(),rayCallback.m_hitPointWorld.z());
						NYBody * ourBody = (NYBody*)body->getUserPointer();
						_LastGivenHitId = 0;
						return ourBody;
					}
				}
			}

			return NULL;
		}

		NYBody * multipleRayPickup(NYVert3Df & rayFrom, NYVert3Df  & rayTo, NYVert3Df * hitPoint, float * distance, void ** shapeOwner)
		{
			btVector3 bRayFrom(rayFrom.X,rayFrom.Y,rayFrom.Z);
			btVector3 bRayTo(rayTo.X,rayTo.Y,rayTo.Z);

			//btCollisionWorld::ClosestRayResultCallback rayCallback(bRayFrom,bRayTo);
			_HitCallBack = new AllCompoundRayResultCallback(bRayFrom,bRayTo);
			_LastGivenHitId = 0;

			DynamicsWorld->rayTest(bRayFrom,bRayTo,*_HitCallBack);

			if (_HitCallBack->hasHit())
			{
				btRigidBody* body = (btRigidBody*)btRigidBody::upcast(_HitCallBack->m_collisionObjects[0]);
				if (body)
				{
					//if (!(body->isStaticObject() || body->isKinematicObject()))
					{
						if(hitPoint)
							*hitPoint = NYVert3Df(_HitCallBack->m_hitPointWorld[0].x(),_HitCallBack->m_hitPointWorld[0].y(),_HitCallBack->m_hitPointWorld[0].z());
						NYBody * ourBody = (NYBody*)body->getUserPointer();
						_LastGivenHitId = 0;
						*distance = _HitCallBack->m_hitFractions[0];
						if(shapeOwner)
							*shapeOwner = _HitCallBack->m_childShapes[0]->getUserPointer();
						return ourBody;
					}
				}
			}

			return NULL;
		}


		NYBody * getNextHit(NYVert3Df * hitPoint, float * distance, void ** shapeOwner)
		{
			if(_HitCallBack == NULL)
				return NULL;

			_LastGivenHitId++;
			if (_HitCallBack->hasHit() && _HitCallBack->m_collisionObjects.size() > _LastGivenHitId)
			{
				btRigidBody* body = (btRigidBody*)btRigidBody::upcast(_HitCallBack->m_collisionObjects[_LastGivenHitId]);
				if (body)
				{
					if (!(body->isStaticObject() || body->isKinematicObject()))
					{
						*hitPoint = NYVert3Df(_HitCallBack->m_hitPointWorld[_LastGivenHitId].x(),_HitCallBack->m_hitPointWorld[_LastGivenHitId].y(),_HitCallBack->m_hitPointWorld[_LastGivenHitId].z());
						NYBody * ourBody = (NYBody*)body->getUserPointer();
						*distance = _HitCallBack->m_hitFractions[_LastGivenHitId];
						if(shapeOwner)
							*shapeOwner = _HitCallBack->m_childShapes[_LastGivenHitId]->getUserPointer();
						return ourBody;
					}
				}
			}

			return NULL;
		}


		NYBody * addSphereObject(bool dynamic, float radius, const NYVert3Df & position, void * owner, int ownerType, NYBody * createdBody = NULL)
		{
			//On regarde si la shape n'existe pas deja
			btCollisionShape* colShape = getSphereColShape(radius);

			//Si elle n'existe pas on la cree
			if(colShape == NULL)
			{
				colShape = new btSphereShape(btScalar(radius));
				CollisionShapes.push_back(colShape);
				//colShape->setMargin(0.1f);
			}

			return addObject(dynamic,colShape,position,owner, ownerType,createdBody);
		}

		NYBody * addPlaneObject(bool dynamic, const NYVert3Df & normal, const NYVert3Df & position, void * owner, int ownerType, NYBody * createdBody = NULL)
		{
			//On regarde si la shape n'existe pas deja
			btCollisionShape* colShape = getPlaneColShape(normal);

			//Si elle n'existe pas on la cree
			if(colShape == NULL)
			{
				colShape = new btStaticPlaneShape(btVector3(normal.X,normal.Y,normal.Z),0);
				CollisionShapes.push_back(colShape);
			}

			return addObject(dynamic, colShape, position, owner, ownerType, createdBody);
		}

		NYBody * addBoxObject(bool dynamic, const NYVert3Df & size, const NYVert3Df & position, void * owner, int ownerType, NYBody * createdBody = NULL)
		{
			//On regarde si la shape n'existe pas deja
			btCollisionShape* colShape = getBoxColShape(size);

			//Si elle n'existe pas on la cree
			if(colShape == NULL)
			{
				colShape = new btBoxShape(btVector3(size.X,size.Y,size.Z));
				CollisionShapes.push_back(colShape);
			}

			return addObject(dynamic, colShape, position, owner, ownerType, createdBody);
		}


		//stride = taille en octets d'un point
		NYHullShape * calcHullShape(float * vertices, int nbVertices,int stride)
		{
			NYHullShape * shape = new NYHullShape();

			btTriangleMesh * trimesh = new btTriangleMesh();
			for (int i = 0; i < nbVertices; i+=3)
			{
				btVector3 vertex0(vertices[3*i], vertices[(3*i)+1],  vertices[(3*i)+2]);
				btVector3 vertex1(vertices[3*(i+1)], vertices[(3*(i+1))+1],  vertices[(3*(i+1))+2]);
				btVector3 vertex2(vertices[3*(i+2)], vertices[(3*(i+2))+1],  vertices[(3*(i+2))+2]);
				//Attention au sens de la normale !
				trimesh->addTriangle(vertex2,vertex1,vertex0);
			}
			btConvexShape * tmpshape = new btConvexTriangleMeshShape(trimesh);
			btShapeHull * hull = new btShapeHull(tmpshape);
			btScalar margin = tmpshape->getMargin();
			hull->buildHull(margin);
			shape->Shape = hull;

			shape->Hull = new btConvexHullShape();
			for(int i=0;i<hull->numVertices();i++)
				shape->Hull->addPoint(hull->getVertexPointer()[i]);

			SAFEDELETE(trimesh);

			return shape;
		}

		NYBody * addConvexHullObject(bool dynamic, NYHullShape * colShape, const NYVert3Df & position, void * owner, int ownerType, NYBody * createdBody = NULL)
		{
			return addObject(dynamic, colShape->Hull, position, owner, ownerType, createdBody);
		}


		NYTriShape * calcTriMeshShape(float * vertices, int nbVertices,int stride)
		{
			NYTriShape * shape = new NYTriShape();

			btTriangleMesh * trimesh = new btTriangleMesh();
			Log::log(Log::ENGINE_INFO,("Building trimesh shape of " + toString(nbVertices) + " vertices").c_str());

			for (int i = 0; i < nbVertices; i+=3)
			{
				btVector3 vertex0(vertices[3*i], vertices[(3*i)+1],  vertices[(3*i)+2]);
				btVector3 vertex1(vertices[3*(i+1)], vertices[(3*(i+1))+1],  vertices[(3*(i+1))+2]);
				btVector3 vertex2(vertices[3*(i+2)], vertices[(3*(i+2))+1],  vertices[(3*(i+2))+2]);

				//Attention au sens de la normale !
				trimesh->addTriangle(vertex2,vertex1,vertex0);
			}

			shape->MeshShape = new btGImpactMeshShape(trimesh);
			shape->MeshShape->setMargin(0.2f);
			shape->MeshShape->updateBound();

			//si modif call btGImpactMeshShape.postUpdate()

			return shape;

		}

		NYCompoundShape * createCompoundShape(void)
		{
			NYCompoundShape * shape = new NYCompoundShape();

			shape->CompoundShape = new btCompoundShape();

			return shape;
		}


		void addSphereShapeToCompound(NYCompoundShape * comp, NYVert3Df center, float radius, void * owner = NULL)
		{
			//On regarde si la shape n'existe pas deja
			btCollisionShape* colShape = getSphereColShape(radius);

			//Si elle n'existe pas on la cree
			if(colShape == NULL)
			{
				colShape = new btSphereShape(btScalar(radius));
				CollisionShapes.push_back(colShape);
			}

			colShape->setUserPointer(owner);

			btTransform trans;
			btVector3 origin(center.X,center.Y,center.Z);
			trans.setIdentity();
			trans.setOrigin(origin);
			comp->CompoundShape->addChildShape(trans,colShape);
		}

		void addBoxShapeToCompound(NYCompoundShape * comp, NYVert3Df center, NYQuaternion rot,  NYVert3Df size, void * owner)
		{
			//On regarde si la shape n'existe pas deja
			/*btCollisionShape* colShape = getBoxColShape(size);

			//Si elle n'existe pas on la cree
			if(colShape == NULL)
			{
				colShape = new btBoxShape(btVector3(size.X,size.Y,size.Z));
				CollisionShapes.push_back(colShape);
			}*/

			btCollisionShape * colShape = new btBoxShape(btVector3(size.X,size.Y,size.Z));
			colShape->setUserPointer(owner);

			btTransform trans;
			btVector3 origin(center.X,center.Y,center.Z);
			trans.setIdentity();
			trans.setOrigin(origin);
			trans.setRotation(btQuaternion(rot.X,rot.Y,rot.Z,rot.T));
			comp->CompoundShape->addChildShape(trans,colShape);
		}

		NYBody * addCompoundObject(bool dynamic, NYCompoundShape * colShape, const NYVert3Df & position, void * owner, int ownerType, NYBody * createdBody = NULL)
		{
			return addObject(dynamic, colShape->CompoundShape, position, owner, ownerType, createdBody);
		}


		NYBody * addTriMeshObject(bool dynamic, NYTriShape * colShape, const NYVert3Df & position, void * owner, int ownerType, NYBody * createdBody = NULL)
		{
			return addObject(dynamic, colShape->MeshShape, position, owner, ownerType, createdBody);
		}

		


		

		void removeObject(NYBody * Body)
		{
			DynamicsWorld->removeRigidBody(Body->Body);
		}

		void step (float elapsed)
		{
			//Physique
			DynamicsWorld->stepSimulation(elapsed);
		}

		void render(void)
		{
			//DynamicsWorld->debugDrawWorld();
		}

	private:

		btCollisionShape * getSphereColShape(float radius)
		{
			for(int i=0;i<CollisionShapes.size();i++)
			{
				if(CollisionShapes[i]->getShapeType() == SPHERE_SHAPE_PROXYTYPE)
				{
					if(((btSphereShape*)CollisionShapes[i])->getRadius() == radius)
					{
						return CollisionShapes[i];
					}
				}
			}
			return NULL;
		}

		btCollisionShape * getPlaneColShape(const NYVert3Df & normal)
		{
			for(int i=0;i<CollisionShapes.size();i++)
			{
				if(CollisionShapes[i]->getShapeType() == STATIC_PLANE_PROXYTYPE)
				{
					if(((btStaticPlaneShape*)CollisionShapes[i])->getPlaneNormal() == btVector3(normal.X,normal.Y,normal.Z))
					{
						return CollisionShapes[i];
					}
				}
			}
			return NULL;
		}

		btCollisionShape * getBoxColShape(const NYVert3Df & size)
		{
			for(int i=0;i<CollisionShapes.size();i++)
			{
				if(CollisionShapes[i]->getShapeType() == BOX_SHAPE_PROXYTYPE )
				{
					if(((btBoxShape*)CollisionShapes[i])->getHalfExtentsWithoutMargin()== btVector3(size.X,size.Y,size.Z))
					{
						return CollisionShapes[i];
					}
				}
			}
			return NULL;
		}

		/**
		createdBody permet d'utiliser un body cree soi meme, par exemple quand on herite de body pour creer sa propre fonction de collisison
		*/
		NYBody * addObject(bool dynamic, btCollisionShape* colShape, const NYVert3Df & position, void * owner, int ownerType, NYBody * createdBody = NULL)
		{
			//On cree l'objet
			btTransform startTransform;
			startTransform.setIdentity();

			//On cree le corps
			NYBody * body = createdBody;
			if (body == NULL)
				body = new NYBody();

			body->Mass = 0;
			if (dynamic)
				body->Mass = 1.0f;

			btVector3 localInertia(0,0,0);
			if (dynamic)
				colShape->calculateLocalInertia(body->Mass,localInertia);
			body->CollShape = colShape;
			body->Dynamic = dynamic;

			startTransform.setOrigin(btVector3(btScalar(position.X),btScalar(position.Y),btScalar(position.Z)));

			btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
			btRigidBody::btRigidBodyConstructionInfo rbInfo(body->Mass,myMotionState,colShape,localInertia);
			body->Body = new btRigidBody(rbInfo);
			body->Body->setUserPointer(body);
			body->BodyOwner = owner;
			body->BodyOwnerType = ownerType;

			float size = body->getSize();

			body->Body->setActivationState(ISLAND_SLEEPING);
			body->Body->setSleepingThresholds(0.01f,0.01f);
			//body->Body->setActivationState(ACTIVE_TAG);
			DynamicsWorld->addRigidBody(body->Body);
			body->Body->setActivationState(ACTIVE_TAG);

			return body;
		}

		

		/**
		* Besoin d'un fix pour fonctionner avec les concave shapes :
		* rajouter
		* if(m_manifoldPtr != NULL)
		*    m_manifoldPtr->refreshContactPoints(body0->getWorldTransform(), body1->getWorldTransform());
		* a la fin de btGImpactCollisionAlgorithm::processCollision(); (from bullet wiki)
		*/
		void setCollisionReportFunction(bool(*ContactProcessedCallback)(btManifoldPoint& cp, void* body0, void* body1))
		{
			gContactProcessedCallback = ContactProcessedCallback;
		}

		static bool collisionReport(btManifoldPoint& cp, void* body0, void* body1)
		{
			btRigidBody* b0 = (btRigidBody*)body0;
			btRigidBody* b1 = (btRigidBody*)body1;
			NYBody * nyb0 = (NYBody*)b0->getUserPointer();
			NYBody * nyb1 = (NYBody*)b1->getUserPointer();

			nyb0->collision(nyb1, cp);
			nyb1->collision(nyb0, cp);
			return false;
		}



};

#endif
