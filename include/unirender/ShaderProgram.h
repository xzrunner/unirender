#pragma once

#include "unirender/ShaderType.h"

#include <string>
#include <unordered_map>
#include <memory>

#ifdef DrawState
#undef DrawState
#endif

namespace ur
{

class Uniform;
class UniformUpdater;
class Context;
struct DrawState;
class StorageBuffer;

class ShaderProgram
{
public:
    virtual ~ShaderProgram() {}

    virtual void Bind() const = 0;
    virtual bool CheckStatus() const = 0;

    virtual void GetComputeWorkGroupSize(int& x, int& y, int& z) const = 0;

    virtual int QueryTexSlot(const std::string& name) const = 0;
    virtual int QueryAttrLoc(const std::string& name) const = 0;
    virtual int QueryImgSlot(const std::string& name) const = 0;

    virtual void BindSSBO(const std::string& name, int idx, 
        const std::shared_ptr<StorageBuffer>& ssbo) const = 0;

    virtual bool HasStage(ShaderType stage) const = 0;

    void AddUniform(const std::string& name,
        const std::shared_ptr<Uniform>& uniform);
    std::shared_ptr<Uniform>
        QueryUniform(const std::string& name) const;

    void AddUniformUpdater(const std::shared_ptr<UniformUpdater>& updater);
    std::shared_ptr<UniformUpdater>
        QueryUniformUpdater(size_t type_id);

    void Clean(const Context& ctx, const DrawState& draw, const void* scene);

private:
    mutable std::unordered_map<std::string, std::shared_ptr<ur::Uniform>> m_uniforms;

    std::unordered_map<size_t, std::shared_ptr<UniformUpdater>> m_updaters;

}; // ShaderProgram

}