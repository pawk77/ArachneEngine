// event_manager.hpp

#pragma once

#include <memory>
#include <queue>
#include <unordered_map>
#include <functional>
#include <vector>
#include "object.hpp"
#include "id_vec.hpp"
#include "types.hpp"

namespace harpy
{
    class cDataBuffer;
    class cContext;
    class cGameObject;
    
    using EventFunction = std::function<void(cDataBuffer* const data)>;

    class cEventHandler
    {
        friend class mEvent;

    public:
        cEventHandler(eEventType type, cGameObject* receiver, EventFunction&& function);
        ~cEventHandler() = default;

        void Invoke(cDataBuffer* data);
        inline cGameObject* GetReceiver() const { return _receiver; }
        inline eEventType GetEventType() const { return _type; }
        inline std::shared_ptr<EventFunction> GetFunction() const { return _function; }

    private:
        eEventType _type = eEventType::NONE;
        cGameObject* _receiver = nullptr;
        mutable std::shared_ptr<EventFunction> _function;
    };

    class cEventDispatcher : public iObject
    {
        REALWARE_OBJECT(cEventDispatcher)

    public:
        explicit cEventDispatcher(cContext* context);
        virtual ~cEventDispatcher() = default;

        void Subscribe(const std::string& id, eEventType type);
        void Unsubscribe(eEventType type, cGameObject* receiver);
        void Send(eEventType type);
        void Send(eEventType type, cDataBuffer* data);

    private:
        std::unordered_map<eEventType, std::shared_ptr<std::vector<cEventHandler>>> _listeners;
    };
}