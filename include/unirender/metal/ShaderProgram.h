#pragma once

#include "unirender/ShaderProgram.h"

#include <string>
#include <vector>

namespace ur
{
namespace metal
{

class ShaderProgram : public ur::ShaderProgram
{
public:
    // Construct from SPIR-V bytecode (cross-compiled to MSL via spirv-cross)
    ShaderProgram(void* mtl_device,
                  const std::vector<unsigned int>& vs,
                  const std::vector<unsigned int>& fs);

    // Construct from raw MSL source
    ShaderProgram(void* mtl_device, const std::string& msl_source);

    virtual ~ShaderProgram();

    virtual void Bind() const override {}
    virtual bool CheckStatus() const override { return m_valid; }

    virtual void GetComputeWorkGroupSize(int& x, int& y, int& z) const override;

    virtual int QueryTexSlot(const std::string& name) const override;
    virtual int QueryAttrLoc(const std::string& name) const override;
    virtual int QueryImgSlot(const std::string& name) const override;

    virtual void BindSSBO(const std::string& name, int idx,
        const std::shared_ptr<StorageBuffer>& ssbo) const override {}

    virtual bool HasStage(ShaderType stage) const override;

    void* GetVertexFunction()   const { return m_vert_func; }
    void* GetFragmentFunction() const { return m_frag_func; }
    void* GetMTLLibrary()       const { return m_mtl_library; }

private:
    void CompileFromSPIRV(const std::vector<unsigned int>& vs,
                          const std::vector<unsigned int>& fs);
    void CompileFromMSL(const std::string& source);

    void* m_mtl_device  = nullptr;  // id<MTLDevice>
    void* m_mtl_library = nullptr;  // id<MTLLibrary>
    void* m_vert_func   = nullptr;  // id<MTLFunction>
    void* m_frag_func   = nullptr;  // id<MTLFunction>

    bool m_valid = false;

}; // ShaderProgram

}
}
