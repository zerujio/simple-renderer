#ifndef SIMPLERENDERER_SHADER_PROGRAM_HPP
#define SIMPLERENDERER_SHADER_PROGRAM_HPP

#include "glutils/program.hpp"
#include "glutils/guard.hpp"

#include <stdexcept>

namespace simple {

class BaseShaderProgram
{
public:
    // Various utility classes

    using Interface = GL::Program::Interface;

    [[nodiscard]]
    GLuint getResourceIndex(Interface interface, const char *name) const;

    template<Interface I>
    struct ResourceIndex
    {
        GLuint value;
    };

    template<Interface I>
    [[nodiscard]]
    ResourceIndex<I> getResourceIndex(const char *name) const
    {
        return {getResourceIndex(I, name)};
    }

    ///////////////////////////////////////// Interface Block //////////////////////////////////////////////////////////

    enum class InterfaceBlockType : GLenum
    {
        uniform = static_cast<GLenum>(Interface::uniform_block),
        shader_storage = static_cast<GLenum>(Interface::shader_storage_block)
    };

    template<InterfaceBlockType BlockType>
    class InterfaceBlock
    {
        friend class BaseShaderProgram;

    public:
        using ResourceIndexType = ResourceIndex<static_cast<Interface>(BlockType)>;

        explicit InterfaceBlock(ResourceIndexType resource_index) : m_resource_index(resource_index)
        {}

        [[nodiscard]] GLuint getBindingIndex() const
        { return m_binding_index; }

    private:
        ResourceIndex<static_cast<Interface>(BlockType)> m_resource_index;
        GLuint m_binding_index = 0;
    };

    template<InterfaceBlockType I>
    void setInterfaceBlockBindingIndex(InterfaceBlock<I> &interface_block, GLuint binding_index) const
    {
        if (binding_index == interface_block.m_binding_index)
            return;

        m_setInterfaceBlockBindingIndex<I>(interface_block.m_resource_index, binding_index);
        interface_block.m_binding_index = binding_index;
    }

    template<InterfaceBlockType I>
    class InterfaceBlockAccessor;

    template<InterfaceBlockType I>
    [[nodiscard]] InterfaceBlockAccessor<I> m_makeAccessor(InterfaceBlock<I> &interface_block)
    { return {*this, interface_block}; }

    ///////////////////////////////////////// Uniform Block ////////////////////////////////////////////////////////////

    using UniformBlock = InterfaceBlock<InterfaceBlockType::uniform>;
    using UniformBlockIndex = ResourceIndex<Interface::uniform_block>;

    [[nodiscard]]
    UniformBlockIndex getUniformBlockIndex(const char *name) const
    {
        return getResourceIndex<Interface::uniform_block>(name);
    }

    void setUniformBlockBindingIndex(UniformBlock &uniform_block, uint binding_index) const
    {
        setInterfaceBlockBindingIndex(uniform_block, binding_index);
    }

    ///////////////////////////////////////// Shader Storage Block /////////////////////////////////////////////////////

    using ShaderStorageBlock = InterfaceBlock<InterfaceBlockType::shader_storage>;
    using ShaderStorageBlockIndex = ResourceIndex<Interface::shader_storage_block>;

    [[nodiscard]]
    ShaderStorageBlockIndex getShaderStorageBlockIndex(const char *name) const
    {
        return getResourceIndex<Interface::shader_storage_block>(name);
    }

    void setShaderStorageBlockBindingIndex(ShaderStorageBlock &shader_storage_block, uint binding_index) const
    {
        setInterfaceBlockBindingIndex(shader_storage_block, binding_index);
    }

    /////////////////////////////////////////// Uniform ////////////////////////////////////////////////////////////////

    template<typename T>
    void setUniform(GLint location, T value) const
    {
        m_program.setUniform(location, value);
    }

    template<typename T>
    void setUniform(GLint location, GLsizei count, const T *values) const
    {
        m_program.setUniform(location, count, values);
    }

    template<typename T>
    void setUniformMatrix(GLint location, GLsizei count, GLboolean transpose,
                          const T *values) const
    {
        m_program.setUniformMatrix(location, count, transpose, values);
    }

    /// Wraps a uniform location.
    struct UniformLocation
    {
        explicit UniformLocation(GLuint v) : value(v) {}
        explicit UniformLocation(GLint v) : value(v) {}

        [[nodiscard]] operator bool() const
        { return value > -1; }

        GLint value{-1};
    };

    [[nodiscard]]
    UniformLocation getUniformLocation(const char *name) const;

    template<typename T>
    void setUniform(UniformLocation location, T value) const
    {
        setUniform(location.value, value);
    }

    template<typename T>
    void setUniform(UniformLocation location, GLsizei count, const T *values) const
    {
        setUniform(location.value, count, values);
    }

    template<typename T>
    void setUniformMatrix(UniformLocation location, GLsizei count, GLboolean transpose,
                          const T *values) const
    {
        setUniformMatrix(location.value, count, transpose, values);
    }

    template<typename T>
    struct RequiredLocations
    {
        static constexpr uint count = 1;
    };

    template<auto L, auto Q>
    struct RequiredLocations<glm::vec<L, double, Q>>
    {
        static constexpr uint count = L > 2 ? 2 : 1;
    };

    template<typename T, uint N>
    struct RequiredLocations<T[N]>
    {
        static constexpr uint count = N * RequiredLocations<T>::count;
    };

    template<typename T>
    [[nodiscard]]
    static constexpr uint getRequiredLocations()
    { return RequiredLocations<T>::count; }

    /// Encodes uniform type information and stores its location.
    template<typename T>
    class TypedUniform
    {
        friend class BaseShaderProgram;

    public:
        explicit TypedUniform(UniformLocation location) : m_location(location)
        {}

    private:
        UniformLocation m_location;
    };

    template<typename T, uint N>
    struct TypedUniform<T[N]>
    {
        friend class BaseShaderProgram;

    public:
        explicit TypedUniform(UniformLocation location) : m_location(location)
        {}

        constexpr TypedUniform<T> operator[](uint index) const
        {
            return TypedUniform<T>(UniformLocation(m_location.value + getRequiredLocations<T>() * index));
        }

    private:
        UniformLocation m_location;
    };

    template<typename T>
    void setUniform(TypedUniform<T> uniform, const T &value) const
    {
        setUniform(uniform.m_location, value);
    }

    template<typename T, uint N, typename ArrayLike>
    void setUniform(TypedUniform<T[N]> uniform, const ArrayLike &value) const
    {
        checkUniformArrayBounds(N, value);
        setUniform(uniform.m_location, N, std::data(value));
    }

    template<auto C, auto R, typename T, auto Q>
    void setUniformMatrix(TypedUniform<glm::mat<C, R, T, Q>> uniform, const glm::mat<C, R, T, Q> &value,
                          bool transpose = false) const
    {
        setUniformMatrix(uniform.m_location, 1, transpose, &value);
    }

    template<auto C, auto R, typename T, auto Q, uint N, typename ArrayLike>
    void m_setUniformMatrix(TypedUniform<glm::mat<C, R, T, Q>[N]> uniform, const ArrayLike &values,
                            bool transpose = false) const
    {
        checkUniformArrayBounds(N, values);
        setUniformMatrix(uniform.m_location, N, transpose, std::data(values));
    }

    /// Encodes uniform type information and stores the uniform location and last set value.
    template<typename T>
    class CachedUniform
    {
        friend class BaseShaderProgram;

        static_assert(!std::is_array_v<T>, "cached uniform arrays are not currently supported");

    public:
        explicit CachedUniform(UniformLocation location) : m_location(location)
        {}

        [[nodiscard]] const T &getValue() const
        { return m_cached_value; }

    private:
        UniformLocation m_location;
        T m_cached_value{};
    };

    template<typename T>
    void setUniform(CachedUniform<T> &uniform, const T &value, bool force_update = false) const
    {
        if (!force_update && value == uniform.m_cached_value)
            return;

        setUniform(uniform.m_location, value);
        uniform.m_cached_value = value;
    }

    ////////////////////////////////////////// UniformAccesor //////////////////////////////////////////////////////////
    template<typename T>
    class ConstUniformAccessor
    {
    public:
        using UniformType = CachedUniform<T>;

        explicit ConstUniformAccessor(const UniformType& uniform) : m_uniform (&uniform) {}

        [[nodiscard]]
        const T& get() const { return m_uniform->getValue(); }

        operator const T& () const { return get(); }

    protected:
        const UniformType *m_uniform;
    };

    template<typename T>
    class UniformAccessor : public ConstUniformAccessor<T>
    {
    public:
        using UniformType = typename ConstUniformAccessor<T>::UniformType;

        UniformAccessor(const BaseShaderProgram& program, UniformType& uniform)
                : ConstUniformAccessor<T>(uniform), m_program(&program)
        {}

        void set(const T& value) const
        { m_program->setUniform(*const_cast<UniformType*>(this->m_uniform), value); }

        UniformAccessor& operator=(const T& value) { set(value); return *this; }

    private:
        const BaseShaderProgram *m_program;
    };
    
    template<typename T>
    ConstUniformAccessor<T> makeAccessor(const CachedUniform<T> &uniform) const 
    { return ConstUniformAccessor<T>(uniform); }
    
    template<typename T>
    UniformAccessor<T> makeAccessor(CachedUniform<T> &uniform) const
    { return {*this, uniform}; }

protected:
    [[nodiscard]] GLuint m_queryInterFaceBlockBindingIndex(InterfaceBlockType block_type, GLuint resource_index) const;

    template<typename ArrayLike>
    static constexpr void checkUniformArrayBounds(uint uniform_array_size, const ArrayLike &values)
    {
        if (std::size(values) != uniform_array_size)
            throw std::logic_error("incorrect number of values for setting uniform array");
    }

    template<InterfaceBlockType I>
    void m_setInterfaceBlockBindingIndex(typename InterfaceBlock<I>::ResourceIndexType resource_index,
                                         GLuint binding_index) const
    {
        constexpr bool is_uniform = I == InterfaceBlockType::uniform;
        constexpr bool is_shader_storage = I == InterfaceBlockType::shader_storage;

        if constexpr (is_uniform)
            m_program.setUniformBlockBinding(resource_index.value, binding_index);

        if constexpr (is_shader_storage)
            m_program.setShaderStorageBlockBinding(resource_index.value, binding_index);

        static_assert(is_uniform || is_shader_storage, "invalid interface block type");
    }
    
    GL::Program m_program;
};


/// Holds the data for a shader program.
class ShaderProgram final : public BaseShaderProgram
{
    friend class Renderer;

public:
    /// Compile and link a new GLSL shader program.
    /**
     * Vertex shaders have access to the following vertex attributes:
     *      vec3 vertex_position: the position of the vertex in model space.
     *      vec3 vertex_normal  : the mesh normal at this vertex, in model space.
     *      vec2 vertex_uv      : texture coordinates of the model.
     *
     * The only predefined output for the vertex stage is the built-in gl_Position variable.
     *
     * Both vertex and fragment shaders have access to the following uniforms:
     *      mat4 model_matrix   : model space to world space transform matrix.
     *      mat4 view_matrix    : world space to camera space transform matrix.
     *      mat4 proj_matrix    : camera space to clip space transform matrix.
     *
     * Fragment shaders have a single pre-defined output:
     *      vec4 frag_color : final color for this fragment.
     *
     * @param vert_src GLSL code for the vertex shader stage.
     * @param frag_src GLSL code for the fragment shader stage.
     */
    ShaderProgram(const char *vert_src, const char *frag_src);

    ShaderProgram(const std::string &vert_src, const std::string &frag_src)
            : ShaderProgram(vert_src.c_str(), frag_src.c_str())
    {}
};

} // simple

#endif //SIMPLERENDERER_SHADER_PROGRAM_HPP
