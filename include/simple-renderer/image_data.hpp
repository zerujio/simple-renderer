#ifndef PROCEDURALPLACEMENTLIB_IMAGE_DATA_HPP
#define PROCEDURALPLACEMENTLIB_IMAGE_DATA_HPP

#include "glm/vec2.hpp"

#include <memory>

namespace simple {

class ImageData
{
public:
    enum class ColorChannels { r = 1u, rg = 2u, rgb = 3u, rgba = 4u};

    [[nodiscard]]
    static ImageData fromFile(const char* filename);

    ImageData(ColorChannels num_channels, glm::uvec2 size, std::shared_ptr<std::byte[]> data)
        : m_channels(num_channels), m_size(size), m_data(std::move(data))
    {}

    [[nodiscard]]
    ColorChannels getChannels() const { return m_channels; }

    [[nodiscard]] glm::uvec2 getSize() const { return m_size; }

    /// Get a shared_ptr to the image data
    [[nodiscard]] const std::shared_ptr<std::byte[]>& getSharedPtr() const { return m_data; }

    /// Get a raw pointer to the image data
    [[nodiscard]] std::byte* getDataPtr() const { return m_data.get(); }

private:
    std::shared_ptr<std::byte[]> m_data;
    glm::uvec2 m_size;
    ColorChannels m_channels;
};

} // simple

#endif //PROCEDURALPLACEMENTLIB_IMAGE_DATA_HPP
