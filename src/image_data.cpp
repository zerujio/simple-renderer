#include "simple_renderer/image_data.hpp"

#include "stb_image.h"

#include <stdexcept>

namespace Simple {
ImageData ImageData::fromFile(const std::string &filename)
{
    int channels;
    glm::ivec2 size;

    auto raw_ptr = reinterpret_cast<std::byte*>(stbi_load(filename.c_str(), &size.x, &size.y, &channels, 0));

    if (!raw_ptr)
        throw std::runtime_error(stbi_failure_reason());

    return {static_cast<ColorChannels>(channels), size, {raw_ptr, stbi_image_free}};
}
} // simple