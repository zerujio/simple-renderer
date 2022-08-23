#include <simple-renderer/shader_program.hpp>

#include "shader_variables.hpp"

#include <glutils/error.hpp>

#include <sstream>
#include <array>

namespace simple {

    using namespace glutils;

    const std::string vertex_attrib_decl_str = (std::ostringstream()
            << vertex_position_decl << '\n'
            << vertex_normal_decl << '\n'
            << vertex_color_decl << '\n'
    ).str();

    static const std::string uniform_decl_str = (std::ostringstream()
            << model_tr_decl << '\n'
            << view_tr_decl << '\n'
            << proj_tr_decl << '\n'
    ).str();

    static const std::string frag_out_decl_str = (std::ostringstream() << frag_color_decl << '\n').str();


    ShaderProgram::ShaderProgram(const char *vert_src, const char *frag_src)
    {
        Guard<Shader> vert {Shader::Type::vertex};
        {   // vertex shader compilation
            std::array strings {
                    glsl_version_c_str,
                    vertex_attrib_decl_str.c_str(),
                    uniform_decl_str.c_str(),
                    vert_src
            };
            vert->setSource(strings.size(), strings.data());
            vert->compile();
            if (!vert->getParameter(Shader::Parameter::compile_status))
                throw GLError("Vertex shader compilation error");
        }

        Guard<Shader> frag {Shader::Type::fragment};
        {   // fragment shader compilation
            std::array strings {
                glsl_version_c_str,
                uniform_decl_str.c_str(),
                frag_out_decl_str.c_str(),
                frag_src
            };
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

        model_location = m_program->getResourceLocation(GL_UNIFORM, model_tr_decl.name);
        view_location = m_program->getResourceLocation(GL_UNIFORM, view_tr_decl.name);
        proj_location = m_program->getResourceLocation(GL_UNIFORM, proj_tr_decl.name);
    }
} // simple