#ifndef SIMPLERENDERER_GLSL_SYNTAX_HPP
#define SIMPLERENDERER_GLSL_SYNTAX_HPP

#include <ostream>
#include <vector>

namespace simple::glsl {

    enum class Type
    {
        float_,
        vec2_, vec3_, vec4_,
        mat2_, mat3_, mat4_
    };

    auto operator<<(std::ostream &out, Type type) -> std::ostream &;

    /// layout(...) declaration
    struct Layout final
    {
        enum class Memory
        {
            none,
            packed,
            shared,
            std140,
            std430,
        }
            memory  {Memory::none};
        int location    {-1};
        int binding     {-1};

        operator bool () const;
    };

    auto operator<<(std::ostream &out, Layout::Memory memory) -> std::ostream &;

    auto operator<<(std::ostream &out, const Layout &layout) -> std::ostream &;

    /// GLSL storage qualifier
    enum class Storage
    {
        none,
        constant,
        in,
        out,
        uniform,
        buffer
    };

    auto operator<<(std::ostream &out, Storage qualifier) -> std::ostream &;

    struct Initializer
    {
        const char * ctor_string {nullptr};
        operator bool () const {return ctor_string;}
    };

    auto operator<<(std::ostream &out, Initializer init) -> std::ostream &;

    /// A GLSL variable declaration
    struct Definition final
    {
        Layout layout {};
        Storage storage {Storage::none};
        Type type {Type::float_};
        const char *name {"unnamed_variable"};
        Initializer init {};
    };

    auto operator<<(std::ostream &out, const Definition &decl) -> std::ostream &;

    struct BlockDefinition
    {
        Layout layout {};
        Storage storage {Storage::uniform};
        const char *block_name {"UnnamedBlock"};
        const char *instance_name {nullptr};
        std::vector<Definition> defs;
    };

    auto operator<<(std::ostream &out, const BlockDefinition &block_def) -> std::ostream &;

} // simple

#endif //SIMPLERENDERER_GLSL_SYNTAX_HPP
