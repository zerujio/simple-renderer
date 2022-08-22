#include "simple-renderer/renderer.hpp"

#include <glutils/error.hpp>

#include <glm/gtc/type_ptr.hpp>

using namespace glutils;

namespace simple {

    static auto vert_src = R"glsl(
    #version 430 core

    layout(location = 0) in vec2 a_position;
    layout(location = 0) uniform vec2 u_position;

    void main()
    {
        gl_Position = vec4(a_position + u_position, 0.0f, 1.0f);
    }
    )glsl";

    static auto frag_src = R"glsl(
    #version 430 core

    out vec4 final_color;

    void main()
    {
        final_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    )glsl";

    static void initShader(Shader shader, const char *src)
    {
        shader.setSource(1, &src, nullptr);
        shader.compile();
        if (!shader.getParameter(Shader::Parameter::compile_status))
            throw glutils::GLError("Shader compilation failed");
    }

    Renderer::Renderer()
    {
        Guard<Shader> vert {Shader::Type::vertex};
        initShader(*vert, vert_src);
        
        Guard<Shader> frag {Shader::Type::fragment};
        initShader(*frag, frag_src);

        m_program->attachShader(*vert);
        m_program->attachShader(*frag);
        m_program->link();
        m_program->detachShader(*vert);
        m_program->detachShader(*frag);

        glm::vec2 zero_vector;
        m_buffer->allocate(sizeof(glm::vec2), Buffer::Usage::static_read, glm::value_ptr(zero_vector));

        m_va->bindVertexBuffer(0, *m_buffer, 0, 0);
        m_va->bindAttribute(0, 0);
        m_va->setAttribFormat(0, VertexArray::AttribSize::two, VertexArray::AttribType::float_, false,
                              sizeof zero_vector);
        m_va->enableAttribute(0);
    }

    void Renderer::draw(glm::vec2 position)
    {
        m_cmd_q.emplace_back(position);
    }

    void Renderer::finishFrame()
    {
        gl.Clear(GL_COLOR_BUFFER_BIT);
        m_va->bind();
        m_program->use();
        for (const auto & v : m_cmd_q)
        {
            gl.Uniform2f(0, v.x, v.y);
            gl.DrawArrays(GL_POINTS, 0, 1);
        }
        m_cmd_q.clear();
    }

    void Renderer::setViewport(glm::ivec2 lower_left, glm::ivec2 top_right) const
    {
        gl.Viewport(lower_left.x, lower_left.y, top_right.x, top_right.y);
    }

} // simple