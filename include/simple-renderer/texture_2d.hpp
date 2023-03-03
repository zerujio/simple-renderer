#ifndef PROCEDURALPLACEMENTLIB_TEXTURE_2_D_HPP
#define PROCEDURALPLACEMENTLIB_TEXTURE_2_D_HPP

#include "glutils/texture.hpp"

namespace simple {

class ImageData;

class Texture2D
{
public:
    explicit Texture2D(const ImageData& image, bool generate_mipmaps = true);

    [[nodiscard]]
    GL::TextureHandle getGLObject() const { return m_texture; }

private:
    GL::Texture m_texture;
};

} // simple

#endif //PROCEDURALPLACEMENTLIB_TEXTURE_2_D_HPP
