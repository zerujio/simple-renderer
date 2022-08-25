#include <simple-renderer/shader_program.hpp>

#include "glsl_definitions.hpp"

#include <glutils/error.hpp>

#include <sstream>
#include <array>
#include <iostream>

namespace simple {

    using namespace glutils;

    static auto getVertexAttribDefString() -> const std::string &
    {
        static const auto str {
            ( std::ostringstream()
            << vertex_position_def  << '\n'
            << vertex_normal_def    << '\n'
            << vertex_color_def     << '\n'
            ).str()
        };
        return str;
    }

    static auto getUniformDefString() -> const std::string &
    {
        static const auto str {
            ( std::ostringstream()
                    << model_matrix_def << '\n'
                    << camera_uniform_block_def << '\n'
            ).str()
        };
        return str;
    }

    static auto getFragOutDefString() -> const std::string &
    {
        static const auto str {(std::ostringstream() << frag_color_def << '\n').str()};
        return str;
    }

    ShaderProgram::ShaderProgram(const char *vert_src, const char *frag_src)
    {
        Guard<Shader> vert {Shader::Type::vertex};
        {   // vertex shader compilation
            std::array strings {
                    glsl_version_c_str,
                    getVertexAttribDefString().c_str(),
                    getUniformDefString().c_str(),
                    vert_src
            };

            /* DEBUG */
            std::cout << "Compiling vertex shader:\n";
            for (auto s : strings)
                std::cout << s;
            std::cout << std::endl;
            /* DEBUG */

            vert->setSource(strings.size(), strings.data());
            vert->compile();
            if (!vert->getParameter(Shader::Parameter::compile_status))
                throw GLError("Vertex shader compilation error");
        }

        Guard<Shader> frag {Shader::Type::fragment};
        {   // fragment shader compilation
            std::array strings {
                    glsl_version_c_str,
                    getUniformDefString().c_str(),
                    getFragOutDefString().c_str(),
                    frag_src
            };

            /* DEBUG */
            std::cout << "Compiling fragment shader:\n";
            for (auto s : strings)
                    std::cout << s;
            std::cout << std::endl;
            /* DEBUG */

            frag->setSource(strings.size(), strings.data());
            frag->compile();
            if (!frag->getParameter(Shader::Parameter::compile_status))
                throw GLError("Fragment shader compilation error");
        }

        m_program->attachShader(*vert);
        m_program->attachShader(*frag);
        m_program->link();
        m_program->detachShader(*vert);
        m_program->detachShader(*frag);

        if (!m_program->getParameter(Program::Parameter::link_status))
            throw GLError("Program linking error");
    }
} // simple