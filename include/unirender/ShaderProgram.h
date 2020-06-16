#pragma once

#include <string>
#include <unordered_map>

namespace ur
{

class Uniform;
class UniformUpdater;
class Context;
struct DrawState;

class ShaderProgram
{
public:
    virtual ~ShaderProgram() {}

    virtual void Bind() const = 0;
    virtual bool CheckStatus() const = 0;

    virtual int GetComputeWorkGroupSize() const = 0;

    virtual int QueryTexSlot(const std::string& name) const = 0;
    virtual int QueryAttrLoc(const std::string& name) const = 0;

    void AddUniform(const std::string& name,
        const std::shared_ptr<Uniform>& uniform);
    std::shared_ptr<Uniform>
        QueryUniform(const std::string& name) const;

    void AddUniformUpdater(const std::shared_ptr<UniformUpdater>& updater);
    std::shared_ptr<UniformUpdater>
        QueryUniformUpdater(size_t type_id);

    void Clean(const Context& ctx, const DrawState& draw, const void* scene);

private:
    std::unordered_map<std::string, std::shared_ptr<ur::Uniform>> m_uniforms;

    std::unordered_map<size_t, std::shared_ptr<UniformUpdater>> m_updaters;

}; // ShaderProgram

}