#include "glsl_syntax.hpp"

#include <array>

namespace simple::glsl {

    struct StringJoiner
    {
        std::ostream &out;
        const char * separator = " ";
        unsigned int items = 0;

        template<class T>
        auto add(const T &value) -> StringJoiner &
        {
            if (items++)
                out << separator;
            out << value;
            return *this;
        }

        template<class T>
        auto addIf(const T &value, const T &null_value) -> StringJoiner &
        {
            if (value != null_value)
                add(value);
            return *this;
        }

        template<class T>
        auto addIf(const T &value) -> StringJoiner &
        {
            if (bool(value))
                add(value);
            return *this;
        }
    };


    auto operator<<(std::ostream &out, Type type) -> std::ostream &
    {
        const std::array type_strings
                {
                        "float",
                        "vec2", "vec3", "vec4",
                        "mat2", "mat3", "mat4"
                };
        return out << type_strings[static_cast<unsigned int>(type)];
    }

    auto operator<<(std::ostream &out, Layout::Memory memory) -> std::ostream &
    {
        const std::array strings {"", "packed", "shared", "std140", "std430"};
        return out << strings[static_cast<unsigned int>(memory)];
    }

    auto operator<<(std::ostream &out, Storage qualifier) -> std::ostream &
    {
        const std::array qualifier_strings {"", "const", "in", "out", "uniform", "buffer"};
        return out << qualifier_strings[static_cast<unsigned int>(qualifier)];
    }

    struct LayoutQualifier
    {
        const char *name;
        int value;

        operator bool () const {return value > -1;}
    };

    auto operator<<(std::ostream &out, const LayoutQualifier &qualifier) -> std::ostream  &
    {
        if (qualifier.name)
            out << qualifier.name << " = " << qualifier.value;
        return out;
    }

    auto operator<<(std::ostream &out, const Layout &layout) -> std::ostream &
    {
        using Memory = Layout::Memory;

        out << "layout(";

        StringJoiner joiner {out, ", "};
        joiner.addIf(layout.memory, Memory::none)
            .addIf(LayoutQualifier{"location", layout.location})
            .addIf(LayoutQualifier{"binding", layout.binding});

        return out << ')';
    }

    auto operator<<(std::ostream &out, Initializer init) -> std::ostream &
    {
        return out << "= " << init.ctor_string;
    }

    auto operator<<(std::ostream &out, const Definition &decl) -> std::ostream &
    {
        StringJoiner join {out, " "};
        join.addIf(decl.layout)
            .addIf(decl.storage, Storage::none)
            .add(decl.type)
            .add(decl.name)
            .addIf(decl.init);
        return out << ';';
    }

    Layout::operator bool() const
    {
        return memory != Memory::none || location != -1 || binding != -1;
    }

    auto operator<<(std::ostream &out, const BlockDefinition &block_def) -> std::ostream &
    {
        StringJoiner join {out, " "};
        join.addIf(block_def.layout)
            .add(block_def.storage)
            .add(block_def.block_name);
        out << "\n{";
        join.separator = "\n  ";
        for (const auto &def : block_def.defs)
            join.add(def);
        out << "\n}";
        join.separator = " ";
        join.addIf(block_def.instance_name);
        return out << ";";
    }
} // simple