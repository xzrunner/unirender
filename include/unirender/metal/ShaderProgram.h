#pragma once

#include "unirender/ShaderProgram.h"

#include <string>
#include <vector>
#include <unordered_map>

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

    // A reflected uniform block. Its backing MTLBuffer (CPU-written via the
    // QueryUniform() path) must be bound at buffer_index in the given stage
    // at draw time.
    struct UBOBinding
    {
        ShaderType stage;          // VertexShader / FragmentShader
        unsigned int buffer_index; // MSL [[buffer(N)]] index
        size_t       size;
        void*        mtl_buffer;   // id<MTLBuffer>, shared storage
    };
    const std::vector<UBOBinding>& GetUBOs() const { return m_ubos; }
    // MSL buffer index reserved for the vertex (stage_in) data, chosen to not
    // collide with any vertex-stage uniform block.
    int GetVertexBufferIndex() const { return m_vtx_buf_index; }

    // Reflected MSL texture/sampler indices (name -> [[texture(N)]]/[[sampler(N)]]).
    // The engine's SetTexture() slot is NOT generally equal to these, so Draw()
    // must map through them when binding.
    const std::unordered_map<std::string, int>& GetTexSlots()     const { return m_tex_slots; }
    const std::unordered_map<std::string, int>& GetSamplerSlots() const { return m_sampler_slots; }

private:
    void CompileFromSPIRV(const std::vector<unsigned int>& vs,
                          const std::vector<unsigned int>& fs);
    void CompileFromMSL(const std::string& source);

    void* m_mtl_device  = nullptr;  // id<MTLDevice>
    void* m_mtl_library = nullptr;  // id<MTLLibrary>
    void* m_vert_func   = nullptr;  // id<MTLFunction>
    void* m_frag_func   = nullptr;  // id<MTLFunction>

    std::vector<UBOBinding>              m_ubos;
    std::unordered_map<std::string, int> m_tex_slots;     // name -> MSL texture index
    std::unordered_map<std::string, int> m_sampler_slots; // name -> MSL sampler index
    int                                  m_vtx_buf_index = 0;

    bool m_valid = false;

}; // ShaderProgram

}
}
