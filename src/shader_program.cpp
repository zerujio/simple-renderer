#include <simple-renderer/shader_program.hpp>

#include "simple-renderer/glsl_definitions.hpp"

#include <glutils/error.hpp>

#include <sstream>
#include <array>
#include <iostream>

namespace simple {

    using namespace GL;

    static auto getVertexAttribDefString() -> const std::string &
    {
        static const auto str {
            ( std::ostringstream()
                    << vertex_position_def << '\n'
                    << vertex_normal_def << '\n'
                    << vertex_uv_def << '\n'
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
        Shader vert {ShaderHandle::Type::vertex};
        {   // vertex shader compilation
            std::array strings {
                    glsl_version_c_str,
                    getVertexAttribDefString().c_str(),
                    getUniformDefString().c_str(),
                    vert_src
            };

            vert.setSource(strings.size(), strings.data());
            vert.compile();
            if (!vert.getParameter(ShaderHandle::Parameter::compile_status))
                throw GLError("Vertex shader compilation error: " + vert.getInfoLog());
        }

        Shader frag {ShaderHandle::Type::fragment};
        {   // fragment shader compilation
            std::array strings {
                    glsl_version_c_str,
                    getUniformDefString().c_str(),
                    getFragOutDefString().c_str(),
                    frag_src
            };

            frag.setSource(strings.size(), strings.data());
            frag.compile();
            if (!frag.getParameter(ShaderHandle::Parameter::compile_status))
            {
                throw GLError("Fragment shader compilation error: " + frag.getInfoLog());
            }
        }

        m_program.attachShader(vert);
        m_program.attachShader(frag);
        m_program.link();
        m_program.detachShader(vert);
        m_program.detachShader(frag);

        if (!m_program.getParameter(ProgramHandle::Parameter::link_status))
            throw GLError("ProgramHandle linking error: " + m_program.getInfoLog());
    }
} // simple