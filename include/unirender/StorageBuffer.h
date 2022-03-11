#pragma once

#include <string>

namespace ur
{

class ShaderProgram;

class StorageBuffer
{
public:
    virtual ~StorageBuffer() {}

    virtual void ReadFromMemory(const void* data, int size, int offset) = 0;
    virtual void* WriteToMemory(int size, int offset) = 0;

    virtual void Bind() const = 0;
    virtual void UnBind() const = 0;

}; // StorageBuffer

}