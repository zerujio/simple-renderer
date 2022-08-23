#include "shader_variables.hpp"

#include <string>
#include <sstream>

namespace simple {

    auto operator<<(std::ostream &out, GLType type) -> std::ostream &
    {
        const char *const type_strings[]
                {
                        "float",
                        "vec2", "vec3", "vec4",
                        "mat2", "mat3", "mat4"
                };
        return out << type_strings[static_cast<unsigned int>(type)];
    }

    auto operator<<(std::ostream &out, Location location) -> std::ostream &
    {
        return out << "location = " << location.index;
    }

    auto operator<<(std::ostream &out, Binding binding) -> std::ostream &
    {
        return out << "binding = " << binding.index;
    }

    auto operator<<(std::ostream &out, const Layout &layout) -> std::ostream &
    {
        if (!layout.location && !layout.binding)
            return out;

        out << "layout(";

        if (layout.location)
        {
            out << layout.location;
            if (layout.binding)
                out << ", " << layout.binding;
        } else
            out << layout.binding;

        return out << ')';
    }

    auto operator<<(std::ostream &out, GLStorage qualifier) -> std::ostream &
    {
        const char *const qualifier_strings[]{"", "const", "in", "out", "uniform", "buffer"};
        return out << qualifier_strings[static_cast<unsigned int>(qualifier)];
    }

    auto operator<<(std::ostream &out, const VariableDeclaration &decl) -> std::ostream &
    {
        out << decl.layout << ' '
            << decl.storage << ' '
            << decl.type << ' '
            << decl.name;
        if (decl.initializer)
            out << " = " << decl.initializer;
        return out << ';';
    }
} // simple