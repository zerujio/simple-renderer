#ifndef SIMPLERENDERER_VERTEX_ARRAY_HPP
#define SIMPLERENDERER_VERTEX_ARRAY_HPP

#include "simple_renderer/buffer.hpp"
#include "simple_renderer/glm_type_traits.hpp"

#include "glutils/vertex_array.hpp"
#include "glutils/vertex_attrib_utils.hpp"

#include <tuple>
#include <utility>
#include <vector>

namespace Simple::Renderer {

template<typename T>
struct ValueTypeStruct
{
    using Type = T;
};

template<auto L, typename T, auto Q>
struct ValueTypeStruct<glm::vec<L, T, Q>> { using Type = T; };

template<typename T>
using ValueType = typename ValueTypeStruct<T>::Type;

/// true if T is one of the supported integer vertex attribute types: int, uint, and their vector counterparts.
template<typename T>
constexpr bool is_integer_vertex_attribute = std::is_same_v<ValueType<T>, GLint> || std::is_same_v<ValueType<T>, GLuint>;

/// true if T is one of the supported integer vertex attribute types: float, double and their vector counterparts.
template<typename T>
constexpr bool is_float_vertex_attribute = std::is_same_v<ValueType<T>, float> || std::is_same_v<ValueType<T>, double>;

template<typename T>
constexpr bool is_vertex_attribute = is_integer_vertex_attribute<T> || is_float_vertex_attribute<T>;

template<typename T>
constexpr uint vertex_attribute_length = 1;

template<auto L, typename T, auto Q>
constexpr uint vertex_attribute_length<glm::vec<L, T, Q>> = L;

/**
 * @brief Describes how the contents of a vertex buffer are interpreted as a vertex attribute.
 * @tparam AttributeType The type of the vertex attribute, as declared in the vertex shader.
 * @tparam SourceType The type of the vertex data as found in the vertex buffer.
 */
template<typename AttributeType, typename SourceType>
struct AttribFormat;

template<typename T, uint L>
struct BaseAttribFormat
{
    static_assert(std::is_scalar_v<T> && !std::is_pointer_v<T> && !std::is_member_pointer_v<T>);
    using BaseType = T; ///< base type of the data in the vertex buffer

    static_assert(1 <= L && L <= 4);
    static constexpr uint length = L; ///< length of the attribute (single element or a 2, 3 or 4 element vector).

    uint relative_offset = 0; ///< byte offset relative to the start of the containing struct.
};

// float attribute formats
template<>
struct AttribFormat<float, float> : BaseAttribFormat<float, 1>
{
};

template<>
struct AttribFormat<float, double> : BaseAttribFormat<double, 1>
{
};

template<auto L>
struct AttribFormat<glm::vec<L, float>, glm::vec<L, float>> : BaseAttribFormat<float, L>
{
};

template<auto L>
struct AttribFormat<glm::vec<L, float>, glm::vec<L, double>> : BaseAttribFormat<double, L>
{
};

// float attribute format converted from int
template<typename SourceType, uint L>
struct IntToFloatAttribFormat : BaseAttribFormat<SourceType, L>
{
    static_assert(is_gpu_compatible_integer<SourceType>);

    /// if true, integers are interpreted as normalized floats; otherwise they are converted as if by a C cast.
    bool normalized = false;
};

template<auto L, typename IntegerType>
struct AttribFormat<glm::vec<L, float>, glm::vec<L, IntegerType>> : IntToFloatAttribFormat<IntegerType, L>
{
};

// integer attribute format
template<auto L, typename AttributeBaseType, typename SourceBaseType>
struct AttribFormat<glm::vec<L, AttributeBaseType>, glm::vec<L, SourceBaseType>> : BaseAttribFormat<SourceBaseType, L>
{
    static_assert(std::is_same_v<AttributeBaseType, GLint> || std::is_same_v<AttributeBaseType, GLuint>);
};

// attribute binding
template<typename AttribType, typename SourceType>
struct AttribBinding
{
    uint index;
    AttribFormat<AttribType, SourceType> format;
};

template<typename Integer>
struct VertexArrayIndex
{
public:
    constexpr explicit VertexArrayIndex(Integer value) noexcept: m_value(value)
    {}

    constexpr explicit operator Integer() const noexcept
    { return value(); }

    [[nodiscard]] constexpr Integer value() const noexcept
    { return m_value; }

private:
    Integer m_value;
};

struct AttribIndex : VertexArrayIndex<uint>
{
    using VertexArrayIndex::VertexArrayIndex;
};
struct BufferIndex : VertexArrayIndex<uint>
{
    using VertexArrayIndex::VertexArrayIndex;
};

using AttribType = GL::VertexAttributeType;

/// Describes how one or more vertex buffer ranges are to be tied together to form an array of vertices.
class VertexArray
{
public:
    /**
     * @brief Bind a range within a buffer as a source for vertex data.
     * @param buffer_index The buffer binding index. Attributes specify their source buffer using this value.
     * @param buffer_range A range within a buffer. May be an invalid range, in which case the binding index is "freed".
     * @param stride Separation between consecutive attribute data elements within the buffer range.
     */
    void bindVertexBuffer(BufferIndex buffer_index, ConstBufferRange<std::byte> buffer_range, uint stride) const;

    /**
     * @brief Bind a range within a buffer as a source for vertex data.
     * @tparam T Data type of the elements within the buffer range.
     * @param buffer_index Buffer binding index. Attributes specify their source buffer using this value.
     * @param buffer_range A range within a buffer, generally a VertexBuffer.
     */
    template<typename T>
    void bindVertexBuffer(BufferIndex buffer_index, BufferRange<T> buffer_range) const
    {
        constexpr uint stride = sizeof(T);
        bindVertexBuffer(buffer_index, static_cast<BufferRange<const std::byte>>(buffer_range), stride);
    }

    /**
     * @brief Specify the instancing divisor for a vertex buffer.
     * @param index buffer binding index.
     * @param divisor if given a non-zero value, the attributes sourced from the buffer will advance every this many
     * rendered instances, instead of for every vertex.
     */
    void setVertexBufferInstanceDivisor(BufferIndex index, uint divisor) const;

    /// Specifies the buffer used for sourcing element indices.
    void bindElementBuffer(const Buffer& element_buffer) const
    {
        m_gl_object.bindElementBuffer(element_buffer.getGLHandle());
    }

    /// Specify that @p attribute sources its vertex data from the @p buffer
    void bindAttribute(AttribIndex attrib, BufferIndex buffer) const;

    /// Enables the vertex attribute with the specified index.
    void enableAttribute(AttribIndex index) const;

    /// Disable the vertex attribute with the specified index.
    void disableAttribute(AttribIndex index) const;

    /// Enable or disable attribute with index @p index based on @p enabled
    void setAttributeEnabled(AttribIndex index, bool enabled) const
    {
        if (enabled)
            enableAttribute(index);
        else
            disableAttribute(index);
    }

    /**
     * @brief Specify the format of a vertex attribute.
     * @tparam ShaderType The type of the attribute in the vertex shader.
     * @param attrib index of the attribute.
     * @param base_type data type of the attribute data found in the buffer.
     * @param relative_offset used for interleaved attributes. Indicates the byte offset of the first data element
     * relative to the start of the buffer range the attribute is bound to. Usually obtained through the offsetof macro.
     * @param normalized When @p T is a floating point type (float, vec2, vec3, vec4) and base_type is an integer type,
     * indicates how the values in the buffer are to be converted. If true, integer normalization is used
     * (255u becomes 1.0f), otherwise they are converted as if by a C-cast (255u becomes 255.0f). If @p base_type is a
     * floating point type, this argument MUST be false.
     */
    template<typename ShaderType>
    std::enable_if_t<is_float_vertex_attribute<ShaderType>, void>
    /*void*/ setAttributeFormat(AttribIndex attrib, AttribType base_type, uint relative_offset, bool normalized = false) const
    {
        constexpr uint size = vertex_attribute_length<ShaderType>;
        static_assert(1 <= size && size <= 4);
        m_gl_object.setAttribFormat(attrib.value(), static_cast<GL::VertexAttributeLength>(size), base_type,
                                    relative_offset, normalized);
    }

    template<typename T>
    std::enable_if_t<is_integer_vertex_attribute<T>, void>
    /*void*/ setAttributeFormat(AttribIndex attrib, AttribType base_type, uint relative_offset) const
    {
        constexpr uint size = vertex_attribute_length<T>;
        static_assert(1 <= size && size <= 4);
        m_gl_object.setAttribIFormat(attrib.value(), static_cast<GL::VertexAttributeLength>(size), base_type,
                                    relative_offset);
    }

    /**
     * @brief Specify the format of a vertex attribute.
     * @tparam ShaderType Type of the attribute in the vertex shader (float, vec3, ivec4, etc.)
     * @tparam BufferType Type of the attribute data within the vertex buffer. This is the actual format of the data.
     * @param attrib Attribute index.
     * @param relative_offset Offset relative to the start of the buffer range that holds the object. Will only be
     * non-zero for objects stored in structs, in which case it can be obtained with the offsetof macro.
     * @param normalized should integer normalization be used to convert integer values to floating point. Only
     * present when @p ShaderType is a floating point type and @p BufferType is not.
     */
    template<typename ShaderType, typename BufferType>
    std::enable_if_t<is_float_vertex_attribute<ShaderType> && !is_float_vertex_attribute<BufferType>, void>
        setAttributeFormat(AttribIndex attrib, uint relative_offset, bool normalized) const
    {
        setAttributeFormat<ShaderType>(attrib, GL::VertexAttrib::type_of<ValueType<BufferType>>, relative_offset,
                                       normalized);
    }

    template<typename S, typename B>
    std::enable_if_t<!is_float_vertex_attribute<S> || is_float_vertex_attribute<B>, void>
        setAttributeFormat(AttribIndex attrib, uint relative_offset) const
    {
        setAttributeFormat<S>(attrib, GL::VertexAttrib::type_of<ValueType<B>>, relative_offset);
    }

    /**
     * @brief Binds a vertex buffer containing a single attribute.
     * @tparam ShaderType Type of the attribute as declared in the vertex shader.
     * @tparam BufferType Type of the data found in the vertex buffer.
     * @param buffer_index Index to bind the vertex buffer range to.
     * @param buffer_range The buffer range to source vertex data from.
     * @param attrib_index The index of the attribute that will be sourced from the buffer.
     * @param normalized indicates if integer normalization is to be used to convert from integer to floating point.
     */
    template<typename ShaderType, typename BufferType>
    void bindVertexBufferAttribute(BufferIndex buffer_index, BufferRange<BufferType> buffer_range,
                                   AttribIndex attrib_index) const
    {
        bindVertexBufferAttributeImpl<ShaderType, BufferType>(buffer_index, buffer_range, attrib_index);
    }

    template<typename S, typename B>
    void bindVertexBufferAttribute(BufferIndex buffer_index, BufferRange<B> buffer_range,
                                   AttribIndex attrib_index, bool normalized) const
    {
        bindVertexBufferAttributeImpl<S, B>(buffer_index, buffer_range, attrib_index, normalized);
    }

    /// Get a handle for the underlying OpenGL object.
    [[nodiscard]]
    GL::VertexArrayHandle getGLObject() const
    { return m_gl_object; }

    /// Checks if the VertexArray object was properly constructed.
    [[nodiscard]]
    bool isValid() const noexcept
    { return (bool) m_gl_object; }

private:
    template<typename S, typename B, typename ... ExtraArgs>
    void bindVertexBufferAttributeImpl(BufferIndex buffer_index, BufferRange<B> buffer_range,
                                       AttribIndex attrib_index, ExtraArgs ... extra_args) const
    {
        bindVertexBuffer(buffer_index, buffer_range);
        bindAttribute(attrib_index, buffer_index);
        setAttributeFormat<S, std::remove_const_t<B>>(attrib_index, 0, extra_args...);
        enableAttribute(attrib_index);
    }

    template<typename T> static const uint attrib_size;

    GL::VertexArray m_gl_object{};
};

} // Simple::Renderer

#endif //SIMPLERENDERER_VERTEX_ARRAY_HPP
