#ifndef PROCEDURALPLACEMENTLIB_TEXTURE_2_D_HPP
#define PROCEDURALPLACEMENTLIB_TEXTURE_2_D_HPP

#include "glutils/texture.hpp"

#include "glm/vec2.hpp"

namespace simple {

class ImageData;

class Texture2D
{
public:
    explicit Texture2D(const ImageData& image, bool generate_mipmaps = true);

    [[nodiscard]]
    GL::TextureHandle getGLObject() const { return m_texture; }

    [[nodiscard]]
    glm::uvec2 getSize() const { return m_size; }

private:
    GL::Texture m_texture;
    glm::uvec2 m_size;
};

} // simple

#endif //PROCEDURALPLACEMENTLIB_TEXTURE_2_D_HPP
