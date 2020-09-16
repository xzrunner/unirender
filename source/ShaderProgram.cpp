#include "unirender/ShaderProgram.h"
#include "unirender/Uniform.h"
#include "unirender/UniformUpdater.h"

namespace ur
{

void ShaderProgram::AddUniform(const std::string& name, const std::shared_ptr<Uniform>& uniform)
{
    m_uniforms.insert({ name, uniform });
}

std::shared_ptr<Uniform>
ShaderProgram::QueryUniform(const std::string& name) const
{
    auto itr = m_uniforms.find(name);
    if (itr != m_uniforms.end()) {
        return itr->second;
    }

    // for ubo, auto gen
    std::string str = "." + name;
    for (auto& unif : m_uniforms) 
    {
        if (unif.first.find(str) != std::string::npos &&
            !unif.first.empty() && unif.first[0] == '_') {
            return unif.second;
        }
    }

    return nullptr;
}

void ShaderProgram::AddUniformUpdater(const std::shared_ptr<UniformUpdater>& updater)
{
    m_updaters.insert(std::make_pair(updater->UpdaterTypeID(), updater));
}

std::shared_ptr<UniformUpdater>
ShaderProgram::QueryUniformUpdater(size_t type_id)
{
    auto itr = m_updaters.find(type_id);
    return itr == m_updaters.end() ? nullptr : itr->second;
}

void ShaderProgram::Clean(const Context& ctx, const DrawState& draw, const void* scene)
{
    for (auto& up : m_updaters) {
        up.second->Update(ctx, draw, scene);
    }
    for (auto& u : m_uniforms) {
        u.second->Clean();
    }
}

}