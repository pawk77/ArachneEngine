// physics_manager.hpp

#pragma once

#include <iostream>
#include <vector>
#include <mutex>
#include <PxPhysics.h>
#include <PxPhysicsAPI.h>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "category.hpp"
#include "id_vec.hpp"
#include "log.hpp"

namespace physx
{
    class PxActor;
    class PxRigidDynamic;
    class PxShape;
    class PxMaterial;
    class PxJoint;
    class PxScene;
    class PxFoundation;
    class PxPhysics;
}

namespace realware
{
    namespace app
    {
        class cApplication;
    }

    namespace render
    {
        struct sVertexBufferGeometry;
        struct sTransform;
    }

    namespace game
    {
        class cGameObject;
    }

    namespace physics
    {
        class cAllocator : public physx::PxAllocatorCallback
        {
            virtual void* allocate(size_t size, const char* typeName, const char* filename, int line) override final
            {
                return malloc(size);
            }

            virtual void deallocate(void* ptr) override final
            {
                free(ptr);
            }
        };

        class cError : public physx::PxErrorCallback
        {
            virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override final
            {
                log::Print(message);
            }
        };

        class cCPUDispatcher : public physx::PxCpuDispatcher
        {
            virtual void submitTask(physx::PxBaseTask& task) override final
            {
                task.run();
                task.release();
            }

            virtual uint32_t getWorkerCount() const override final
            {
                return 0;
            }
        };

        class cSimulationEvent : public physx::PxSimulationEventCallback
        {
			virtual void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override final {}
			virtual void onWake(physx::PxActor** actors, physx::PxU32 count) override final {}
			virtual void onSleep(physx::PxActor** actors, physx::PxU32 count) override final {}
			virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override final {}
			virtual void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override final {}
			virtual void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override final {}
        };

        struct sSimulationScene : public utils::sIdVecObject
        {
            sSimulationScene() = default;
            explicit sSimulationScene(const physx::PxScene* const scene, const physx::PxControllerManager* const controllerManager) : Scene((physx::PxScene*)scene), ControllerManager((physx::PxControllerManager*)controllerManager) {}
            ~sSimulationScene() = default;

            physx::PxScene* Scene = nullptr;
            physx::PxControllerManager* ControllerManager = nullptr;
        };

        struct sSubstance : public utils::sIdVecObject
        {
            sSubstance() = default;
            explicit sSubstance(const physx::PxMaterial* const substance) : Substance((physx::PxMaterial*)substance) {}
            ~sSubstance() = default;

            physx::PxMaterial* Substance = nullptr;
        };

        struct sController : public utils::sIdVecObject
        {
            sController() = default;
            explicit sController(const physx::PxController* const controller, const types::f32 eyeHeight) : Controller((physx::PxController*)controller), EyeHeight(eyeHeight) {}
            ~sController() = default;

            physx::PxController* Controller = nullptr;
            types::f32 EyeHeight = 0.0f;
        };

        struct sActor : public utils::sIdVecObject
        {
            sActor() = default;
            explicit sActor(const game::cGameObject* const gameObject, const physx::PxActor* const actor, const game::Category& actorType) : GameObject((game::cGameObject*)gameObject), Actor((physx::PxActor*)actor), Type(actorType) {}
            ~sActor() = default;

            game::cGameObject* GameObject = nullptr;
            physx::PxActor* Actor = nullptr;
            game::Category Type = game::Category::PHYSICS_ACTOR_DYNAMIC;
        };

        class mPhysics
        {
        public:
            explicit mPhysics(const app::cApplication* const app);
            ~mPhysics();

            sSimulationScene* CreateScene(const std::string& id, const glm::vec3& gravity = glm::vec3(0.0f, -9.81f, 0.0f));
            sSubstance* CreateSubstance(const std::string& id, const glm::vec3& params = glm::vec3(0.5f, 0.5f, 0.6f));
            sActor* CreateActor(const std::string& id, const game::Category& staticOrDynamic, const game::Category& shapeType, const sSimulationScene* const scene, const sSubstance* const substance, const types::f32 mass, const render::sTransform* const transform, const game::cGameObject* const gameObject);
            sController* CreateController(const std::string& id, const types::f32 eyeHeight, const types::f32 height, const types::f32 radius, const render::sTransform* const transform, const glm::vec3& up, const sSimulationScene* const scene, const sSubstance* const substance);
            sSimulationScene* FindScene(const std::string&);
            sSubstance* FindSubstance(const std::string&);
            sActor* FindActor(const std::string&);
            sController* FindController(const std::string&);
            void DestroyScene(const std::string& id);
            void DestroySubstance(const std::string& id);
            void DestroyActor(const std::string& id);
            void DestroyController(const std::string& id);

            void MoveController(const sController* const controller, const glm::vec3& position, const types::f32 minStep = 0.001f);
            glm::vec3 GetControllerPosition(const sController* const controller);

            void Simulate();

        private:
            app::cApplication* _app = nullptr;
            cAllocator* _allocator = nullptr;
            cError* _error = nullptr;
            cCPUDispatcher* _cpuDispatcher = nullptr;
            cSimulationEvent* _simulationEvent = nullptr;
            physx::PxFoundation* _foundation = nullptr;
            physx::PxPhysics* _physics = nullptr;
            std::mutex _mutex;
            utils::cIdVec<sSimulationScene> _scenes;
            utils::cIdVec<sSubstance> _substances;
            utils::cIdVec<sActor> _actors;
            utils::cIdVec<sController> _controllers;
        };
    }
}