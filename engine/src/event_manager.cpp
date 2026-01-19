// event_manager.cpp

#pragma once

#include "application.hpp"
#include "gameobject_manager.hpp"
#include "event_manager.hpp"
#include "buffer.hpp"

using namespace types;

namespace harpy
{
    cEventHandler::cEventHandler(eEventType type, cGameObject* receiver, EventFunction&& function) : _receiver(receiver), _type(type), _function(std::make_shared<EventFunction>(std::move(function))) {}

    void cEventHandler::Invoke(cDataBuffer* data)
    {
        _function->operator()(data);
    };

    cEventDispatcher::cEventDispatcher(cContext* context) : iObject(context) {}

    void cEventDispatcher::Subscribe(const std::string& id, eEventType type)
    {
        const auto listener = _listeners.find(type);
        if (listener == _listeners.end())
            _listeners.insert({ type, std::make_shared<std::vector<cEventHandler>>() });
        _listeners[type]->emplace_back(id, type);
    }

    void cEventDispatcher::Unsubscribe(eEventType type, cGameObject* receiver)
    {
        if (_listeners.find(type) == _listeners.end())
            return;

        auto& events = _listeners[type];
        auto it = events->begin();
        while (it != events->end())
        {
            const cGameObject* listenerReceiver = it->GetReceiver();
            if (listenerReceiver == receiver)
            {
                it = events->erase(it);
                return;
            }
            else
            {
                ++it;
            }
        }
    }

    void cEventDispatcher::Send(eEventType type)
    {
        cDataBuffer data(_context);

        Send(type, &data);
    }

    void cEventDispatcher::Send(eEventType type, cDataBuffer* data)
    {
        if (_listeners.find(type) == _listeners.end())
            return;

        auto& events = _listeners[type];
        for (usize i = 0; i < events->size(); i++)
            events->data()[i].Invoke(data);
    }
}