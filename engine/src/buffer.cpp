// buffer.cpp

#pragma once

#include "buffer.hpp"

namespace harpy
{
    cDataBuffer::cDataBuffer(cContext* context) : iObject(context) {}

    cDataBuffer::~cDataBuffer()
    {
    }

    void cDataBuffer::Create(void* data, types::usize byteSize)
    {
    }
}