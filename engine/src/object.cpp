// object.cpp

#include "object.hpp"
#include "event_manager.hpp"
#include "context.hpp"

namespace harpy
{
	cIdentifier::cIdentifier(const std::string& id) : _id(id) {}

    iObject::~iObject()
    {
        delete _identifier;
    }

    void iObject::Subscribe(const std::string& id, eEventType type)
    {
        cEventDispatcher* dispatcher = _context->GetSubsystem<cEventDispatcher>();
        dispatcher->Subscribe(id, type);
    }

    void iObject::Unsubscribe(eEventType type, cGameObject* receiver)
    {
        cEventDispatcher* dispatcher = _context->GetSubsystem<cEventDispatcher>();
        dispatcher->Unsubscribe(type, receiver);
    }

    void iObject::Send(eEventType type)
    {
        cEventDispatcher* dispatcher = _context->GetSubsystem<cEventDispatcher>();
        dispatcher->Send(type);
    }

    void iObject::Send(eEventType type, cDataBuffer* data)
    {
        cEventDispatcher* dispatcher = _context->GetSubsystem<cEventDispatcher>();
        dispatcher->Send(type, data);
    }
}