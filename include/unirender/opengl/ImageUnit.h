#pragma once

#include "unirender/ImageUnit.h"
#include "unirender/AccessType.h"

namespace ur
{
namespace opengl
{

class ImageUnit : public ur::ImageUnit
{
public:
	ImageUnit(int index);

	virtual void SetTexture(const std::shared_ptr<Texture>& texture) override;

    void SetAccess(ur::AccessType access);

    void Clean();

private:
    int m_index = 0;
    ur::AccessType m_access;

    std::shared_ptr<Texture> m_texture = nullptr;

    bool m_texture_dirty = false;

}; // ImageUnit

}
}