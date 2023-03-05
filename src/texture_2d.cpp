#include "simple-renderer/texture_2d.hpp"
#include "simple-renderer/image_data.hpp"

#include "glm/common.hpp"

#include <utility>
#include <stdexcept>

namespace simple {

[[nodiscard]]
std::pair<GL::Texture::SizedInternalFormat, GL::Texture::DataFormat>
        parseFormat(ImageData::ColorChannels channels)
{
    using IF = GL::Texture::SizedInternalFormat;
    using DF = GL::Texture::DataFormat;

    switch (channels)
    {
        case ImageData::ColorChannels::r:
            return {IF::r8, DF::red};
        case ImageData::ColorChannels::rg:
            return {IF::rg8, DF::rg};
        case ImageData::ColorChannels::rgb:
            return {IF::rgb8, DF::rgb};
        case ImageData::ColorChannels::rgba:
            return {IF::rgba8, DF::rgba};
        default:
            throw std::logic_error("invalid enum value");
    }
}

int calculateMipmapLevels(glm::uvec2 image_size)
{
    int mipmap_levels = 0;
    for (glm::uvec2 size = image_size; size.x > 1 && size.y > 1; size = glm::max({1, 1}, size / 2u))
        mipmap_levels++;
    return mipmap_levels;
}

Texture2D::Texture2D(const ImageData &image, bool generate_mipmaps) : m_texture(GL::Texture::Type::_2d)
{
    const int mipmap_levels = generate_mipmaps ? calculateMipmapLevels(image.getSize()) : 1;

    const auto [internal_format, data_format] = parseFormat(image.getChannels());

    m_texture.setStorage2D(mipmap_levels, internal_format, image.getSize().x, image.getSize().y);
    m_texture.updateImage2D(0, 0, 0, image.getSize().x, image.getSize().y, data_format,
                            GL::Texture::DataType::ubyte, image.getDataPtr());

    if (generate_mipmaps)
        m_texture.generateMipmap();
}
} // simple