// physics_manager.cpp

#include <windows.h>
#include <cooking/PxCooking.h>
#include "../../thirdparty/glm/glm/gtc/quaternion.hpp"
#include "application.hpp"
#include "render_manager.hpp"
#include "physics_manager.hpp"
#include "gameobject_manager.hpp"

using namespace physx;

namespace realware
{
    using namespace app;
    using namespace game;
    using namespace render;
    using namespace types;

    namespace physics
    {
        PxFilterFlags FilterShader(
            PxFilterObjectAttributes attributes0, PxFilterData filterData0,
            PxFilterObjectAttributes attributes1, PxFilterData filterData1,
            PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
        {
            if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
            {
                pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
                return PxFilterFlag::eDEFAULT;
            }
            pairFlags = PxPairFlag::eCONTACT_DEFAULT;

            if ((filterData0.word0 & filterData1.word0) && (filterData1.word1 & filterData0.word1))
                pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;

            return PxFilterFlag::eDEFAULT;
        }

        mPhysics::mPhysics(const cApplication* const app) :
            _app((cApplication*)app),
            _allocator(new cAllocator()),
            _error(new cError()),
            _cpuDispatcher(new cCPUDispatcher()),
            _simulationEvent(new cSimulationEvent()),
            _scenes(_app, ((cApplication*)_app)->GetDesc()->MaxPhysicsSceneCount),
            _substances(_app, ((cApplication*)_app)->GetDesc()->MaxPhysicsSubstanceCount),
            _actors(_app, ((cApplication*)_app)->GetDesc()->MaxPhysicsActorCount),
            _controllers(_app, ((cApplication*)_app)->GetDesc()->MaxPhysicsControllerCount)
        {
            _foundation = PxCreateFoundation(PX_PHYSICS_VERSION, *_allocator, *_error);
            if (_foundation == nullptr)
            {
                MessageBox(0, "Failed to initialize PhysXFoundation!", "Error", MB_ICONERROR);
                return;
            }

            _physics = PxCreatePhysics(PX_PHYSICS_VERSION, *_foundation, PxTolerancesScale(), false, nullptr);
            if (_physics == nullptr)
            {
                MessageBox(0, "Failed to initialize PhysXPhysics!", "Error", MB_ICONERROR);
                return;
            }
        }

        mPhysics::~mPhysics()
        {
            _physics->release();
            _foundation->release();
            delete _simulationEvent;
            delete _cpuDispatcher;
            delete _error;
            delete _allocator;
        }

        sSimulationScene* mPhysics::CreateScene(const std::string& id, const glm::vec3& gravity)
        {
            PxSceneDesc sceneDesc(_physics->getTolerancesScale());
            sceneDesc.gravity = PxVec3(gravity.y, gravity.x, gravity.z);
            sceneDesc.cpuDispatcher = _cpuDispatcher;
            sceneDesc.filterShader = PxDefaultSimulationFilterShader;

            PxScene* scene = _physics->createScene(sceneDesc);

            PxControllerManager* controllerManager = PxCreateControllerManager(*scene);

            return _scenes.Add(id, scene, controllerManager);
        }

        sSubstance* mPhysics::CreateSubstance(const std::string& id, const glm::vec3& params)
        {
            PxMaterial* material = _physics->createMaterial(params.x, params.y, params.z); // (staticFriction, dynamicFriction, restitution)

            return _substances.Add(id, material);
        }

        sController* mPhysics::CreateController(const std::string& id, const f32 eyeHeight, const f32 height, const f32 radius, const render::sTransform* const transform, const glm::vec3& up, const sSimulationScene* const scene, const sSubstance* const substance)
        {
            glm::vec3 position = transform->Position;

            PxCapsuleControllerDesc desc;
            desc.setToDefault();
            desc.height = height;
            desc.radius = radius;
            desc.position = PxExtendedVec3(position.y, position.x, position.z);
            desc.stepOffset = 0.5f;
            desc.slopeLimit = cosf(PxPi / 4.0f);
            desc.contactOffset = 0.01f;
            desc.upDirection = PxVec3(up.y, up.x, up.z);
            desc.material = substance->Substance;

            PxController* controller = scene->ControllerManager->createController(desc);

            return _controllers.Add(id, controller, eyeHeight);
        }

        sActor* mPhysics::CreateActor(const std::string& id, const Category& staticOrDynamic, const Category& shapeType, const sSimulationScene* const scene, const sSubstance* const substance, const f32 mass, const sTransform* const transform, const cGameObject* const gameObject)
        {
            glm::vec3 position = transform->Position;
            glm::vec3 scale = transform->Scale;

            PxTransform pose(PxVec3(position.y, position.x, position.z));
            
            PxShape* shape = nullptr;
            if (shapeType == Category::PHYSICS_SHAPE_PLANE)
                shape = _physics->createShape(PxPlaneGeometry(), *substance->Substance, false, PxShapeFlag::eSIMULATION_SHAPE | PxShapeFlag::eSCENE_QUERY_SHAPE);
            else if (shapeType == Category::PHYSICS_SHAPE_BOX)
                shape = _physics->createShape(PxBoxGeometry(scale.y * 0.5f, scale.x * 0.5f, scale.z * 0.5f), *substance->Substance, false, PxShapeFlag::eSIMULATION_SHAPE | PxShapeFlag::eSCENE_QUERY_SHAPE);

            if (shape == nullptr)
                return nullptr;

            shape->setContactOffset(0.1f);
            shape->setRestOffset(0.05f);
            
            PxActor* actor = nullptr;
            if (staticOrDynamic == Category::PHYSICS_ACTOR_STATIC)
            {
                actor = _physics->createRigidStatic(pose);
                ((PxRigidStatic*)actor)->attachShape(*shape);
            }
            else if (staticOrDynamic == Category::PHYSICS_ACTOR_DYNAMIC)
            {
                actor = _physics->createRigidDynamic(pose);
                ((PxRigidDynamic*)actor)->attachShape(*shape);
                ((PxRigidDynamic*)actor)->setAngularDamping(0.75f);
                ((PxRigidDynamic*)actor)->setLinearVelocity(PxVec3(0.0f, 0.0f, 0.0f));
                PxRigidBodyExt::updateMassAndInertia(*((PxRigidBody*)actor), mass);
            }

            if (shape != nullptr)
                shape->release();

            if (actor != nullptr)
                scene->Scene->addActor(*actor);

            return _actors.Add(id, gameObject, actor, staticOrDynamic);
        }

        sSimulationScene* mPhysics::FindScene(const std::string& id)
        {
            return _scenes.Find(id);
        }

        sSubstance* mPhysics::FindSubstance(const std::string& id)
        {
            return _substances.Find(id);
        }

        sActor* mPhysics::FindActor(const std::string& id)
        {
            return _actors.Find(id);
        }

        sController* mPhysics::FindController(const std::string& id)
        {
            return _controllers.Find(id);
        }

        void mPhysics::DestroyScene(const std::string& id)
        {
            _scenes.Delete(id);
        }

        void mPhysics::DestroySubstance(const std::string& id)
        {
            _substances.Delete(id);
        }

        void mPhysics::DestroyActor(const std::string& id)
        {
            _actors.Delete(id);
        }

        void mPhysics::DestroyController(const std::string& id)
        {
            _controllers.Delete(id);
        }

        void mPhysics::MoveController(const sController* const controller, const glm::vec3& position, const f32 minStep)
        {
            PxController* pxController = controller->Controller;
            f32 deltaTime = _app->GetDeltaTime();

            PxControllerFilters filters = PxControllerFilters();
            PxU32 collisionFlags = pxController->move(
                PxVec3(position.y, position.x, position.z),
                minStep,
                deltaTime,
                filters
            );
        }

        glm::vec3 mPhysics::GetControllerPosition(const sController* const controller)
        {
            PxController* pxController = controller->Controller;
            PxExtendedVec3 position = pxController->getPosition();

            return glm::vec3(position.y, position.x + controller->EyeHeight, position.z);
        }

        void mPhysics::Simulate()
        {
            const auto& actors = _actors.GetObjects();
            const usize actorCount = actors.size();

            for (auto& pair : actors)
            {
                if (pair.Type != Category::PHYSICS_ACTOR_DYNAMIC)
                    continue;

                sTransform* const transform = pair.GameObject->GetTransform();
                PxActor* const pxActor = pair.Actor;

                const PxTransform actorTransform = ((PxRigidDynamic*)pxActor)->getGlobalPose();
                const glm::quat q = glm::quat(
                    actorTransform.q.w,
                    actorTransform.q.x,
                    actorTransform.q.y,
                    actorTransform.q.z
                );
                const glm::vec3 actorEuler = glm::eulerAngles(q);

                {
                    std::lock_guard<std::mutex> lock(_mutex);
                    transform->Position = glm::vec3(actorTransform.p.y, actorTransform.p.x, actorTransform.p.z);
                    transform->Rotation = glm::vec3(actorEuler.y, actorEuler.x, actorEuler.z);
                }
            }

            const auto& scenes = _scenes.GetObjects();
            const usize sceneCount = scenes.size();

            for (usize i = 0; i < sceneCount; i++)
            {
                PxScene* const scene = scenes[i].Scene;
                scene->simulate(1.0f / 60.0f);
                scene->fetchResults(true);
            }
        }

        /*void mPhysics::Update()
        {
            // Actors
            for (auto& pair : _actors)
            {
                sCTransform* transformComponent = pair.Scene->Get<sCTransform>(pair.Entity);
                sCPhysicsActor* actorComponent = pair.Scene->Get<sCPhysicsActor>(pair.Entity);

                PxTransform actorTransform = ((PxRigidDynamic*)actorComponent->Actor)->getGlobalPose();
                glm::vec3 actorEuler = glm::eulerAngles(
                    glm::quat(
                        actorTransform.q.w,
                        actorTransform.q.x,
                        actorTransform.q.y,
                        actorTransform.q.z
                    )
                );

                {
                    std::lock_guard<std::mutex> lock(_mutex);
                    transformComponent->Position = glm::vec3(actorTransform.p.y, actorTransform.p.x, actorTransform.p.z);
                    transformComponent->Rotation = glm::vec3(actorEuler.y, actorEuler.x, actorEuler.z);
                }
            }

            // Controllers
            for (auto& pair : _controllers)
            {
                sCTransform* transformComponent = pair.Scene->Get<sCTransform>(pair.Entity);
                sCPhysicsCharacterController* controllerComponent =
                    pair.Scene->Get<sCPhysicsCharacterController>(pair.Entity);

                if (controllerComponent->IsGravityEnabled == K_TRUE) {
                    SetCharacterControllerMovement(
                        pair, glm::vec3(0.0f, -controllerComponent->GravitySpeed, 0.0f)
                    );
                }

                PxExtendedVec3 position = controllerComponent->Controller->getPosition();
                
                {
                    std::lock_guard<std::mutex> lock(_mutex);
                    transformComponent->Position = glm::vec3(
                        position.y, position.x, position.z
                    );
                }
            }

            // Scenes
            for (auto& pair : _scenes)
            {
                sCPhysicsScene* component = pair.Scene->Get<sCPhysicsScene>(pair.Entity);
                component->Scene->simulate(1.0f / 60.0f);
                component->Scene->fetchResults(true);
            }
        }

        sCPhysicsScene* mPhysics::AddScene(const sEntityScenePair& scene)
        {
            PxSceneDesc sceneDesc(_physics->getTolerancesScale());
            sceneDesc.gravity = PxVec3(-9.81f, 0.0f, 0.0f);
            sceneDesc.cpuDispatcher = _cpuDispatcher;
            sceneDesc.simulationEventCallback = _simulationEvent;
            sceneDesc.filterShader = FilterShader;

            sCPhysicsScene* component = scene.Scene->Add<sCPhysicsScene>(scene.Entity);
            component->Scene = _physics->createScene(sceneDesc);
            component->ControllerManager = PxCreateControllerManager(*component->Scene);

            _scenes.push_back(scene);

            return component;
        }

        sCPhysicsActor* mPhysics::AddActor(
            const sEntityScenePair& scene,
            const sEntityScenePair& actor,
            const mPhysics::eActorDescriptor& actorDesc,
            const mPhysics::eShapeDescriptor& shapeDesc,
            const glm::vec4& extents,
            const sVertexBufferGeometry* const geometry
        )
        {
            sCTransform* transformComponent = actor.Scene->Get<sCTransform>(actor.Entity);

            glm::vec3& entityPosition = transformComponent->Position;
            glm::vec3 position = glm::vec3(entityPosition.y, entityPosition.x, entityPosition.z);

            // Creating material
            PxMaterial* physicsMaterial = _physics->createMaterial(0.05f, 0.05f, 0.05f);

            // Creation of actor
            PxActor* physicsActor = nullptr;
            
            switch (actorDesc)
            {

            case mPhysics::eActorDescriptor::STATIC:
                physicsActor = _physics->createRigidStatic(PxTransform(position.x, position.y, position.z));
                break;

            case mPhysics::eActorDescriptor::DYNAMIC:
                physicsActor = _physics->createRigidDynamic(PxTransform(position.x, position.y, position.z));
                ((PxRigidDynamic*)physicsActor)->setAngularDamping(0.75f);
                ((PxRigidDynamic*)physicsActor)->setLinearVelocity(PxVec3(0.0f, 0.0f, 0.0f));
                break;

            }

            if (physicsActor == nullptr) { return nullptr; }

            sCPhysicsScene* sceneComponent = scene.Scene->Get<sCPhysicsScene>(scene.Entity);
            sceneComponent->Scene->addActor(*physicsActor);

            // Shape creation
            PxShape* physicsShape = nullptr;

            switch (shapeDesc)
            {
                case mPhysics::eShapeDescriptor::SPHERE:
                {
                    physicsShape = _physics->createShape(
                        PxSphereGeometry(extents.x),
                        *physicsMaterial
                    );
                    break;
                }

                case mPhysics::eShapeDescriptor::CAPSULE:
                {
                    physicsShape = _physics->createShape(
                        PxCapsuleGeometry(extents.x, extents.y),
                        *physicsMaterial
                    );
                    break;
                }

                case mPhysics::eShapeDescriptor::BOX:
                {
                    physicsShape = _physics->createShape(
                        PxBoxGeometry(extents.x, extents.y, extents.z),
                        *physicsMaterial
                    );
                    break;
                }

                case mPhysics::eShapeDescriptor::PLANE:
                {
                    physicsShape = _physics->createShape(
                        PxPlaneGeometry(),
                        *physicsMaterial
                    );
                    break;
                }

                case mPhysics::eShapeDescriptor::TRIANGLE_MESH:
                {
                    if (geometry->Format !=
                        render::sVertexBufferGeometry::eFormat::POSITION_TEXCOORD_NORMAL_VEC3_VEC2_VEC3)
                    {
                        MessageBox(0, "PhysX shape 'TRIANGLE_MESH' can only be created with geometry of 'POSITION_TEXCOORD_NORMAL_VEC3_VEC2_VEC3' format!", "Error", MB_ICONERROR);
                        return nullptr;
                    }

                    PxVec3* verticesPX = new PxVec3[geometry->VertexCount];
                    for (usize i = 0; i < geometry->VertexCount; i++) {
                        verticesPX[i] = PxVec3(
                            ((render::sVertex*)geometry->VertexPtr)[i].Position.y,
                            ((render::sVertex*)geometry->VertexPtr)[i].Position.x,
                            ((render::sVertex*)geometry->VertexPtr)[i].Position.z
                        );
                    }

                    PxTriangleMeshDesc meshDesc;
                    meshDesc.points.count = geometry->VertexCount;
                    meshDesc.points.stride = sizeof(PxVec3);
                    meshDesc.points.data = verticesPX;

                    meshDesc.triangles.count = geometry->IndexCount / 3;
                    meshDesc.triangles.stride = 3 * sizeof(PxU32);
                    meshDesc.triangles.data = geometry->IndexPtr;

                    PxTolerancesScale scale(1.0f, 10.0f);
                    PxCookingParams params(scale);
                    PxDefaultMemoryOutputStream writeBuffer;
                    PxTriangleMeshCookingResult::Enum result;
                    bool status = PxCookTriangleMesh(params, meshDesc, writeBuffer, &result);
                    if (!status || result == PxTriangleMeshCookingResult::eFAILURE)
                    {
                        MessageBox(0, "PhysX can't cook triangle mesh!", "Error", MB_ICONERROR);
                        return nullptr;
                    }

                    PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
                    PxTriangleMesh* triangleMesh = _physics->createTriangleMesh(readBuffer);
                    
                    ((PxRigidDynamic*)physicsActor)->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
                    
                    physicsShape = _physics->createShape(
                        PxTriangleMeshGeometry(
                            triangleMesh,
                            PxMeshScale(PxVec3(1.0f, 1.0f, 1.0f))
                        ),
                        *physicsMaterial,
                        false,
                        PxShapeFlag::eSIMULATION_SHAPE
                    );

                    delete[] verticesPX;

                    break;
                }
            }

            if (physicsShape == nullptr) { return nullptr; }

            physicsShape->setContactOffset(0.1f);
            physicsShape->setRestOffset(0.05f);

            PxFilterData FilterData;
            FilterData.word0 = 0; FilterData.word1 = 0; FilterData.word2 = 0; FilterData.word3 = 0;
            physicsShape->setSimulationFilterData(FilterData);

            switch (actorDesc)
            {

            case mPhysics::eActorDescriptor::STATIC:
                ((PxRigidStatic*)physicsActor)->attachShape(*physicsShape);
                break;

            case mPhysics::eActorDescriptor::DYNAMIC:
                ((PxRigidDynamic*)physicsActor)->attachShape(*physicsShape);
                break;

            }
            
            sCPhysicsActor* component = actor.Scene->Add<sCPhysicsActor>(actor.Entity);
            component->Actor = physicsActor;
            component->Material = physicsMaterial;

            _actors.push_back(actor);

            return component;
        }

        sCPhysicsCharacterController* mPhysics::AddCharacterController(
            const sEntityScenePair& scene,
            const sEntityScenePair& controller,
            const mPhysics::eShapeDescriptor& shapeDesc,
            const glm::vec4& extents
        )
        {
            sCPhysicsScene* sceneComponent = scene.Scene->Get<sCPhysicsScene>(scene.Entity);
            sCTransform* transformComponent = controller.Scene->Get<sCTransform>(controller.Entity);
            PxControllerManager* controllerManager = sceneComponent->ControllerManager;
            
            PxMaterial* physicsMaterial = _physics->createMaterial(0.05f, 0.05f, 0.05f);

            PxController* physicsController = nullptr;
            switch (shapeDesc)
            {
                case mPhysics::eShapeDescriptor::CAPSULE:
                {
                    PxCapsuleControllerDesc desc = PxCapsuleControllerDesc();
                    desc.setToDefault();
                    desc.radius = extents.x;
                    desc.height = extents.y;
                    desc.position = PxExtendedVec3(
                        transformComponent->Position.y,
                        transformComponent->Position.x,
                        transformComponent->Position.z
                    );
                    desc.upDirection = PxVec3(1.0f, 0.0f, 0.0f);
                    desc.contactOffset = 0.01;
                    desc.stepOffset = desc.height * 0.25f;
                    desc.slopeLimit = glm::cos(glm::radians(45.0f));
                    desc.material = physicsMaterial;

                    physicsController = controllerManager->createController(desc);
                    break;
                }

                case mPhysics::eShapeDescriptor::BOX:
                {
                    PxBoxControllerDesc desc = PxBoxControllerDesc();
                    desc.halfForwardExtent = extents.x;
                    desc.halfSideExtent = extents.y;
                    desc.halfHeight = extents.z;
                    desc.stepOffset = desc.halfHeight * 0.25f;
                    desc.slopeLimit = glm::cos(glm::radians(45.0f));
                    desc.material = physicsMaterial;

                    physicsController = controllerManager->createController(desc);
                    break;
                }
            }

            sCPhysicsCharacterController* controllerComponent =
                controller.Scene->Add<sCPhysicsCharacterController>(controller.Entity);
            controllerComponent->Controller = physicsController;
            controllerComponent->Material = physicsMaterial;

            _controllers.push_back(controller);
            
            return controllerComponent;
        }

        void mPhysics::SetForce(const sEntityScenePair& actor, const glm::vec3& force)
        {
            sCPhysicsActor* component = actor.Scene->Get<sCPhysicsActor>(actor.Entity);
            ((PxRigidDynamic*)component->Actor)->setForceAndTorque(
                PxVec3(force.y, force.x, force.z),
                PxVec3(0.0f, 0.0f, 0.0f)
            );
        }

        void mPhysics::SetCharacterControllerMovement(const sEntityScenePair& controller, const glm::vec3& direction)
        {
            sCPhysicsCharacterController* controllerComponent =
                controller.Scene->Get<sCPhysicsCharacterController>(controller.Entity);

            PxControllerFilters filters = PxControllerFilters();
            controllerComponent->Controller->move(
                PxVec3(direction.y, direction.x, direction.z),
                0.01f,
                1.0f,
                filters
            );
        }*/
    }
}