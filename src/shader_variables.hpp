#ifndef SIMPLERENDERER_SHADER_VARIABLES_HPP
#define SIMPLERENDERER_SHADER_VARIABLES_HPP

#include <ostream>

namespace simple {

    constexpr auto glsl_version_c_str = "#version 430 core\n";

    /// GLSL types
    enum class GLType
    {
        float_,
        vec2_, vec3_, vec4_,
        mat2_, mat3_, mat4_
    };

    auto operator<<(std::ostream &out, GLType type) -> std::ostream &;

    /// Qualifiers for layout(...) declarations
    struct LayoutQualifier
    {
        int index{-1};

        constexpr LayoutQualifier() = default;

        constexpr LayoutQualifier(int i) : index(i) {}

        constexpr operator bool() const { return index >= 0; }
    };

    /// "location = ..." layout qualifier
    struct Location final : public LayoutQualifier
    {
        using LayoutQualifier::LayoutQualifier;
    };

    auto operator<<(std::ostream &out, Location location) -> std::ostream &;

    /// "binding = ..." layout qualifier
    struct Binding final : public LayoutQualifier
    {
        using LayoutQualifier::LayoutQualifier;
    };

    auto operator<<(std::ostream &out, Binding binding) -> std::ostream &;

    /// layout(...) declaration
    struct Layout final
    {
        Location location;
        Binding binding;

        constexpr Layout() = default;

        constexpr Layout(Location l) : location(l) {}

        constexpr Layout(Binding b) : binding(b) {}

        constexpr Layout(Location l, Binding b) : location(l), binding(b) {}

        constexpr Layout(Binding b, Location l) : Layout(l, b) {}
    };

    auto operator<<(std::ostream &out, const Layout &layout) -> std::ostream &;

    /// GLSL storage qualifier
    enum class GLStorage
    {
        none,
        constant,
        in,
        out,
        uniform,
        buffer
    };

    auto operator<<(std::ostream &out, GLStorage qualifier) -> std::ostream &;

    /// A GLSL variable declaration
    struct VariableDeclaration final
    {
        Layout layout{};
        GLStorage storage{GLStorage::none};
        GLType type;
        const char *name;
        const char *initializer;

        constexpr VariableDeclaration(GLType t, const char *n, const char *i = nullptr)
                : type(t), name(n), initializer(i) {}

        constexpr VariableDeclaration(GLStorage s, GLType t, const char *n, const char *i = nullptr)
                : storage(s), type(t), name(n), initializer(i) {}

        constexpr VariableDeclaration(Layout l, GLStorage s, GLType t, const char *n, const char *i = nullptr)
                : layout(l), storage(s), type(t), name(n), initializer(i) {}
    };

    auto operator<<(std::ostream &out, const VariableDeclaration &decl) -> std::ostream &;

    constexpr VariableDeclaration vertex_position_decl
        {Layout(Location(0)), GLStorage::in, GLType::vec3_, "vertex_position"};
    constexpr VariableDeclaration vertex_normal_decl
        {Layout(Location(1)), GLStorage::in, GLType::vec3_, "vertex_normal"};
    constexpr VariableDeclaration vertex_color_decl
        {Layout(Location(2)), GLStorage::in, GLType::vec3_, "vertex_color"};

    constexpr VariableDeclaration model_tr_decl
        {GLStorage::uniform, GLType::mat4_, "model_tr", "mat4(1.0f)"};
    constexpr VariableDeclaration view_tr_decl
        {GLStorage::uniform, GLType::mat4_, "view_tr", "mat4(1.0f)"};
    constexpr VariableDeclaration proj_tr_decl
        {GLStorage::uniform, GLType::mat4_, "proj_tr", "mat4(1.0f)"};

    constexpr VariableDeclaration frag_color_decl
        {GLStorage::out, GLType::vec4_, "frag_color"};
} // simple

#endif //SIMPLERENDERER_SHADER_VARIABLES_HPP
